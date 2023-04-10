CFLAGS=-Wall -g
LDFLAGS=
TESTDIR=tests


all: testlex minishell

analex.o: analex.c analex.h
	gcc ${CFLAGS} -c $<

minishell.o: minishell.c analex.h
	gcc ${CFLAGS} -c $<

testlex.o: testlex.c analex.h 
	gcc ${CFLAGS} -c $<

testlex: testlex.o analex.o 
	gcc ${LDFLAGS} -o $@ testlex.o analex.o

minishell: minishell.o analex.o 
	gcc ${LDFLAGS} -o $@ minishell.o analex.o

test: minishell ${TESTDIR}/*.ms
	./runtest.sh ${TESTDIR}/*.ms

testarchive:
	./test_archive.sh

clean:
	rm -rf *.o *~ .minishell.tmp testlex minishell
