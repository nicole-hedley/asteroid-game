UNAME := $(shell uname)

CFLAGS = -Wall -std=c99 -g

ifeq "$(UNAME)"  "Darwin" 
# operating system is on Mac
LIBS = -L /usr/local/lib/ -lchipmunk -lm `pkg-config --libs gtk+-3.0`
LIBCFLAGS = -I /usr/local/include/chipmunk `pkg-config --cflags gtk+-3.0`
else
# assume operating system is Linux (CS servers)
LIBS = -L /usr/local/lib/ -lchipmunk -lm `pkg-config --libs gtk+-3.0`
LIBCFLAGS = -I /usr/local/include/chipmunk `pkg-config --cflags gtk+-3.0`
endif

CFLAGS += $(LIBCFLAGS)

OBJS = boxdrop.o core.o graphics.o gui.o client.o rbuff.o
BINS = boxdrop core graphics gui server client rbuff

all: boxdrop server

boxdrop: $(OBJS) Makefile
	gcc $(CFLAGS) -o boxdrop $(OBJS) $(LIBS)

core: core.c
	gcc -DTEST $(CFLAGS) -o core core.c $(LIBS)

graphics: graphics.c core.o
	gcc -c -DTEST_GRAPH $(CFLAGS) -o graphics.o graphics.c $(LIBS)
	gcc -DTEST_GRAPH $(CLFAGS) -o graphics graphics.o core.o $(LIBS)

rbuff: rbuff.c rbuff.h
	gcc -DTESTRB $(CFLAGS) -o rbuff rbuff.c $(LIBS)

client: client.c rbuff.c
	gcc -DTESTCLI $(CFLAGS) -o client client.c rbuff.c

gui: gui.c
	gcc -DTEST $(CFLAGS) -o gui gui.c client.c $(LIBS)

server: server.o rbuff.o core.o
	gcc $(CFLAGS) -o server server.o rbuff.o core.o $(LIBS)

testserv: server.c rbuff.o core.o
	gcc $(CFLAGS) -DTESTSERV -o testserv server.c rbuff.o core.o $(LIBS)


clean:
	rm -f $(OBJS) $(BINS) *~
	rm -frd *.dSYM
