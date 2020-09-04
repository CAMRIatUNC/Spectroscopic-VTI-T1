/****************************************************************
 *
 * $Source: /pv/CvsTree/pv/gen/src/prg/methods/NSPECT/parsRelations.c,v $
 *
 * Copyright (c) 2002-2011
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 * $Id: parsRelations.c,v 1.28.2.2 2014/12/17 18:22:29 josh Exp $
 *
 ****************************************************************/

static const char resid[] = "$Id: parsRelations.c,v 1.28.2.2 2014/12/17 18:22:29 josh Exp $ (C) 2002 Bruker BioSpin MRI GmbH";

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
#include <libCore/Math/Math.hh>



/****************************************************************/
/*	I M P L E M E N T A T I O N   S E C T I O N		*/
/****************************************************************/


/****************************************************************/
/*		G L O B A L   F U N C T I O N S			*/
/****************************************************************/


/* ------------------------------------------------------------
   relations of DeadTime
   -------------------------------------------------------------*/
void DeadTimeRels(void)
{
  DeadTimeRange();
  backbone();
}

void DeadTimeRange(void)
{
  if(ParxRelsParHasValue("DeadTime") == No)
    DeadTime = 0.05;
  DeadTime = MAX_OF(0.001, DeadTime);
  DeadTime = MIN_OF(5.0, DeadTime);
}

void updateDeadTime(void)
{
  double min_us;
  
  /* The minimum delay between RF pulse and ACQ_START is given by 10us
   * which are included in the acqdec-subroutine before acquisition.
   * (the 10us include the DE delay)*/

  min_us = 10; 

  DeadTime =  MAX_OF(DeadTime, min_us*1e-3);
}

/*===============================================================
 * ExcPulse1EnumRelation
 * Relation of ExcPulse1Enum (a dynamic enmueration parameter which
 * allows to select one of the existing  pulses)
 *===============================================================*/

void ExcPulse1EnumRelation(void)
{
  DB_MSG(("-->ExcPulse1EnumRelation"));
  
  UT_SetRequest("ExcPulse1Enum");
  backbone();

  DB_MSG(("<--ExcPulse1EnumRelation"));
}

/*===============================================================
 * ExcPulse1AmplRel
 * Relation of ExcPulse1Ampl 
 * This parameter is used in the setup parameter card to adjust
 * the RF pulse amplitude manually
 *===============================================================*/

void ExcPulse1AmplRel(void)
{
  DB_MSG(("-->ExcPulse1AmplRel"));
  UT_SetRequest("ExcPulse1Ampl");
  HandleRFPulseAmplitude();
  DB_MSG(("-->ExcPulse1AmplRel"));
}

void HandleRFPulseAmplitude(void)
{
  DB_MSG(("-->HandleExcPulse1Amplitude"));

  STB_UpdateRFShapeAmplitude("ExcPulse1Ampl",No);
  ATB_SetRFPulse("ExcPulse1","ACQ_RfShapes[0]");
 
  DB_MSG(("<--HandleExcPulse1Amplitude"));
}


/* ===================================================================
 * Relation of ExcPulse1 and InvPulse
 * 
 * All pulses of type PVM_RF_PULSE must have relations like this.
 * However, if you clone this funtion for a different pulse parameter
 * remember to replace the param name in the call to UT_SetRequest!
 *
 * IMPORTANT: this function should not be invoked in the backbone!
 ====================================================================*/

void ExcPulse1Relation(void)
{
  DB_MSG(("-->ExcPulse1Relation"));

  /* Tell the request handling system that the parameter
     ExcPulse1 has been edited */
  UT_SetRequest("ExcPulse1");

  /* Check the values of ExcPulse1 */
  ExcPulse1Range();

  /* call the backbone; further handling will take place there
     (by means of STB_UpdateRFPulse)  */
 
  backbone();

  DB_MSG(("-->ExcPulse1Relation"));
}


void ExcPulse1Range(void)
{
  DB_MSG(("-->ExcPulse1Range"));
  
  // range checker fields to be controlled may be
  // .Length  
  // .Bandwidth
  // .Flipangle
  // .Calculated
  // .Sharpness
  // .Flatness

  double dval=ExcPulse1.Flipangle;
  
  ExcPulse1.Flipangle = MIN_OF(90.0,MAX_OF(dval,1.0));
 
  DB_MSG(("<--ExcPulse1Range"));
}

void InvPulseRelation(void)
{
  DB_MSG(("-->InvPulseRelation"));

  /* Tell the request handling system that the parameter
     ExcPulse1 has been edited */
  UT_SetRequest("InvPulse");

  /* Check the values of InvPulse */
  InvPulseRange();

  /* call the backbone; further handling will take place there
     (by means of STB_UpdateRFPulse)  */
 
  backbone();

  DB_MSG(("-->InvPulseRelation"));
}

void InvPulseRange(void)
{
  DB_MSG(("-->InvPulseRange"));
  
  // range checker fields to be controlled may be
  // .Length  
  // .Bandwidth
  // .Flipangle
  // .Calculated
  // .Sharpness
  // .Flatness
  
InvPulse.Flipangle = 180;
 
  DB_MSG(("<--InvPulseRange"));
}


