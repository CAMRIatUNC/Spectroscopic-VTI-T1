/****************************************************************
 *
 * $Source: /pv/CvsTree/pv/gen/src/prg/methods/NSPECT/BaseLevelRelations.c,v $
 *
 * Copyright (c) 2001 - 2011
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 * $Id: BaseLevelRelations.c,v 1.25 2013/07/01 15:26:08 josh Exp $
 *
 ****************************************************************/
static const char resid[] = "$Id: BaseLevelRelations.c,v 1.25 2013/07/01 15:26:08 josh Exp $ (C) 2002 - 2011 Bruker BioSpin MRI GmbH";

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



void SetBaseLevelParam( void )
{

  DB_MSG(("Entering SetBaseLevelParam()"));

  SetBasicParameters();
  
  SetMachineParameters();

  SetFrequencyParameters();
   
  SetPpgParameters();
  
  SetGradientParameters();
  
  SetInfoParameters();
  

  /* settings for multi channel acquisition*/
  ATB_SetMultiRec();

  /* setting baselevel parameters used by modules */
  ATB_SetFatSupBaselevel();
  ATB_SetDecBaseLevel();
  ATB_SetNoeBaseLevel();
  ATB_SetWsBaseLevel();
  ATB_SetFovSatBaseLevel();
  ATB_SetDriftCompBaseLevel(1);

  #if DEBUG
  printTiming();
  #endif
  
  DB_MSG(("Exiting SetBaseLevelParam"));
}





void SetBasicParameters( void )
{
  int specDim;
  int dim;
  
  DB_MSG(("Entering SetBasicParameters()"));
  
  /* ACQ_dim */
  
  specDim = PTB_GetSpecDim();
  
  ACQ_dim = specDim;
  ParxRelsParRelations("ACQ_dim",Yes);
  
  /* ACQ_dim_desc */
  
  /*ATB_SetAcqDimDesc( specDim, spatDim, NULL );  */
  for(dim=0; dim<ACQ_dim; dim++)
    ACQ_dim_desc[dim] = Spectroscopic;
  ParxRelsParRelations("ACQ_dim_desc", Yes);
  
  /* ACQ_size */  
  ACQ_size[0] =  2*PVM_SpecMatrix[0];
  
  for(dim=1; dim<ACQ_dim; dim++)
    ACQ_size[dim] = PVM_SpecMatrix[dim];
  
  ParxRelsParRelations("ACQ_size", Yes);


  
  /* NSLICES */  
  ATB_SetNSlices( 1 );
  
  /* NR */  
  ATB_SetNR( PVM_NRepetitions );
  
  /* NI */  
  ATB_SetNI(1 );
  
  /* Jobs */
  SetJobParameters();

  /* Averaging via jobs and Reco -> NA=1 */  
  ATB_SetNA(1);
  AverageList[0]= PVM_NAverages;


  /* NAE */  
  ATB_SetNAE( 1 );
 
  /* ACQ_ns */
  
  ACQ_ns_list_size = 1;
  
  dim = PARX_get_dim("ACQ_ns_list",1);
  if( dim != 1 )
  {
    PARX_change_dims( "ACQ_ns_list",1 );
  }
  
  NS = 1;
  ACQ_ns = NS;
  ACQ_ns_list[0] = ACQ_ns;
  
  ParxRelsParRelations("ACQ_ns",Yes);
  
  
  /* NECHOES */ 
  NECHOES = 1;
  
  
  
  /* ACQ_obj_order */  
  PARX_change_dims("ACQ_obj_order",NI);
  ACQ_obj_order[0] = 0;
  
  
  /* DS */  
  DS =PVM_DummyScans;
  ACQ_DS_enabled = Yes;
  
  
  ATB_DisableAcqUserFilter();

  ATB_SetAcqScanSize( One_scan );
  
  DB_MSG(("Exiting SetBasicParameters()"));
}

