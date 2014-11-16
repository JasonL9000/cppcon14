all: ../out/variant.test
	../out/variant.test

../out/variant.test: ../out/variant.test.o ../out/lick.o
	mkdir -p ../out; clang++ -g -o ../out/variant.test ../out/variant.test.o ../out/lick.o

../out/variant.test.o: variant.test.cc variant.h lick.h
	mkdir -p ../out; clang++ -std=c++1y -c -g -Wall -Wextra -o ../out/variant.test.o variant.test.cc

../out/lick.o: lick.cc lick.h
	mkdir -p ../out; clang++ -std=c++1y -c -g -Wall -Wextra -o ../out/lick.o lick.cc

clean:
	rm -r ../out
