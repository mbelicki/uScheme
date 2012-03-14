INCLUDE=-I./inc

R_FLAGS=-O3 -Wall -pedantic -std=c99
D_FLAGS=-Wall -pedantic -std=c99

release: src/scheme.c r_parser.o r_eval.o test
	gcc src/scheme.c r_parser.o r_eval.o -o uScheme $(R_FLAGS) $(INCLUDE)
	./test_suite

test: r_parser.o r_eval.o
	gcc tests/suite.c r_parser.o r_eval.o -o test_suite $(R_FLAGS) $(INCLUDE)

r_parser.o: src/parser.c 
	gcc src/parser.c -o r_parser.o -c $(R_FLAGS) $(INCLUDE)

r_eval.o: src/eval.c
	gcc src/eval.c -o r_eval.o -c $(R_FLAGS) $(INCLUDE)

clean:
	rm *.o uScheme test_suite
