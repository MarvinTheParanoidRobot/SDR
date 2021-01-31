//********************************************************************
//*                    SDR                                           *
//*==================================================================*
//* WRITTEN BY: Liam McEvoy    	                 		             *
//* DATE CREATED:   01/01/21                                         *
//* MODIFIED:                                                        *
//*==================================================================*
//* PROGRAMMED IN: Visual Studio                                     *
//*==================================================================*
//* DESCRIPTION: Program to capture and process sampled data for     *
//*             Software Defined Radio program                       *
//********************************************************************
// INCLUDE FILES
//====================================================================

#include <../include/Test_Data.h>
#include <../include/tinywav.h>
#include <fftw3.h>
#include <math.h>
#include <stdlib.h>
//====================================================================
// SYMBOLIC CONSTANTS
//====================================================================
#define _USE_MATH_DEFINES
//====================================================================
// GLOBAL VARIABLES
//====================================================================

//====================================================================
// FUNCTION DECLARATIONS
//====================================================================
int triangle(int window_size, float data[]);

int Welch(int window_size, float data[]);

int Sine(int window_size, float data[]);

int Hann(int window_size, float data[]);

int Hamming(int window_size, float data[]);

int Blackman(int window_size, float data[]);

int Gaussian(int window_size, float data[],float delta);

int multiply(float window[],float data[],int window_size, float final_data[]);

int divide(float window[],float data[],int window_size, float final_data[]);

int writetextc(float data[][2],int N,char *name);

int writetextf(float data[],int N,char *name, bool normalised);

int testsdr(void);

//====================================================================
// FUNCTION DEFINITIONS
//====================================================================
int triangle(int window_size, float data[]){
    int i;
    for (i=0;i<=window_size;i++){
        data[i]=1-(i*(window_size/2))/(window_size/2);
    }
    return 0;
}

int Welch(int window_size, float data[]){
    int i;
    for (i=0;i<=window_size;i++){
        data[i]=1-powf(((i-(window_size/2))/(window_size/2)),2);
    }
    return 0;
}

int Sine(int window_size, float data[]){
    int i;
    for (i=0;i<=window_size;i++){
        data[i]=sinf((M_PI*i)/window_size);
    }
    return 0;
}

int Hann(int window_size, float data[]){
    int i;
    for (i=0;i<=window_size;i++){
        data[i]=powf(sinf((M_PI*i)/window_size),2);
    }
    return 0;
}

int Hamming(int window_size,float data[]){
    int i;
    for (i=0;i<=window_size;i++){
        data[i]=0.54 + 0.46*cosf((2*M_PI/window_size)*i);
    }
    return 0;
}

int Blackman(int window_size, float data[]){
    int i;
    for (i=0;i<=window_size;i++){
        data[i]=0.42 - 0.5*cosf((2*M_PI/window_size)*i)+  0.08*cosf((2*M_PI/window_size)*i);
    }
    return 0;
}

int Gaussian(int window_size, float data[],float delta){
    int i;
    for (i=0;i<=window_size;i++){
        data[i]=expf(-0.5*powf((i-(window_size/2))/(delta*window_size/2),2));
    }
    return 0;
}

int multiply( float window[],float data[],int window_size,float final_data[]){
    int i;
    for(i=0;i<=window_size;i++){
        final_data[i]= window[i]*data[i];
    }
    return 0;
}

int divide(float window[],float data[],int window_size, float final_data[]){
    int i;
    for(i=0;i<=window_size;i++){
        final_data[i]= data[i]/window[i];
    }
    return 0;
}

int writetextf(float data[],int N,char *name, bool normalised){
    FILE *f = fopen(name, "w");
    if (f == NULL){
        printf("Error opening file!\n");
        exit(1);
    }
    int i;
    for (i=0;i<=N;i++){
        if (normalised==0)
            fprintf(f,"%f\n", data[i]/N);
        else{
            fprintf(f,"%f\n", data[i]);
        }
     
        
    }
    
    fclose(f);
    return 0;
}

int writetextc(float data[][2],int N,char *name){
    FILE *f = fopen(name, "w");
    if (f == NULL){
        printf("Error opening file!\n");
        exit(1);
    }
    int i;
    for (i=0;i<=N;i++){
        fprintf(f,"%f\n ", data[i][0]); 
    }
    
    fclose(f);
    return 0;
}

int testsdr(void){
    int NUM_CHANNELS,SAMPLE_RATE,BLOCK_SIZE;
    NUM_CHANNELS=1,SAMPLE_RATE=48000,BLOCK_SIZE=48000;
    
    TinyWav tw;
    char * outputPath;
    outputPath="../data/output.wav"; 
    tinywav_open_write(&tw, NUM_CHANNELS, SAMPLE_RATE, TW_FLOAT32, TW_INLINE, outputPath);
    
    
    int Number_of_samples;
    Number_of_samples=SAMPLE_RATE;
    float datain[Number_of_samples];
    double frequency;
    frequency=1000;
    int i;
    for(i=0;i<10;i++){
        Create_Sine_Wave(SAMPLE_RATE,frequency,Number_of_samples,16,0, datain);
        tinywav_write_f(&tw, datain, Number_of_samples);
        frequency=frequency+1000;
    }
    tinywav_close_write(&tw);
    
    float datatest[Number_of_samples];
    Create_Sine_Wave(SAMPLE_RATE,1000,Number_of_samples,16,0, datatest);
    float *out;
    fftwf_complex *out_cpx;
    out = (float *) malloc(BLOCK_SIZE*sizeof(float));
    fftwf_plan fft;
    fftwf_plan ifft;
    out_cpx = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex)*BLOCK_SIZE);
    fft=fftwf_plan_dft_r2c_1d(BLOCK_SIZE, datatest, out_cpx, FFTW_MEASURE);
    fftwf_execute(fft); 
    writetextc(out_cpx,BLOCK_SIZE,"../data/Test fft");
    ifft=fftwf_plan_dft_c2r_1d(BLOCK_SIZE, out_cpx,  out,FFTW_MEASURE);
    fftwf_execute(ifft);
    writetextf(out,BLOCK_SIZE,"../data/Test ifft",0);
    writetextf(datatest,BLOCK_SIZE,"../data/Test data",1);
    fftwf_destroy_plan(fft);
    fftwf_destroy_plan(ifft);
    free(out);fftwf_free(out_cpx);
    
    return 0;
}


//********************************************************************
// END OF PROGRAM
//********************************************************************