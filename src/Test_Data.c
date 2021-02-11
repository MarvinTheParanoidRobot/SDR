//********************************************************************
//*                    Test Data                                     *
//*==================================================================*
//* WRITTEN BY: Liam McEvoy    	                 		             *
//* DATE CREATED:   15/12/20                                         *
//* MODIFIED:                                                        *
//*==================================================================*
//* PROGRAMMED IN: Visual Studio                                     *
//*==================================================================*
//* DESCRIPTION: Creates sampled data for use in testing Software    *
//*              Defined Radio program                               *
//********************************************************************
// INCLUDE FILES
//====================================================================
#include <stdio.h>
#include <math.h>
#include <../include/tinywav.h>
#define _USE_MATH_DEFINES
#include <../include/Test_Data.h>
//====================================================================
// SYMBOLIC CONSTANTS
//====================================================================

//====================================================================
// GLOBAL VARIABLES
//====================================================================

//====================================================================
// FUNCTION DECLARATIONS
//====================================================================
void Create_Sine_Wave(double sample_rate, double frequency, int Number_of_samples,int bit,double t_start,float dataout[]); //creates a sampled sinewave of length N

void DSB_SC_MOD(double sample_rate, double frequency, int Number_of_samples,int bit,double t_start,float dataout[]);//short carrier modulator samples of length N

void DSB_LC_MOD(double sample_rate, double frequency, int Number_of_samples,int bit, int A,double t_start,float  dataout[]);//Large carrier modulatorsamples of length N

//====================================================================
// FUNCTION DEFINITIONS
//====================================================================

//Creates a simple sinewave where you can designate the frequency,sample rate, number of samples, bit level and what time it should start at.
void Create_Sine_Wave(double sample_rate, double frequency, int Number_of_samples,int bit,double t_start,float dataout[]){
    
    int i;
    double w,t;
    w=2*M_PI*frequency;
    t=1/sample_rate;
    for (i=0;i<Number_of_samples;i++){
        double data;
        t_start = t_start + t;
        data=sin(w*t_start)+1;
        data=round(((pow(2,bit)-1)*data)/2);
        data=(2*data)/(pow(2,bit)-1)-1;
        dataout[i]=(float)data;
        
        
    }
}

// Simple Double Sideband Short Carrier Modulation implementation
void DSB_SC_MOD(double sample_rate, double frequency, int Number_of_samples,int bit,double t_start,float dataout[]){

    int i;
    double w,t;
    w=2*M_PI*frequency;
    t=1/sample_rate;
    for (i=0;i<Number_of_samples;i++){
        double data;
        t_start = t_start + t;
        data=cos(w*t_start)+1;
        data=round(((pow(2,bit)-1)*data)/2);
        data=(2*data)/(pow(2,bit)-1)-1;
        dataout[i]=(float)(data*dataout[i]);
        
        
    }
}

// Simple Double Sideband Large Carrier Modulation implementation
void DSB_LC_MOD(double sample_rate, double frequency, int Number_of_samples,int bit, int A,double t_start,float dataout[]){
     
    int i;
    double w,t;
    w=2*M_PI*frequency;
    t=1/sample_rate;
    for (i=0;i<Number_of_samples;i++){
        double data;
        t_start = t_start + t;
        data=cos(w*t_start)+1;
        data=round(((pow(2,bit)-1)*data)/2);
        data=(2*data)/(pow(2,bit)-1)-1;
        dataout[i]=(float)(data*(dataout[i]+A));
        
        
    }
}
//********************************************************************
// END OF PROGRAM
//********************************************************************
