/****************************************************************
 *
 * $Source: /pv/CvsTree/pv/gen/src/prg/methods/NSPECT/backbone.c,v $
 *
 * Copyright (c) 2002-2011
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 * $Id: backbone.c,v 1.19.2.2 2014/12/17 18:20:23 josh Exp $
 *
 ****************************************************************/

static const char resid[] = "$Id: backbone.c,v 1.19.2.2 2014/12/17 18:20:23 josh Exp $ (C) 2002 Bruker BioSpin MRI GmbH";

#define DEBUG		0
#define DB_MODULE	0
#define DB_LINE_NR	0


/****************************************************************/
/****************************************************************/
/*		I N T E R F A C E   S E C T I O N		*/
/****************************************************************/
/****************************************************************/

/****************************************************************/
/*		I N C L U D E   F I L E S			*/
/****************************************************************/

#include "method.h"

/****************************************************************/
/*	I M P L E M E N T A T I O N   S E C T I O N		*/
/****************************************************************/


/****************************************************************/
/*		G L O B A L   F U N C T I O N S			*/
/****************************************************************/


/* ------------------------------------------------------------ 
  backbone 
  The main part of method code. The consitency of all parameters is checked
  chere, relations between them are resolved and, finally, functions setting
  the base level parameters are called.
  --------------------------------------------------------------*/
void backbone( void )
{
  DB_MSG(("--> backbone"));

  /*
   *  control appearance in GeoEditor: method doesn't support any geometric operation
   */

  /* Nucleus and  PVM_GradCalConst
     are handled by this funtion: */
  STB_UpdateNuclei(Yes);
 
  DB_MSG(("nucleus ok"));

  /* handle RF pulses */   
  UpdateRFPulses();

  PVM_NEchoImages = 1;

  PARX_change_dims("TrIncrement", PVM_NRepetitions);
  /* ------------- spectroscopy part ----------------------- */
  STB_UpdateSpectroscopy( PVM_Nucleus1 );

  STB_UpdateEncoding();
  DB_MSG(("encoding ok"));

  /* handle the Retro Frequency lock */
  if ( PVM_NavOnOff == On )
  {
    ParxRelsShowClassInEditor("NavParameters");
    /* Check PVM_NavPoints AFTER Update of DigPars (in STB_UpdateSpectroscopy): Should be >2*DigShift.
       Otherwise only filtercharacterisitcs are measured or DRU does not send data */
    PVM_NavPoints=MAX_OF(PVM_DigShift*2,PVM_NavPoints);
  }
  else 
  {
    ParxRelsHideClassInEditor("NavParameters");
  }

   /* handling of modules */
  STB_UpdateFatSupModule(PVM_Nucleus1, PVM_DeriveGains);
  LocalDecNoeHandling();
  STB_UpdateFovSatModule(PVM_Nucleus1, PVM_DeriveGains);
  STB_UpdateWsModule(0.0,1);
  PVM_NavSWh=PVM_SpecSWH[0]; // fixed to same acq bandwidth
  STB_UpdateTriggerModule();

  /* optimization params*/
  if (PVM_NavOnOff==On)
  {
    ParxRelsMakeEditable("PVM_DriftCompYesNo");
  }
  else
  {
    PVM_DriftCompYesNo=No;
    ParxRelsMakeNonEditable("PVM_DriftCompYesNo");
  }

  STB_UpdateNavModule(1);  //job1 used for nav acq

  UpdateRfPulseVisibility();
  
  /* DeadTime */
  updateDeadTime();

  /* repetition time */
  repetitionTimeRels();

  /*
   * update ReferenceScan parclass
   */
  STB_UpdateMCRefScan(PVM_EncNReceivers);

  /* DriftCompensation */
  STB_UpdateDriftComp(PVM_RepetitionTime);

  /* set up adjustment list */
  SetAdjustments();

  /* update mapshim parameter class */
  STB_UpdateMapShim(PVM_Nucleus1,"");

  /* set GS parameters */
  SetGSparameters();

  /* set baselevel acquisition parameter */
  SetBaseLevelParam();

  /* set baselevel reconstruction parameter */
  SetRecoParam();

  DB_MSG(("<-- backbone"));
}

