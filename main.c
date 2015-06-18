#include <stdio.h>
#include <assert.h>

#include "portaudio.h"
#include "fft_block.h"

#ifdef _WIN32
#include <conio.h>
#endif

#define SAMPLE_RATE 48000
#define FFT_LENGTH  2048
#define PA_CHECKERROR(x) assert( (x) == paNoError);


static int callback(const void* input,
                    void* output,
                    unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* time_info,
                    PaStreamCallbackFlags flags,
                    void* userData)
{
    float* in = (float*)input;
    float* out = (float*)output;

    /* Perform FFT process */
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

    /* Initialize fft block */
    fft_err = fft_block_init(SAMPLE_RATE, FFT_LENGTH);

    /* Init Portaudio */
    err = Pa_Initialize();
    PA_CHECKERROR(err);

    /* Use default audio device with one input and output */
    err = Pa_OpenDefaultStream(&stream
                               ,1
                               ,1
                               ,paFloat32
                               ,SAMPLE_RATE
                               ,256
                               ,callback
                               ,NULL);
    PA_CHECKERROR(err);

    /* Let Portaudio start */
    printf("Starting stream... press 'enter' to exit\n");
    err = Pa_StartStream(stream);
    PA_CHECKERROR(err);

/* Run until user provides keyboard input */
#ifdef _WIN32
    _getch();
#else
    getchar();
#endif

    /* Stop Portaudio */
    printf("\nDone!\n");
    err = Pa_StopStream(stream);
    PA_CHECKERROR(err);

    /* free the fft block */
    fft_block_close();

    /* Clean up Portaudio */
    err = Pa_CloseStream(stream);
    PA_CHECKERROR(err);

    err = Pa_Terminate();
    PA_CHECKERROR(err);

    return 0;
}
