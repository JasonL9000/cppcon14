all: ../out/variant.test
	../out/variant.test

../out/variant.test: ../out/variant.test.o ../out/lick.o
	clang++ -g -o../out/variant.test ../out/variant.test.o ../out/lick.o

../out/variant.test.o: variant.test.cc variant.h lick.h
	clang++ -std=c++1y -c -g -Wall -Wextra -o../out/variant.test.o variant.test.cc

../out/lick.o: lick.cc lick.h
	clang++ -std=c++1y -c -g -Wall -Wextra -o../out/lick.o lick.cc
