#ifndef FFT_PLOT_BLOCK_H
#define FFT_PLOT_BLOCK_H

#include "fftw3.h"
#include "gnuplot_i.h"

typedef struct
{
    /**
     * PCM Samples from Portaudio
     * Will be an array of length N (65536 for now)
    **/
    double *p_pcm_samples;

    /**
     * Output FFT samples from FFTW library
     * Will be an array of length (N / 2) + 1
    **/
    fftw_complex *fft_out_cmplx;

    /**
     * Magnitude converted samples
     * ie. sqrt(re^2 + im^2) of fft_out_cmplx
    **/
    double *p_fft_mag;

    /**
     * Frequency bins for fft in Hz
    **/
    double *p_freq_bins;

    /**
     * Keeps track of how many samples have been
     * copied to the p_pcm_samples buffer.  When
     * this number is >= N the fft will be performed
    **/
    unsigned int num_samples;
    unsigned int pcm_length;
    unsigned int fft_length;

    /**
     * FFT plan from FFTW library
    **/
    fftw_plan plan;

    /**
     * GNUPLOT vars
    **/
    gnuplot_ctrl *ctrl;

} fft_block_ctx;

/** ------------------------------------------
 *  fft_block_init
 *  ------------------------------------------
 *      Takes a sample rate and an fft length
 *      configures the static ctx instance and
 *      returns a pointer to it
 *  ==========================================
**/
//fft_block_ctx *fft_block_init
int fft_block_init
(
    unsigned int samplerate
    ,unsigned int fftlength
);

/** -----------------------------------------------
 *  fft_block_close 
 *  -----------------------------------------------
 *      Called to close and clean up the fft_block
 *      instance
 *  ===============================================
**/
void fft_block_close();

/** ----------------------------------------------------
 *  fft_block_process
 *  ----------------------------------------------------
 *      Called every time Portaudio calls our callback
 *      This will passthrough audio after making a
 *      local copy of it in the instance's p_pcm_samples
 *  ====================================================
**/
int fft_block_process
(
    const float* input
    ,float *output
    ,unsigned long framesPerBuffer
    ,void *userData
);

#endif