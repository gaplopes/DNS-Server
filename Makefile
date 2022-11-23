FLAGS = -Wall -pthread
CC = gcc
PROG = dnsserver
OBJS = dnsserver.o header.o
all: ${PROG}

clean:
	rm ${OBJS} ${PROG} *~

${PROG}: ${OBJS}
	${CC} ${FLAGS} ${OBJS} -o $@

.c.o:
	${CC} ${FLAGS} $< -c

################
header.o: header.h header.c
dnsserver.o: header.h dnsserver.c
dnsserver: header.o dnsserver.o