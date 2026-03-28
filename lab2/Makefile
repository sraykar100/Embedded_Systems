CFLAGS = -Wall

OBJECTS = lab2.o fbputchar.o usbkeyboard.o keyboard.o display.o

TARFILES = Makefile lab2.c \
	fbputchar.h fbputchar.c \
	usbkeyboard.h usbkeyboard.c \
	keyboard.h keyboard.c \
	display.h display.c

lab2 : $(OBJECTS)
	cc $(CFLAGS) -o lab2 $(OBJECTS) -lusb-1.0 -pthread

lab2.tar.gz : $(TARFILES)
	rm -rf lab2
	mkdir lab2
	ln $(TARFILES) lab2
	tar zcf lab2.tar.gz lab2
	rm -rf lab2

lab2.o : lab2.c fbputchar.h usbkeyboard.h keyboard.h display.h
fbputchar.o : fbputchar.c fbputchar.h
usbkeyboard.o : usbkeyboard.c usbkeyboard.h
keyboard.o : keyboard.c keyboard.h usbkeyboard.h display.h
display.o : display.c display.h fbputchar.h

.PHONY : clean
clean :
	rm -rf *.o lab2
