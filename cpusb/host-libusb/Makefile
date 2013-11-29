
CFLAGS_S=-O2 -Wall -rdynamic `pkg-config libusb-1.0 --cflags`
CFLAGS_D=-O2 -Wall -fPIC
LDFLAGS_S=-lm -Wl,-rpath=. -ldl `pkg-config libusb-1.0 --libs`
LDFLAGS_D=-lm -Wl,-rpath=. -shared -lpulse-simple -lpulse
CC=gcc

PLUGINS=$(wildcard p_*.c)
PLUG_SO=$(PLUGINS:.c=.so)

all: cpusb ${PLUG_SO}

%.o: %.c
	${CC} ${CFLAGS_D} -c -o $@ $<

cpusb.o: cpusb.c
	${CC} ${CFLAGS_S} -c -o $@ $<

%.so: %.o
	${CC} ${LDFLAGS_D} -o $@ $<

cpusb: cpusb.o
	${CC} ${LDFLAGS_S} -o $@ $<


$(TARGET).o: $(TARGET).c
$(TARGET): $(TARGET).o

clean:
	rm -rf *.o *.so cpusb
