# wmoc makefile

CC = cc
CFLAGS = -g -O2

PROG = wmoc
OBJS = main.o

all: ${OBJS}
	${CC} ${LDFLAGS} -o ${PROG} ${OBJS}

test:
	cd tests && ./test.sh

clean:
	rm -f ${PROG} ${OBJS} ${PROG}.core
