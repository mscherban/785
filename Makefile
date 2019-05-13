CVFLAGS=`pkg-config --cflags --libs opencv`
CFLAGS=-Wall -O3
LIBS=-lrt
CC=g++

all: tracktriangles tracktriangles_sample

tracktriangles: tracktriangles.o
	$(CC) -o $@ $< $(CFLAGS) $(LIBS) $(CVFLAGS) $(DISPLAY)
	
tracktriangles_sample: tracktriangles_sample.o
	$(CC) -o $@ $< $(CFLAGS) $(LIBS) $(CVFLAGS)

main: main.o
	$(CC) -o $@ $< $(CFLAGS) $(LIBS) $(CVFLAGS)
	
main2: main2.o
	$(CC) -o $@ $< $(CFLAGS) $(LIBS) $(CVFLAGS)

version: version.o
	$(CC) -o $@ $< $(CFLAGS) $(LIBS) $(CVFLAGS)
	
clean: 
	rm -rf version main tracktriangles tracktriangles_sample main2 *.o

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS) $(CVFLAGS) $(DISPLAY)
