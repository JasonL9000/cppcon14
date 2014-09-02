all: variant.test
	./variant.test

variant.test: variant.test.o lick.o
	g++ -g -ovariant.test variant.test.o lick.o

variant.test.o: variant.test.cc variant.h lick.h
	g++ -std=c++11 -c -g -Wall -Wextra -ovariant.test.o variant.test.cc

lick.o: lick.cc lick.h
	g++ -std=c++11 -c -g -Wall -Wextra -olick.o lick.cc
