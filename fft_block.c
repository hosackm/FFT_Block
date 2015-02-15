#include <stdlib.h>
#include <math.h>

#include "fft_block.h"
#include "portaudio.h"

#define FFT_BLOCK_DEFAULT_FFT_LENGTH    65536
#define FFT_BLOCK_DEFAULT_SAMPLE_RATE   48000


static fft_block_ctx gCTX;
static fft_block_ctx *_this = &gCTX;
static unsigned int b_initialized = 0;

/* --------------- Function Prototypes --------------- */
void hanning(double *samples, const unsigned length);
void convert_mag(fftw_complex *in, double *out, const unsigned length);
/* --------------------------------------------------- */

/**
 *  fft_block_init
 *      Takes a sample rate and an fft length
 *      configures the static ctx instance and
 *      returns a pointer to it
**/
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

    b_initialized = 1;
    
    /* SIZES */
    _this->num_samples = 0;
    _this->pcm_length = fftlength;
    _this->fft_length = fftlength / 2 + 1; /* real to complex concatenation */
    
    /* PORTAUDIO */
    _this->p_pcm_samples = (double *)malloc(sizeof(double) * _this->pcm_length);
    _this->p_fft_mag = (double *)malloc(sizeof(double) * _this->fft_length );
    
    /* FFTW */
    _this->fft_out_cmplx = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * _this->fft_length );
    _this->p_freq_bins = (double *)malloc(sizeof(double) * _this->fft_length );
    _this->plan = fftw_plan_dft_r2c_1d(fftlength
                                       ,_this->p_pcm_samples
                                       ,_this->fft_out_cmplx
                                       ,FFTW_ESTIMATE
                                       );
    
    /* GNUPLOT */
    _this->ctrl = gnuplot_init();
    gnuplot_setstyle(_this->ctrl, "lines");
    gnuplot_cmd(_this->ctrl, "set term aqua title \"FFT Block Window\"");
    gnuplot_cmd(_this->ctrl, "set title \"Microphone Audio Spectrum\"");
    gnuplot_cmd(_this->ctrl, "set logscale x");
    gnuplot_cmd(_this->ctrl, "set yrange [0:100]");
    gnuplot_cmd(_this->ctrl, "set xrange [20:20000]");
    gnuplot_cmd(_this->ctrl, "set ylabel \"Magnitude (dB)\"");
    gnuplot_cmd(_this->ctrl, "set xlabel \"Frequency (Hz)\"");
    
    /* Fill Frequency bins */
    for(i = 0; i < _this->fft_length; ++i)
    {
        _this->p_freq_bins[i] = (i * ((float) FFT_BLOCK_DEFAULT_SAMPLE_RATE) / _this->pcm_length);
    }

    return 0;
}

/**
 *  fft_block_close 
 *      Called to close and clean up the fft_block
 *      instance
**/
void fft_block_close()
{
    if(!b_initialized)
    {
        return;
    }

    b_initialized = 0;
    free(_this->p_pcm_samples);
    free(_this->p_fft_mag);
    free(_this->p_freq_bins);
    fftw_free(_this->fft_out_cmplx);
    
    gnuplot_close(_this->ctrl);
}

/**
 *  fft_block_process
 *      Called every time Portaudio calls our callback
 *      This will passthrough audio after making a
 *      local copy of it in the instance's p_pcm_samples
**/
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
    {
        return paAbort;
    }

    for(i = 0; i < framesPerBuffer; ++i)
    {
        *output++ = _this->p_pcm_samples[_this->num_samples++] = *input++;

        if(_this->num_samples == _this->pcm_length)
        {
            /* Perform fft and copy to p_fft_out */
            hanning(_this->p_pcm_samples, _this->pcm_length);
            convert_mag(_this->fft_out_cmplx, _this->p_fft_mag, _this->fft_length);
            fftw_execute(_this->plan);
            
            /* Plot */
            gnuplot_resetplot(_this->ctrl);
            gnuplot_plot_xy(_this->ctrl, _this->p_freq_bins, _this->p_fft_mag, _this->fft_length, "");

            /* Wrap around to start of p_pcm_samples */
            _this->num_samples -= _this->pcm_length;
        }
    }

    return paContinue;
}

/**
 *  Apply Hanning window to samples to smooth edges of sample blocks
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
        double mult = 0.5 * (1.0 - cos(2 * M_PI * i / N));
        *samples = mult * (*samples);
        samples++;
    }
    
}

/**
 *  Convert complex numbers to magnitudes.  This is done
 *  by taking the square root of the sum of squares of
 *  real and imaginary parts.  20 * log(n) => dB
**/
void convert_mag
(
    fftw_complex *in
    ,double *out
    ,const unsigned length
)
{
    unsigned int i;
    
    for(i = 0; i < length; ++i)
    {
        double real, imag;
        
        real = in[i][0];
        imag = in[i][1];
        
        out[i] = 20.0f * log(sqrt(real*real + imag*imag));
    }
}
