all: ../out/variant.test
	../out/variant.test

../out/variant.test: ../out/variant.test.o ../out/lick.o
	g++ -g -o../out/variant.test ../out/variant.test.o ../out/lick.o

../out/variant.test.o: variant.test.cc variant.h lick.h
	g++ -std=c++1y -c -g -Wall -Wextra -o../out/variant.test.o variant.test.cc

../out/lick.o: lick.cc lick.h
	g++ -std=c++1y -c -g -Wall -Wextra -o../out/lick.o lick.cc
