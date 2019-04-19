CVFLAGS=`pkg-config --cflags --libs opencv`
CFLAGS=-Wall -O1
CC=g++

main: main.o
	$(CC) -o $@ $< $(CFLAGS) $(CVFLAGS)

version: version.o
	$(CC) -o $@ $< $(CFLAGS) $(CVFLAGS)

clean: 
	rm -rf version main *.o

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)
