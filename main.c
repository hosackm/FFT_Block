#include <stdio.h>

#include "portaudio.h"
#include "fft_block.h"

#define SAMPLE_RATE 48000
#define FFT_LENGTH  65536
#define NUM_SECONDS 5


static int callback(const void* input,
                    void* output,
                    unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* time_info,
                    PaStreamCallbackFlags flags,
                    void* userData)
{
    float* in = (float*)input;
    float* out = (float*)output;
    int i;
    

    fft_block_process(in
                    ,out
                    ,framesPerBuffer
                    ,userData
                    );
    
    return 0;
}

int main(int argc, const char * argv[])
{
    int fft_err;
    PaStream *stream;
    PaError err;


    /* Initialize my fft block */
    fft_err = fft_block_init(SAMPLE_RATE, FFT_LENGTH);

    err = Pa_Initialize();
    if (err != paNoError){
        return -1;
    }
    
    err = Pa_OpenDefaultStream( &stream,
                               1,
                               1,
                               paFloat32,
                               SAMPLE_RATE,
                               256,
                               callback,
                               NULL);
    if (err != paNoError){
        return -1;
    }

    printf("Starting stream...\n");    
    err = Pa_StartStream(stream);
    if (err != paNoError){
        return -1;
    }

    Pa_Sleep(NUM_SECONDS * 1000);
    printf("\nDone!\n");

    err = Pa_CloseStream(stream);
    if (err != paNoError){
        return -1;
    }
    
    err = Pa_Terminate();
    if (err != paNoError){
        return -1;
    }
    
    return 0;
}
