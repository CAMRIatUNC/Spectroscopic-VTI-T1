/****************************************************************
 *
 * $Source: /pv/CvsTree/pv/gen/src/prg/methods/NSPECT/parsDefinition.h,v $
 *
 * Copyright (c) 1999-2003
 * Bruker BioSpin MRI GmbH
 * D-76275 Ettlingen, Germany
 *
 * All Rights Reserved
 *
 * $Id: parsDefinition.h,v 1.13.2.1 2014/12/17 18:21:16 josh Exp $
 *
 ****************************************************************/



/****************************************************************/
/* INCLUDE FILES						*/
/****************************************************************/

double parameter 
{ 
    display_name "TrIncrement";
    units "ms";
    format "%.3f";
    minimum 0 outofrange nearestval;
    maximum 50000.0 outofrange nearestval;
    relations backbone;
  
}TrIncrement[];

//Excitation Pulse
PV_PULSE_LIST parameter
{
  display_name "Excitation Pulse Shape";
  relations    ExcPulse1EnumRelation;
}ExcPulse1Enum;


PVM_RF_PULSE parameter
{
  display_name "Excitation Pulse";
  relations    ExcPulse1Relation;
}ExcPulse1;

PVM_RF_PULSE_AMP_TYPE parameter
{
  display_name "Excitation Pulse Amplitude";
  relations ExcPulse1AmplRel;
}ExcPulse1Ampl;

//Inversion Pulse
PVM_RF_PULSE parameter
{
  display_name "Inversion Pulse";
  relations    InvPulseRelation;
}InvPulse;



double parameter
{
  display_name "Pre-Acquisition Delay";
  relations DeadTimeRels;
  format "%.3f";
  units "ms";
} DeadTime;

OnOff parameter
{
    display_name "Retro Frequency Lock";
    relations RetroFrequencyLockRelation;
} RetroFrequencyLock_OnOff;

OnOff parameter
{
    display_name "Eddy Current Compensation";
    short_description "Uses reference scan to compensate for eddy current induced phase distortions.";
    relations SetRecoParam;
} Edc_OnOff;

YesNo parameter
{
  display_name "Manual Filter Setting";
  short_description "Deactivates automatic determination of Gaussian filter width for water line extraction.";
  relations SetRecoParam;
} EdcManualFilter;
 	 
double parameter
{
  display_name "Filter Width";
  short_description "Width of Gaussian filter to extract water line.";
  units "Hz";
  format "%.2f";
  minimum 1.0 outofrange nearestval;
  maximum 100000.0 outofrange nearestval;
  //Reco Param -> no call of backbone!
} EdcFilterWidthHz;

int parameter AverageList[];
/****************************************************************/
/*	E N D   O F   F I L E					*/
/****************************************************************/

