#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "fft_block.h"
#include "portaudio.h"
#include "gnuplot_i.h"

#define FFT_BLOCK_DEFAULT_FFT_LENGTH    65536
#define FFT_BLOCK_DEFAULT_SAMPLE_RATE   48000


/** 
 *  One static instance of the fft block context is allowed
 *  It can be referenced by the global pointer _this
 *
 *  Same goes for the gnuplot_ctrl.  It can be referenced
 *  inside this source file using the _ctrl reference
**/
static fft_block_ctx gCTX;
static fft_block_ctx *_this = &gCTX;
static gnuplot_ctrl *_ctrl;
static unsigned int b_initialized = 0;

/* ------------------------ Function Prototypes --------------------------- */

void hanning(double *samples, const unsigned length);
void convert_mag(const fftw_complex *in, double *out, const unsigned length);

/* ------------------------------------------------------------------------ */


int fft_block_init
(
    unsigned int samplerate
    ,unsigned int fftlength
)
{
    unsigned i;
    
    if(b_initialized || samplerate != 48000)
    {
        return -1;
    }

    /* Avoid double initializing */
    b_initialized = 1;
    
    /* Init SIZES */
    _this->num_samples = 0;
    _this->pcm_length = fftlength;
    _this->fft_length = fftlength / 2 + 1; /* real to complex concatenation */
    
    /* Init PORTAUDIO */
    _this->p_pcm_samples = (double *) malloc(sizeof(double) * _this->pcm_length);
    _this->p_fft_mag = (double *) malloc(sizeof(double) * _this->fft_length );
    
    /* Init FFTW */
    _this->fft_out_cmplx = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * _this->fft_length );
    _this->p_freq_bins = (double *) malloc(sizeof(double) * _this->fft_length );
    _this->plan = fftw_plan_dft_r2c_1d(fftlength
                                       ,_this->p_pcm_samples
                                       ,_this->fft_out_cmplx
                                       ,FFTW_ESTIMATE
                                       );
    
    /* Init GNUPLOT and setup window */
    _ctrl = gnuplot_init();
    
    gnuplot_cmd(_ctrl, "set term aqua title \"FFT Block Window\"");
    gnuplot_cmd(_ctrl, "set title \"Microphone Audio Spectrum\"");
    gnuplot_cmd(_ctrl, "set logscale x");
    gnuplot_cmd(_ctrl, "set yrange [0:100]");
    gnuplot_cmd(_ctrl, "set xrange [20:20000]");
    gnuplot_cmd(_ctrl, "set ylabel \"Magnitude (dB)\"");
    gnuplot_cmd(_ctrl, "set xlabel \"Frequency (Hz)\"");
    
    gnuplot_setstyle(_ctrl, "lines");

    /** ------------------------------------------------------
     *  Fill Frequency bins
     *  ------------------------------------------------------
     *  Each bin will be: (Sample Rate) / (FFT Length) Hz wide
     *  ie:  Sample Rate: 48 kHz, FFT Length: 8192
     *          48000 / 8192 = 5.86 Hz
     *  ======================================================
    **/
    for(i = 0; i < _this->fft_length; ++i)
    {
        _this->p_freq_bins[i] = (i * ((float) FFT_BLOCK_DEFAULT_SAMPLE_RATE) / _this->pcm_length);
    }

    return 0;
}

void fft_block_close()
{
    if(!b_initialized)
    {
        return;
    }

    /* Free dynamic memory. 4 malloc's == 4 free's */
    free(_this->p_pcm_samples);
    free(_this->p_fft_mag);
    free(_this->p_freq_bins);
    fftw_free(_this->fft_out_cmplx);

    /* Close GNUPLOT handle */
    gnuplot_close(_ctrl);
    
    /* Now we can initialize again */
    b_initialized = 0;
}

int fft_block_process
(
    const float* input
    ,float *output
    ,unsigned long framesPerBuffer
    ,void *userData
)
{
    unsigned long i;

    if(!b_initialized)
    {   /* Trying to process before initializing */
        return paAbort;
    }

    for(i = 0; i < framesPerBuffer; ++i)
    {
        /* Copy input to p_pcm_samples and passthrough to output */
        *output++ = _this->p_pcm_samples[_this->num_samples++] = *input++;

        /* Check if we've buffered enough samples */
        if(_this->num_samples == _this->pcm_length)
        {
            /* Apply Hanning Window */
            hanning(_this->p_pcm_samples, _this->pcm_length);
            
            /* Perform FFT */
            fftw_execute(_this->plan);
            
            /* Convert complex numbers into magnitudes */
            convert_mag(_this->fft_out_cmplx, _this->p_fft_mag, _this->fft_length);
            
            /* Clear gnuplot */
            gnuplot_resetplot(_ctrl);
            
            /* Plot magnitudes vs. frequencies */
            gnuplot_plot_xy(_ctrl, _this->p_freq_bins, _this->p_fft_mag, _this->fft_length, "");

            /* Wrap around to start of p_pcm_samples */
            _this->num_samples -= _this->pcm_length;
        }
    }

    /* Everything worked fine */
    return paContinue;
}

/**
 *  Apply Hanning window to samples to smooth edges of sample blocks
 *  More info here: http://en.wikipedia.org/wiki/Hann_function
**/
void hanning
(
    double *samples
    ,const unsigned length
)
{
    unsigned i;
    int N = length - 1;
    
    
    for (i = 0; i < length; ++i) {
        /* Calculate Hanning multiplier w(n) and scale the sample by it */
        double mult = 0.5 * (1.0 - cos(2 * M_PI * i / N));
        samples[i] = mult * samples[i];
    }
    
}

/**
 *  Convert complex numbers to magnitudes.  This is done
 *  by taking the square root of the sum of squares of
 *  real and imaginary parts.  20 * log(n) => dB
**/
void convert_mag
(
    const fftw_complex *in
    ,double *out
    ,const unsigned length
)
{
    unsigned int i;
    
    for(i = 0; i < length; ++i)
    {
        double real = in[i][0];
        double imag = in[i][1];
        
        /* Calculate magnitude */
        out[i] = 20.0f * log(sqrt(real*real + imag*imag));
    }
}
