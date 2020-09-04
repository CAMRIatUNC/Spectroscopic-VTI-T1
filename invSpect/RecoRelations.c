/****************************************************************
 *
 * $Source: /pv/CvsTree/pv/gen/src/prg/methods/NSPECT/RecoRelations.c,v $
 *
 * Copyright (c) 2001 - 2003
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 *
 * $Id: RecoRelations.c,v 1.13.2.1 2014/12/17 18:18:23 josh Exp $
 *

 ****************************************************************/

static const char resid[] = "$Id: RecoRelations.c,v 1.1.2.2 2001/01/11 MAWI ";

#define DEBUG           0
#define DB_MODULE       0
#define DB_LINE_NR      0



#include "method.h"

void SetRecoParam( void )
{
  DB_MSG(("-->SetRecoParam\n"));

  /* set baselevel reconstruction parameter */
  /* default initialization of reco based on acqp pars allready set */
  ATB_InitUserModeReco(
      1,
      0,
      PVM_SpecMatrix, // acq size
      PVM_SpecMatrix, // image size
      NULL, //effAntiAlias=1,
      NULL, //effPftOverscans=halfsize,
      1,
      NULL,
      1,
      NULL,  //linear order,
      NULL,  //linear order (not used)
      NULL,  //linear order (not used)
      PVM_EncNReceivers,
      PVM_EncChanScaling,
      0,
      1);

  /* complex channel combination: */
  RecoCombineMode = AddImages;

  /* configure information available during setup mode */
  GS_info_dig_filling = Yes;
  ParxRelsParRelations("GS_info_dig_filling",Yes); 
  GS_shuffle_profiles=No; // combine spectra in GSP mode

  /* set Reco Params for EDC and RFL*/
  setOptimizationRecoParams();

  DB_MSG(("<--SetRecoParam\n"));
}


void DeriveReco(void)
{
  char avOptions[200], spectCorrOptions[400], zfillOptions[PATH_MAX+200], fileOptions[PATH_MAX+200], fidFile[PATH_MAX];
  int nrReceivers  = RecoNrActiveReceivers();

  DB_MSG(("-->DeriveReco, nRx= %i",nrReceivers));

  if (RecoUserUpdate == No)
  {
    DB_MSG(("<--RecoDerive: no update"));
    return;
  }

  /* standard settings for reconstruction */  
  if(RecoPrototype == No)
    SetRecoParam();

  /*no automatic FT during first pass but after topspin file generation*/
  if(ACQ_scan_type != Setup_Experiment)
    RecoFTOrder[0]=-1;
  else
    RecoFTOrder[0]=0; //FT during GSP

  ParxRelsParRelations("RecoUserUpdate", Yes);

  if(ACQ_scan_type != Setup_Experiment)
  {
    const char *hook = nrReceivers>1? "SOS":"Q";

    if (PVM_RefScanYN == Yes)
      writeRefFile();

    /*averaging*/
    sprintf(avOptions,"avList=<AverageList>; avListSize=1;"
            " nObj=%d; newSize=<RECO_ft_size>;", NI);
    for (int i = 0; i < nrReceivers; i++)
      RecoComputeAppendStage(RECOFIRSTPASS,i,"CAST","RecoAverageFilter","AVE",avOptions);

    if (RetroFrequencyLock_OnOff==On || Edc_OnOff==On) // in case of Drift or EDC correction
    {
      double scanShift, effDigShift;
      int filterWidth=0;

      if (ACQ_jobs[0].scanShift==-1)
      {
                   scanShift=PVM_DigShift;
      } else {
        scanShift= abs(ACQ_jobs[0].scanShift/2.0);
      }

      effDigShift=(PVM_DigShift - scanShift);

      if (EdcManualFilter==Yes && Edc_OnOff==On)
      {
        filterWidth=round(EdcFilterWidthHz/PVM_SpecNomRes[0]/2);
      }
      //else: filterWidth==0 -> auto determination.

      for (int i = 0; i < nrReceivers; i++)
      {
        sprintf(spectCorrOptions,
                "useNav = %d;"
                "refPoint = %f;"
                "digShift = %f;"
                "refcor = %d;"
                "refData = <PVM_RefScan[%d]>;"
                "filterWidth = %d",
                (RetroFrequencyLock_OnOff == On) ? 1:0,
                (RetroFrequencyLock_OnOff==On)?effDigShift:0,
                effDigShift,
                (Edc_OnOff==On) ? 1:0,
                i,
                filterWidth);

        /*Drift + EDC Correction*/
        RecoComputeAppendStage(RECOFIRSTPASS,i,"CAST","RecoSpectCorrFilter","SPECTCORR",spectCorrOptions);
      }

      if (RetroFrequencyLock_OnOff==On)
      {
        const char *castHook = nrReceivers>1? "JSPLIT":"JQ1";

        /*the sink created by default for job1 is not needed*/
        RecoComputeRemoveStage(RECOFIRSTPASS,-1,"JS1");
        if (nrReceivers > 1)
        {
          /*split multichannel data into single data streams*/
          RecoComputeAddStage(RECOFIRSTPASS,-1,"RecoSplitFilter","JSPLIT","dim=1;keepBlocksize=1;");
          RecoComputeConnectStages(RECOFIRSTPASS,-1,"JQ1","JSPLIT");
        }

        /*casting job1-data to float*/
        for (int i = 0; i < nrReceivers; i++)
        {
          RecoComputeAddStage(RECOFIRSTPASS,i,"RecoCastFilter","JCAST","dataRep=FLOAT;wordSize=8;");
          char c1[20];
          sprintf(c1,"JCAST%d",i);
          RecoComputeConnectStages(RECOFIRSTPASS,-1,castHook,c1);     
          RecoComputeConnectStages(RECOFIRSTPASS,i,"JCAST","SPECTCORR");
        }
      }
    }

    /* insert FT along time after channel combination */
    RecoComputeAppendStage(RECOPREPPASS, 0, hook, "RecoFTShiftFilter", "FTS",
                           "shift=0.5; winDirection=0; exponent=1");
    RecoComputeAppendStage(RECOPREPPASS, 0, "FTS", "RecoFTFilter", "FT",
                           "direction=0; exponent=1");

    /* insert branching for export-to-topspin-file with accumulated (and corrected) data */
    PvOvlUtilGetExpnoPath(fidFile, PATH_MAX, "fid");
    sprintf(fileOptions,"filename=\"%s\";shuffle=true",fidFile);

    RecoComputeAppendStage(RECOPREPPASS, 0, hook, "RecoTeeFilter", "T","");
    RecoComputeAddStage(RECOPREPPASS,0,"RecoCastFilter","FILECAST","wordSize=4; dataRep=SIGNED");
    RecoComputeConnectStages(RECOPREPPASS, 0,"T","FILECAST");

    RecoComputeAddStage(RECOPREPPASS,0,"RecoFileSink","FSINK",fileOptions);

    // zero-padding to ensure 1k block size for topspin
    if (GO_block_size == Standard_KBlock_Format)
    {
      sprintf(zfillOptions,"fillingPosition=1.0; fixedSize=%d", ((1024-(PVM_SpecMatrix[0]*2*4)%1024)%1024)/4/2+PVM_SpecMatrix[0]);
      RecoComputeAddStage(RECOPREPPASS,0,"RecoZfillFilter","ZFILL",zfillOptions );
      RecoComputeConnectStages(RECOPREPPASS, 0,"FILECAST","ZFILL");
      RecoComputeConnectStages(RECOPREPPASS, 0,"ZFILL","FSINK");
    }
    else
    {
      RecoComputeConnectStages(RECOPREPPASS, 0,"FILECAST","FSINK");
    }
  }   

  DB_MSG(("<--DeriveReco\n"));
}