/*Navigator Relations and RangeChecker */

void RetroFrequencyLockRange(void)
{
  if(!ParxRelsParHasValue("RetroFrequencyLock_OnOff"))
     RetroFrequencyLock_OnOff=Off;
}

void RetroFrequencyLockRelation(void)
{
  RetroFrequencyLockRange();
  //call of backbone not allowed for reco params
}

/****************************************************************/
/*	         L O C A L   F U N C T I O N S			*/
/****************************************************************/

void Local_NAveragesRange(void)
{
  int ival;
  DB_MSG(("Entering Local_NAveragesRange"));
  
  /* 
   *  Range check of PVM_NAverages: prevent it to be negative or 0
   */

  if(ParxRelsParHasValue("PVM_NAverages") == No)
    {
      PVM_NAverages = 1;
    }

  ival = PVM_NAverages;
  PVM_NAverages = MAX_OF(ival,1);
  
  DB_MSG(("Exiting Local_NAveragesRange"));
}


void Local_NAveragesHandler(void)
{

  DB_MSG(("Exiting Local_NAveragesHandler with value %d",PVM_NAverages));

  Local_NAveragesRange();

  /*
   *   Averages range check is finished, handle the request by
   *   the method:
   */
  
  backbone();

  DB_MSG(("Exiting Local_NAveragesHandler with value %d",PVM_NAverages));
  return;
}

/*
 * set parameters of the GS class 
 */
void SetGSparameters(void)
{
  GS_info_normalized_area = Of_raw_data;
}


void LocalAdjHandler(void)
{
  DB_MSG(("-->LocalAdjHandler"));
  
  /* non-method-specific adjustment */
  if (PTB_AdjMethSpec() == No)
  { 
    DB_MSG(("No method specific adjustments"));
    ParxRelsParRelations("PVM_AdjHandler", Yes);
    return;
  }

  /* array phase adjustment */
  if (STB_ArrayPhaseHandleAdjustmentRequests() == 1)
  {
    DB_MSG(("ArrayPhaseAdjustment"));
    /*Turn water suppression pulses off*/
    PVM_WsOnOff = Off;
    ParxRelsParRelations("PVM_WsOnOff",No);
    PVM_NavOnOff = Off;
    ParxRelsParRelations("PVM_NavOnOff",Yes);
    Edc_OnOff = Off;
    ParxRelsParRelations("Edc_OnOff",Yes);
    PVM_NAverages = 1;
    ParxRelsParRelations("PVM_NAverages",No);
    backbone();
    return;
  }

  switch(PTB_GetAdjCategory())
  {
    default:
      return;
    case OTHERADJ:
      if(Yes==PTB_AdjMethSpec() && !strcmp(PTB_GetAdjName(),"ReferenceScan"))
      {
        DB_MSG(("RefScanAdjustment"));
        //        strcpy(GS_auto_name,"PVM_RefScanCounter");
        ATB_RgAdjSetGsPars("PVM_RefScanCounter");
        PVM_DecPower = PVM_NoePower = 
          CFG_RFPulseHighestAttenuation();
        PVM_WsOnOff  = Off;
        ParxRelsParRelations("PVM_WsOnOff",No);
        Edc_OnOff = Off;
        ParxRelsParRelations("Edc_OnOff",Yes);
        PVM_NAverages=PVM_RefScanNA;
        ParxRelsParRelations("PVM_NAverages",No);
        ParxRelsHideInEditor("PVM_NAverages"); //do not show in AdjPlatform but use PVM_RefScanNA
        PVM_NavOnOff=Off;
        ParxRelsParRelations("PVM_NavOnOff",Yes);
        backbone();
      }
      break;
    case RCVR:  /* receiver gain adjustment */
      DB_MSG(("ReceiverGainAdjustment"));
      PVM_NAverages = 1;
      ParxRelsParRelations("PVM_NAverages",No);
      ParxRelsParRelations("PVM_AdjHandler", Yes);
      backbone();
      break;
  }

      
  
  DB_MSG(("<--LocalAdjHandler"));
}

/*called before scan start*/
void ScanStartHandler(void)
{
  DB_MSG(("--> ScanStartHandler"));
  ACQ_DriftCompOffset = 0;  //each scan/gsp has to start with "0"
  
  //(B) -> see description in backbone/UpdateRfPulseVisibility()
  if (PVM_NavOnOff == On)
  {
    ParxRelsMakeNonEditable("ExcPulse1Ampl");
    ParxRelsMakeNonEditable("PVM_ChPul1Ampl");
    ParxRelsMakeNonEditable("PVM_ChPul2Ampl");
    ParxRelsMakeNonEditable("PVM_ChPul3Ampl");
    ParxRelsMakeNonEditable("PVM_VpPul1Ampl");
    ParxRelsMakeNonEditable("PVM_VpPul2Ampl");
  }
  
  DB_MSG(("<-- ScanStartHandler"));
}


/****************************************************************/
/*		E N D   O F   F I L E				*/
/****************************************************************/








