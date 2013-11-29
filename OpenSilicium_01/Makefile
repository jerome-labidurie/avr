# Makefile
MCU = attiny45
#avr2
TARGET=main
CC=avr-gcc
CFLAGS=-Os -Wall -I. -mmcu=$(MCU)
LDFLAGS=-mmcu=$(MCU) -lm -Wl,--gc-sections -Os
# PGMERARD=-c arduino -b 115200 -P /dev/ttyUSB0
PGMERISP=-c avrisp  -b 57600 -P /dev/ttyUSB0
STK500=-c stk500  -P /dev/ttyUSB0
PGMER=$(STK500)
DUDE=/usr/bin/avrdude -V -p t45

# if avr slow clock is defined (128KHz/8)
# use make SLOW_CLOCK=Y
ifeq "$(SLOW_CLOCK)" "Y"
	DUDE+=  -B62
else
	DUDE+=  -B1.1
endif

# C_SRCS=$(wildcard *.c)
C_SRCS=$(TARGET).c
OBJ_FILES=$(C_SRCS:.c=.o)

all: $(TARGET).hex size

clean:
	rm -f *.hex *.o $(TARGET).elf

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET).elf: $(OBJ_FILES)
	$(CC) $(LDFLAGS) -o $@ $(OBJ_FILES)

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex

upload: $(TARGET).hex
	$(DUDE) $(PGMER) -U flash:w:$(TARGET).hex

size: $(TARGET).elf
	@echo "----------------"
	@avr-size --format=avr --mcu=$(MCU) $(TARGET).elf

display_fuses:
	$(DUDE) $(PGMER) -q -U lfuse:r:-:b -U hfuse:r:-:b -U efuse:r:-:b

