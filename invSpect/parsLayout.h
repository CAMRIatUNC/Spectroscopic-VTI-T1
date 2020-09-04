/****************************************************************
 *
 * $Source: /pv/CvsTree/pv/gen/src/prg/methods/NSPECT/parsLayout.h,v $
 *
 * Copyright (c) 1999-2003
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 * $Id: parsLayout.h,v 1.16.2.1 2014/12/17 18:21:36 josh Exp $
 *
 ****************************************************************/

/****************************************************************/
/*	PARAMETER CLASSES				       	*/
/****************************************************************/


/*--------------------------------------------------------------*
 * Definition of the PV class...
 *--------------------------------------------------------------*/

parclass
{
  DeadTime;
}
attributes
{
  display_name "Sequence Details";
} Sequence_Details;


parclass
{
  NavParameters;
  PVM_NavOnOff;
  Edc_OnOff;
  RetroFrequencyLock_OnOff;
  PVM_RefScanYN;
  ReferenceScan;
  DriftComp_Parameters;
} Optimize;

parclass
{
  ExcPulse1Enum;
  ExcPulse1;
  ExcPulse1Ampl;
  InvPulse;
}
attributes
{
  display_name "RF Pulses";
} RF_Pulses;

parclass
{ 
  DummyScans_Parameters;
  PVM_TriggerModule;
  Trigger_Parameters;
  PVM_FatSupOnOff;
  Fat_Sup_Parameters;
  PVM_FovSatOnOff;
  Fov_Sat_Parameters;
  Sat_Slices_Parameters;
  Suppression;
  PVM_DecOnOff;
  Decoupling_Parameters;
  PVM_NoeOnOff;
  NOE_Parameters;
} Preparation;

parclass
{
  Method;
  PVM_RepetitionTime;
  PVM_NAverages;
  AverageList;
  PVM_NRepetitions;
  PVM_ScanTimeStr;
  PVM_ScanTime;
  PVM_DeriveGains;
  RF_Pulses;
  Nuclei;
  Spectroscopy;
  Preparation;
  Optimize;
  Encoding;
  Sequence_Details;
  PVM_ArrayPhase;
  MapShim;
  TrIncrement;
} MethodClass;

// parameters that should be tested after any editing
conflicts
{
  PVM_RepetitionTime;
  PVM_RefScanYN;
};


// parameters for reconstruction 
parclass
{
  RetroFrequencyLock_OnOff;
  Edc_OnOff;
  EdcManualFilter;
  EdcFilterWidthHz;
}attributes
{
  display_name "Reconstruction Options";
}MethodRecoGroup;


/****************************************************************/
/*	E N D   O F   F I L E					*/
/****************************************************************/



