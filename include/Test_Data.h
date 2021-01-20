#ifndef _TESTDATA_H_
#define _TESTDATA_H_

#ifdef __cplusplus
extern "C" {
#endif


void Create_Sine_Wave(double sample_rate, double frequency, int Number_of_samples,int bit,double t_start,float dataout[]);



void DSB_SC_MOD(double sample_rate, double frequency, int Number_of_samples,int bit,double t_start,float dataout[]);




void DSB_LC_MOD(double sample_rate, double frequency, int Number_of_samples,int bit, int A,double t_start,float dataout[]);












#ifdef __cplusplus
}
#endif

#endif