#include <stdlib.h>

#include "fft_block.h"
#include "portaudio.h"

#define FFT_BLOCK_DEFAULT_FFT_LENGTH    65536
#define FFT_BLOCK_DEFAULT_SAMPLE_RATE   48000


static fft_block_ctx gCTX;
static fft_block_ctx *_this = &gCTX;
static unsigned int b_initialized = 0;

/* --------------- Function Prototypes --------------- */
void hanning(float *samples, const unsigned int length);
//void convert_mag(fftw_complex *in, float *out);
/* --------------------------------------------------- */

/**
 *  fft_block_init
 *      Takes a sample rate and an fft length
 *      configures the static ctx instance and
 *      returns a pointer to it
**/
//fft_block_ctx *fft_block_init
int fft_block_init
(
    unsigned int samplerate
    ,unsigned int fftlength
)
{
    if(b_initialized || samplerate != 48000 || fftlength != 65536)
    {
        return -1;
    }

    b_initialized = 1;
    _this->p_pcm_samples = (float *)malloc(sizeof(float) * fftlength);
    _this->p_fft_out = (float *)malloc(sizeof(float) * (fftlength / 2 + 1) );
    _this->num_samples = 0;

    //return _this;
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
    free(_this->p_fft_out);
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

        if(_this->num_samples == FFT_BLOCK_DEFAULT_FFT_LENGTH)
        {
            /* Perform fft and copy to p_fft_out */

            /* Wrap around to start of p_pcm_samples */
            _this->num_samples -= FFT_BLOCK_DEFAULT_FFT_LENGTH;
        }
    }

    return paContinue;
}