cmake_minimum_required(VERSION 3.0)
project(fft_block)

# portaudio stuff
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(FindPortaudio)
include(FindFFTW)

set(FFT_BLOCK_SOURCES   src/fft_block.c
                        src/gnuplot_i.c
                        src/main.c)



add_executable(fft_block ${FFT_BLOCK_SOURCES})
target_include_directories(fft_block PUBLIC ${PORTAUDIO_INCLUDE_DIRS})
target_link_libraries(fft_block ${PORTAUDIO_LIBRARIES})
target_include_directories(fft_block PUBLIC ${FFTW_INCLUDE_DIRS})
target_link_libraries(fft_block ${FFTW_DOUBLE_LIB})
