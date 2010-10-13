

all: testwrapbox

testwrapbox:testwrapbox.c eggwrapbox.o 
	gcc -Wall -o $@ -g $^ `pkg-config --cflags --libs gtk+-3.0`

eggwrapbox.o: eggwrapbox.c eggwrapbox.h
	gcc -Wall -o $@ -g -c eggwrapbox.c `pkg-config --cflags gtk+-3.0`


clean:
	rm -f testwrapbox *.o *.so *~
