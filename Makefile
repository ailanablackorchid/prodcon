INCLUDES        = -I. -I/usr/include

LIBS		= libsocklib.a  \
			-ldl -lpthread -lm

COMPILE_FLAGS   = ${INCLUDES} -c
COMPILE         = gcc ${COMPILE_FLAGS}
LINK            = gcc -o

C_SRCS		= \
		passivesock.c \
		connectsock.c \
		producer.c \
		consumer.c \
		threadedechoserver.c

SOURCE          = ${C_SRCS}

OBJS            = ${SOURCE:.c=.o}

EXEC		= producer consumer echoserver

.SUFFIXES       :       .o .c .h

all		:	library producer consumer echoserver

.c.o            :	${SOURCE}
			@echo "    Compiling $< . . .  "
			@${COMPILE} $<

library		:	passivesock.o connectsock.o
			ar rv libsocklib.a passivesock.o connectsock.o

echoserver	:	threadedechoserver.o
			${LINK} $@ threadedechoserver.o ${LIBS}

producer		:	producer.o
			${LINK} $@ producer.o ${LIBS}
			
consumer		:	consumer.o
			${LINK} $@ consumer.o ${LIBS}			

clean           :
			@echo "    Cleaning ..."
			rm -f tags core *.out *.o *.lis *.a ${EXEC} libsocklib.a