void setOptimizationRecoParams(void)
{
  DB_MSG(("-->updateOptimizationReco\n"));

  if (PVM_NavOnOff==On)
  {
    ParxRelsMakeEditable("RetroFrequencyLock_OnOff");
  }
  else
  {
    RetroFrequencyLock_OnOff=Off;
    ParxRelsMakeNonEditable("RetroFrequencyLock_OnOff");

  }

  if (PVM_RefScanYN == Yes)
  {
    ParxRelsMakeEditable("Edc_OnOff");  
  }
  else
  {
    Edc_OnOff = Off;
    ParxRelsMakeNonEditable("Edc_OnOff");
  }

  if (Edc_OnOff==On)
  {
    ParxRelsShowInEditor("EdcManualFilter");
    if (EdcManualFilter==Yes)
    {
      ParxRelsShowInEditor("EdcFilterWidthHz");
    }
    else
    {
      ParxRelsHideInEditor("EdcFilterWidthHz");
    }
  }
  else
  {
    ParxRelsHideInEditor("EdcManualFilter,EdcFilterWidthHz");
  }

  /* Setting phase offsets for multi RX channel combination */
  if (Edc_OnOff==Off)
    ATB_ArrayPhaseSetRecoPhase();
  /* else: keep default values initialized by InitUserModeReco,
     since EDC shifts channel to 0-phase*/

  DB_MSG(("<--updateOptimizationReco\n"));
}


void writeRefFile(void)
{
  DB_MSG(("--> writeRefFile"));
 	 
  uint npoints=RECO_inp_size[0]*2; //real+imag pairs
  uint nchan= RecoNumInputChan;
 	 
  if (PVM_RefScanYN == Yes && ParxRelsParHasValue("PVM_RefScan") == Yes &&
      PARX_get_dim("PVM_RefScan",1)==nchan && PARX_get_dim("PVM_RefScan",2)==npoints)
  {
    FILE *fp=NULL;
 	 
    double *phase = PVM_ArrayPhase;
    double *scale= RecoScaleChan;
 	 
    char fname[PATH_MAX], dpath[PATH_MAX];
    PvOvlUtilGetExpnoPath(dpath, PATH_MAX, 0);
    sprintf(fname,"%s/%s",dpath,"fid.refscan");
    fp=fopen(fname,"w");
    if(fp!=NULL)
    {
      uint i,k;
      int sumRe, sumIm;
      double re, im;
      for(i=0;i<npoints;i+=2)
      {
        sumRe=0; sumIm=0;
        for (k=0;k<nchan;k++)
        {
          //phase shift and scaling before channel combination
          re=(PVM_RefScan[k][i]*cos(phase[k]/180.0*M_PI) - PVM_RefScan[k][i+1] * sin(phase[k]/180.0*M_PI))*scale[k];
          im=(PVM_RefScan[k][i]*sin(phase[k]/180.0*M_PI) + PVM_RefScan[k][i+1] * cos(phase[k]/180.0*M_PI))*scale[k];
 	           
          sumRe+=(int)re;
          sumIm+=(int)im;
        }
        fwrite(&sumRe, sizeof(int),1,fp);
        fwrite(&sumIm, sizeof(int),1,fp);
      }
      fclose(fp);
    }
  }
  DB_MSG(("<-- writeRefFile"));
}

/****************************************************************/
/*		E N D   O F   F I L E				*/
/****************************************************************/
