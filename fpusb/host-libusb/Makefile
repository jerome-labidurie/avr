
CFLAGS=-O2 -Wall -rdynamic `pkg-config libusb-1.0 --cflags`
LDFLAGS=-lm -Wl,-rpath=. -ldl `pkg-config libusb-1.0 --libs`
CC=gcc

all: fpusb

# %.o: %.c
# 	${CC} ${CFLAGS_S} -c -o $@ $<
# 	
# fpusb: fpusb.o
# 	${CC} ${LDFLAGS_S} -o $@ $<
# 
# 

$(TARGET).o: $(TARGET).c
$(TARGET): $(TARGET).o

clean:
	rm -rf *.o fpusb