void UpdateRFPulses(void)
{
  DB_MSG(("--->UpdateRFPulses"));

  /* Updates all parameters that belong to ExcPulse1 pulse structure
     (as initialized by STB_InitRFPulse see initMeth.c)
  */

  STB_UpdateRFPulse("ExcPulse1",1,PVM_DeriveGains,Conventional);
  
  if(PVM_DeriveGains==Yes)
  {
    ParxRelsHideInEditor("ExcPulse1Ampl");
  }
  else
  {
    ParxRelsShowInEditor("ExcPulse1Ampl");
  }

  ParxRelsShowInFile("ExcPulse1Ampl");
  
  STB_UpdateRFPulse("InvPulse",1,PVM_DeriveGains,Conventional);


  DB_MSG(("<---UpdateRFPulses"));

  return;
}

void repetitionTimeRels( void )
{
  int i,dim;
  double TotalTime,amplifierenable;

  DB_MSG(("--> minRepetitionTimeRels"));

  TotalTime = 0.0;
  amplifierenable = CFG_AmplifierEnable();
  

  PVM_MinRepetitionTime =
    amplifierenable          +  /* time before RF-Pulse */
    PVM_TriggerModuleTime    +
    PVM_FatSupModuleTime     +
    PVM_NoeModuleTime        +
    PVM_WsModuleDuration     +
    PVM_FovSatModuleTime     +
    ExcPulse1.Length         +
    InvPulse.Length          +
    DeadTime                 +  
    PVM_SpecAcquisitionTime  +
    PVM_DigEndDelOpt         + 
    1.0                      + /* min d0 */
    0.14;                      /* fixed delays in ppg */
  
  if(PVM_NavOnOff == On)
    PVM_MinRepetitionTime+=(0.16 + PVM_NavigatorModuleTime);

  PVM_RepetitionTime = ( PVM_RepetitionTime < PVM_MinRepetitionTime ? 
			 PVM_MinRepetitionTime : PVM_RepetitionTime );


  /* updating Dummy Scans afte RepetitionTime and before TotalScanTime */
  STB_UpdateDummyScans(PVM_RepetitionTime);  
  
  /** Calculate Total Scan Time and Set for Scan Editor **/ 
  dim = PTB_GetSpecDim();
  
  double totalTrInc = 0;
  for (int i=0;i<PVM_NRepetitions;i++)
  {
      totalTrInc += TrIncrement[i];
  }
  TotalTime = PVM_NRepetitions*PVM_RepetitionTime*PVM_NAverages+totalTrInc;
  
  for(i=1; i<dim; i++)
     TotalTime *= PVM_SpecMatrix[i];

  //TotalTime *= PVM_NRepetitions;

  PVM_ScanTime = TotalTime;
  UT_ScanTimeStr(PVM_ScanTimeStr,TotalTime);
 

  ParxRelsShowInEditor("PVM_ScanTimeStr");
  ParxRelsMakeNonEditable("PVM_ScanTimeStr");

  DB_MSG(("<-- repetitionTimeRels"));
}

