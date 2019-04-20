CVFLAGS=`pkg-config --cflags --libs opencv`
CFLAGS=-Wall -O2
LIBS=-lrt
CC=g++

main: main.o
	$(CC) -o $@ $< $(CFLAGS) $(LIBS) $(CVFLAGS)

version: version.o
	$(CC) -o $@ $< $(CFLAGS) $(LIBS) $(CVFLAGS)

clean: 
	rm -rf version main *.o

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS) $(CVFLAGS)