void SetFrequencyParameters( void )
{
  
  DB_MSG(("Entering SetFrequencyParameters()"));
  
  ATB_SetNuc1(PVM_Nucleus1);
  
  sprintf(NUC2,"off");
  sprintf(NUC3,"off");
  sprintf(NUC4,"off");
  sprintf(NUC5,"off");
  sprintf(NUC6,"off");
  sprintf(NUC7,"off");
  sprintf(NUC8,"off");
  
  ATB_SetNucleus(PVM_Nucleus1);
  
  
  /* setting of SW_h, DIGMOD, DSPFIRM and AQ_mod */
  ATB_SetDigPars();
  
  ACQ_O1_mode = BF_plus_Offset;
  ParxRelsParRelations("ACQ_O1_mode",Yes);
  
  ACQ_O2_mode = BF_plus_Offset_list;
  ParxRelsParRelations("ACQ_O2_mode",Yes);
  
  ACQ_O3_mode = BF_plus_Offset_list;
  ParxRelsParRelations("ACQ_O3_mode",Yes);

  O1 = 0.0; 
  O2 = 0.0;
  O3 = 0.0;
  O4 = 0.0;
  O5 = 0.0;
  O6 = 0.0;
  O7 = 0.0;
  O8 = 0.0;

  /* Set BF's to working frequencies on used channels */
  ACQ_BF_enable = No;
  BF1 = PVM_FrqWork[0];
  BF2 = PVM_FrqWork[1];
  SF0ppm=PVM_FrqWork[0]+STB_GetFreqOffset(1,0.0)*1e-6; //reference freq for topspin
  /* call relations of BF1 (no need for other BF's) */
  ParxRelsParRelations("BF1", Yes); 

  {
    int OrderList[1] = {0};
    double OffsetHz[1] = {0.0};
    
    ATB_SetAcqO1List( 1,
		      OrderList,
		      OffsetHz );
    
    ATB_SetAcqO1BList( 1,
		       OrderList,
		       OffsetHz);    
   }
  
  DB_MSG(("Exiting SetFrequencyParameters()"));
}

void SetGradientParameters( void )
{
 
  DB_MSG(("Entering SetGradientParameters()"));
  
  
  ATB_SetAcqPhaseFactor( 1 );
  
  { 
    double GradOrient[3][3] = {{1.0, 0.0, 0.0}, 
                               {0.0, 1.0, 0.0},
                               {0.0, 0.0, 1.0}};
    int ObjOrderList[1] = {0},
      npack = 1,
        nslperpack[1] = {1};
      
      ATB_SetAcqGradMatrix( npack, nslperpack,
			    &GradOrient,
			    ObjOrderList );
  }
  
  
  ACQ_scaling_read  = 1.0;
  ACQ_scaling_phase = 1.0;
  ACQ_scaling_slice = 1.0;
  
  ACQ_rare_factor = 1;
  
  ACQ_grad_str_X = 0.0;
  ACQ_grad_str_Y = 0.0;
  ACQ_grad_str_Z = 0.0;
  
  
  strcpy(GRDPROG, "");
  
  ATB_SetAcqTrims( 1,
                   0.0
                   );
  

  DB_MSG(("Exiting SetGradientParameters()"));
}

void SetInfoParameters( void )
{
  
  DB_MSG(("Entering SetInfoParameters()"));

  // initialize ACQ_n_echo_images ACQ_echo_descr
  //            ACQ_n_movie_frames ACQ_movie_descr
  ATB_ResetEchoDescr();
  ATB_ResetMovieDescr();
  
  
  ACQ_flip_angle = ExcPulse1.Flipangle;
  
  PARX_change_dims("ACQ_echo_time",1);
  ACQ_echo_time[0] = PVM_EchoTime;
  
  PARX_change_dims("ACQ_inter_echo_time",1);
  ACQ_inter_echo_time[0] = PVM_EchoTime;
  
  PARX_change_dims("ACQ_repetition_time",1);
  ACQ_repetition_time[0] = PVM_RepetitionTime;
  
  PARX_change_dims("ACQ_recov_time",1);
  ACQ_recov_time[0] =  PVM_RepetitionTime - ExcPulse1.Length;
  
  PARX_change_dims("ACQ_inversion_time",1);
  ACQ_inversion_time[0] = PVM_InversionTime;
  
  ATB_SetAcqPatientPosition();
  
  ATB_SetAcqMethod();
  
  DB_MSG(("Exiting SetInfoParameters()"));
  
}

