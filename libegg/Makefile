CC=gcc
CFLAGS=`pkg-config gtk+-2.0 --cflags` -g -Wall 
LIBS=`pkg-config gtk+-2.0 --libs`

OBJS=	\
	eggcellrendererpopup.o \
	eggtreeviewstate.o \
	eggreorder.o \
	eggcellrendererpixbuf.o \
	test-egg.o

all: test-egg

clean: 
	rm -f *o test-egg core

test-egg: $(OBJS) 
	$(CC) -o test-egg $(OBJS) $(LIBS)