void LocalDecNoeHandling(void)
{
 if((PVM_DecOnOff == On ) || (PVM_NoeOnOff == On))
 {
   if(PVM_NumberOfNuclei!=2)
   {
     STB_InitNuclei(2);
     if(PVM_NumberOfNuclei < 2)
     {
       /* system configuration does not support 2nd RF channel */
       PVM_DecOnOff=PVM_NoeOnOff=Off;
     }
   }
 }
 else if( (PVM_DecOnOff ==Off ) && (PVM_NoeOnOff == Off))
 {
   if(PVM_NumberOfNuclei!=1)
   {
     STB_InitNuclei(1);
   }
 }

 DB_MSG(("Updating DecModule"));
 STB_UpdateDecModule(PVM_Nucleus2,PVM_SpecAcquisitionTime);
 STB_UpdateNoeModule(PVM_Nucleus2);

 if( PVM_DecOnOff == On  && PVM_NoeOnOff == On &&
     PVM_DecMode == Composite_Pulse && PVM_NoeMode == Composite_Pulse_Noe)
 {
   /* both modules use F2 channel and share therefore the CPD pulse 
      element duration PCPD[1] so the element durations have to be matched */
   
   if(PVM_NoePulseElementDuration != PVM_DecPulseElementDuration)
   {
     PVM_NoePulseElementDuration = PVM_DecPulseElementDuration;
     STB_UpdateNoeModule(PVM_Nucleus2);
   }
 }

}
 


void SetAdjustments(void )
{
  DB_MSG(("-->SetAdjustments"));

  PTB_ClearAdjustments();

  STB_ArrayPhaseAppendAdjustment(PVM_EncNReceivers, No);

    if(PVM_RefScanYN == Yes)
    {
      PTB_AppendAdjustment("ReferenceScan",
                           "Reference Scan",
                           "Acquisition of a not suppressed reference scan",
                           per_scan, OTHERADJ);
    }
    else
    {
      PTB_AppendAdjustment("ReferenceScan",
                           "Reference Scan",
                           "Acquisition of a not suppressed reference scan",
                           on_demand,OTHERADJ);
    }

  PTB_AppendConfiguredAdjustment(per_scan, RCVR);

  DB_MSG(("<--SetAdjustments"));
}


void UpdateRfPulseVisibility(void)
{
  /* In PV6.0, job-acquisition prevents the change of pulse
     amplitudes in setup mode. Therefore, amplitude sliders
     are non-editable in setup mode if navigator is on.

     (A) backbone/UpdateRfPulseVisibility
         makes amplitudes editable by default.
     (B) PVM_AcqScanHandler relation makes amplitudes
         non-editable at start of acquisition
     (C) backbone/UpdateRfPulseVisibility makes
         amplitudes non-editable if backbone is
         called during setup mode
     (D) if WsPulses are deactivated, amplitudes are always
         made non-editable to prevent errors when changing
         not used pulses.
  */

  //(A)
  ParxRelsMakeEditable("ExcPulse1Ampl");
  ParxRelsMakeEditable("PVM_ChPul1Ampl");
  ParxRelsMakeEditable("PVM_ChPul2Ampl");
  ParxRelsMakeEditable("PVM_ChPul3Ampl");
  ParxRelsMakeEditable("PVM_VpPul1Ampl");
  ParxRelsMakeEditable("PVM_VpPul2Ampl");
 
  //(C)
  if (PVM_NavOnOff == On)
  {
    if(ACQ_scan_type == Setup_Experiment) 
    {
      ParxRelsMakeNonEditable("ExcPulse1Ampl");
      ParxRelsMakeNonEditable("PVM_ChPul1Ampl");
      ParxRelsMakeNonEditable("PVM_ChPul2Ampl");
      ParxRelsMakeNonEditable("PVM_ChPul3Ampl");
      ParxRelsMakeNonEditable("PVM_VpPul1Ampl");
      ParxRelsMakeNonEditable("PVM_VpPul2Ampl");
    }
  }

  //(D)
  if (PVM_WsOnOff==Off)
  {
    ParxRelsMakeNonEditable("PVM_ChPul1Ampl");
    ParxRelsMakeNonEditable("PVM_ChPul2Ampl");
    ParxRelsMakeNonEditable("PVM_ChPul3Ampl");
    ParxRelsMakeNonEditable("PVM_VpPul1Ampl");
    ParxRelsMakeNonEditable("PVM_VpPul2Ampl");
  }
}

/****************************************************************/
/*		E N D   O F   F I L E				*/
/****************************************************************/


