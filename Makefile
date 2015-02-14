CC=clang
CFLAGS=
LIBS=-L/opt/local/lib -lportaudio
INCLUDES=-I/opt/local/include
SOURCES=main.c fft_block.c
EXECUTABLE=fft_block

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -o $@ $^