void SetMachineParameters( void )
{
  DB_MSG(("Entering SetMachineParameters()"));
  
  
  if( ParxRelsParHasValue("ACQ_word_size") == No )
  {
    ACQ_word_size = _32_BIT;
  }
  
  DE = (DE < 6.0) ? 6.0:DE;
  
  DEOSC = (PVM_SpecAcquisitionTime + PVM_DigEndDelOpt)*1000.0;
  ACQ_scan_shift = 0; //not used in job-acquisition
  ParxRelsParRelations("ACQ_scan_shift",Yes);
  
  PAPS = QP;
  
  DB_MSG(("Exiting SetMachineParameters"));
}

void SetPpgParameters( void )
{
  DB_MSG(("Entering SetPpgParameters()"));
    
  ACQ_vd_list_size=NR;
  ParxRelsParRelations("ACQ_vd_list_size",Yes);
  for(int i=0; i<NR; i++)
      ACQ_vd_list[i]=TrIncrement[i]/1000;


  ParxRelsParRelations("ACQ_vd_list",Yes);
  
  ACQ_vp_list_size=1;
  PARX_change_dims("ACQ_vp_list",1);
  ACQ_vp_list[0] = 1e-6;
  ParxRelsParRelations("ACQ_vp_list",Yes);


  ATB_SetPulprog("invSpect.ppg");

  D[0]  = (PVM_RepetitionTime - PVM_MinRepetitionTime 
            + 1.0) / 1000.0;
  D[1] = DeadTime / 1000.0;
  D[2] = PVM_DigEndDelOpt/1000.0;
  D[8] = CFG_AmplifierEnable()/1000.0;

  ParxRelsParRelations("D",Yes);
  
  /* set shaped pulses, in this method ACQ_RfShapes[0] and ACQ_RfShapes[1] is used           
     the pulse duration is stored in baselevel parameter P[0] and P[1]
  */
  ATB_SetRFPulse("ExcPulse1","ACQ_RfShapes[0]","P[0]");
  ATB_SetRFPulse("InvPulse","ACQ_RfShapes[1]","P[1]");
  
  
  DB_MSG(("Exiting SetPpgParameters"));
}

void SetJobParameters(void)
{
  DB_MSG(("Entering SetJobParameters"));
  
  if (PVM_NavOnOff == On)
    ACQ_jobs_size = 2;
  else
    ACQ_jobs_size = 1;

  ParxRelsParRelations("ACQ_jobs_size",Yes);
  ATB_SetNavModuleBaseLevel(1, 1, PVM_NAverages*PVM_NRepetitions, PVM_DummyScans); //sp1, 1block, totalScans, DS dummies

  ACQ_jobs[0].scanSize = ACQ_size[0];
  ACQ_jobs[0].transactionBlocks = 1; 
  ACQ_jobs[0].nTotalScans = PVM_NAverages*PVM_NRepetitions;
  ACQ_jobs[0].nStoredScans = ACQ_jobs[0].nTotalScans;
  ACQ_jobs[0].dummyScans = PVM_DummyScans;
  ACQ_jobs[0].swh = PVM_SpecSWH[0];
  ACQ_jobs[0].scanShift = 0;

  ParxRelsParRelations("ACQ_jobs",Yes);

  DB_MSG(("Exiting SetJobParameters"));
}


#if DEBUG
#define d(n) (D[n]*1e3)
#define p(n) (P[n]*1e-3)

void printTiming(void)
{
  double aqq=PVM_DigDur,tr;
 
  /* TR */
  tr = 0.01+0.01+d(8)+p(0)+d(1)+aqq+d(2)+0.01+0.1+d(0)+0.01;

  if (PVM_NavOnOff == On)
    tr+=0.16+PVM_NavigatorModuleTime;

  DB_MSG(("TR: %f, should be %f, diff %f (without modules)", tr, PVM_RepetitionTime, tr-PVM_RepetitionTime));

  return;
}

#undef d
#undef p
#endif
