.PHONY: clean

INCLUDE=-I./inc

R_FLAGS=-O3 -Wall -pedantic -std=c99
D_FLAGS=-Wall -pedantic -std=c99

release: src/scheme.c r_parser.o r_eval.o r_env.o r_error.o test
	gcc src/scheme.c r_parser.o r_eval.o r_env.o r_error.o -o uScheme $(R_FLAGS) $(INCLUDE)
	./test_suite

test: r_parser.o r_eval.o r_env.o r_error.o
	gcc tests/suite.c r_parser.o r_eval.o r_env.o r_error.o -o test_suite $(R_FLAGS) $(INCLUDE)

r_parser.o: src/parser.c r_error.o 
	gcc src/parser.c r_error.o -o r_parser.o -c $(R_FLAGS) $(INCLUDE)

r_eval.o: src/eval.c r_env.o r_error.o
	gcc src/eval.c r_env.o r_error.o -o r_eval.o -c $(R_FLAGS) $(INCLUDE)

r_env.o: src/env.c
	gcc src/env.c -o r_env.o -c $(R_FLAGS) $(INCLUDE)

r_error.o: src/error.o
	gcc src/error.c -o r_error.o -c $(R_FLAGS) $(INCLUDE)

clean:
	rm *.o uScheme test_suite
