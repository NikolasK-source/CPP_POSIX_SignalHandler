all: static_lib
static_lib: libSignalHandler.a

libSignalHandler.a: SignalHandler.o
	ar rcs $@ $^

SignalHandler.o: SignalHandler.cpp
	g++ -std=c++11 -O2 -c $< -o $@

clean:
	rm -f *.o *.a
