#include <stdio.h>

#include "portaudio.h"
#include "fft_block.h"

#ifdef _WIN32
#include <conio.h>
#endif

#define SAMPLE_RATE 48000
#define FFT_LENGTH  2048


static int callback(const void* input,
                    void* output,
                    unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* time_info,
                    PaStreamCallbackFlags flags,
                    void* userData)
{
    float* in = (float*)input;
    float* out = (float*)output;
    

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

    printf("Starting stream... press 'enter' to exit\n");
    err = Pa_StartStream(stream);
    if (err != paNoError){
        return -1;
    }

/* Run until user provides keyboard input */
#ifdef _WIN32
    _getch();
#else
    getchar();
#endif
    printf("\nDone!\n");
    
    fft_block_close();

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
