#ifndef _SDR_H_
#define _SDR_H_

#ifdef __cplusplus
extern "C" {
#endif

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


#ifdef __cplusplus
}
#endif

#endif