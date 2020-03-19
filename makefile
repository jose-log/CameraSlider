

# Output directory
OUTDIR := output

# Binary file name
PROGRAM = main

# Source code files
SRC = main.c \
	  driver.c \
	  external_int.c \
	  init.c \
	  lcd.c \
	  menu.c \
	  move.c \
	  timers.c \
	  uart.c \
	  util.c

LIB = -I./


# enter the parameters for the avrdude isp tool
AVRDUDE_PORT	   = usb
AVRDUDE_PROGRAMMER = usbasp


MCU = atmega328p
AVR_FREQ = 16000000L
#LDSECTION  = --section-start=.text=0x7800
OPTIMIZE   = -Os


# AVRDude
AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -v
AVRDUDE_WRITE_FLASH = -U flash:w:$(OUTDIR)/$(PROGRAM).hex
#AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(PROGRAM).eep
AVRDUDE_WRITE_FUSES = lock:w:$(LOCK):m -U efuse:w:$(EFUSE):m -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m
SAFEMODE = -u
# Fuses
HFUSE := 0x
LFUSE := 0x
EFUSE := 0x
LOCK  := 0x3F
# Fuses (ATmega328p - Arduino Uno)
#HFUSE := 0xDA
#LFUSE := 0xFF
#EFUSE := 0x05
#LOCK  := 0x3F


# Object files tracking based on $(SOURCES)
OBJ  := $(SRC:.c=.o)
DPND := $(SRC:.c=.d)

#CFLAGS    = -g -Wall $(OPTIMIZE) -mmcu=$(MCU) -DF_CPU=$(AVR_FREQ) $(LIB)
CFLAGS    = -g -Wall $(OPTIMIZE) -mmcu=$(MCU) $(LIB)
LDFLAGS   = -Wl,$(LDSECTION)
#override LDFLAGS   = -Wl,-Map,$(PROGRAM).map,$(LDSECTION)

CC          = avr-gcc
CC_SIZE		= avr-size
OBJCOPY     = avr-objcopy
OBJDUMP     = avr-objdump
AVRDUDE 	= avrdude

.PHONY: build program program_fuses poke clean

$(OUTDIR):
	mkdir -p ./$(OUTDIR)

build: $(OUTDIR) $(PROGRAM).hex
	@echo
	@echo ">> Build Finished =)"

program: $(OUTDIR) $(PROGRAM).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH) $(AVRDUDE_WRITE_EEPROM)	

program_fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(SAFEMODE) $(AVRDUDE_WRITE_FUSES)

poke:
	$(AVRDUDE) $(AVRDUDE_FLAGS)


%.elf: $(OBJ)
	$(CC) $(CFLAGS) -o ./$(OUTDIR)/$@ $^
#	$(CC) $(CFLAGS) $(LDFLAGS) -o ./$(OUTDIR)/$@ $^

%.lst: %.elf
	$(OBJDUMP) -h -S ./$(OUTDIR)/$< > ./$(OUTDIR)/$@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex ./$(OUTDIR)/$< ./$(OUTDIR)/$@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec ./$(OUTDIR)/$< ./$(OUTDIR)/$@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary ./$(OUTDIR)/$< ./$(OUTDIR)/$@
	
clean:
	rm $(OUTDIR)/*
	rm -rf *.o *.elf *.lst *.map *.sym *.lss *.eep *.srec *.bin *.hex
