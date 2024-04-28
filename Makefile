CC=g++
CFLAGS=-I/usr/include/libusb-1.0
LDFLAGS=-L/usr/lib -lusb-1.0

all: main

main: main.cpp
	$(CC) -o main main.cpp $(CFLAGS) $(LDFLAGS)

clean:
	rm -f main
