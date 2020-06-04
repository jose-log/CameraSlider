
###############################################################################
#	INPUT & OUTPUT FILES
###############################################################################

# Output directory
OUTDIR := output

# Binary file name
PROGRAM = main

# Source code files
SRC = 			\
	main.c 		\
	driver.c 	\
	encoder.c 	\
	init.c 		\
	lcd.c 		\
	menu.c 		\
	motor.c 	\
	move.c 		\
	timers.c 	\
	uart.c 		\
	util.c

INC = -I./

# Object files tracking based on $(SOURCES)
OBJ  := $(SRC:.c=.o)

###############################################################################
#	AVRDUDE PARAMETERS
###############################################################################

# enter the parameters for the avrdude isp tool
AVRDUDE 		   = avrdude
AVRDUDE_PORT	   = usb
AVRDUDE_PROGRAMMER = usbasp

MCU = atmega328p
AVR_FREQ = 16000000L

# AVRDude
AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -v
AVRDUDE_WRITE_FLASH = -U flash:w:$(OUTDIR)/$(PROGRAM).hex
AVRDUDE_ERASE_FLASH = -e
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

###############################################################################
#	COMPILER/LINKER PARAMETERS
###############################################################################

CC          = avr-gcc
CC_SIZE		= avr-size
OBJCOPY     = avr-objcopy
OBJDUMP     = avr-objdump

OPTIMIZE   	= -Os
VERBOSE 	= -v
DEBUGSYMB	= -g
#LDSECTION  = --section-start=.text=0x7800
LDMAP 		+= -Map,./$(OUTDIR)/$(PROGRAM).map

#CFLAGS    = $(DEBUGSYMB) -Wall $(OPTIMIZE) -mmcu=$(MCU) -DF_CPU=AVR_FREQ $(INC)
CFLAGS    = $(DEBUGSYMB) -Wall $(OPTIMIZE) -mmcu=$(MCU) $(INC)
LDFLAGS   = -Wl,$(LDMAP)

CSIZE_FLAGS_AVR	= -Cd --mcu=$(MCU)
CSIZE_FLAGS_SYS	= -Ad

# Intermix source code with disassembly. Test -d and -h flags to explore the output
OBJDUMP_FLAGS = -h -S
OBJCOPY_FLAGS_HEX 	= -j .text -j .data -O ihex
OBJCOPY_FLAGS_SREC 	= -j .text -j .data -O srec
OBJCOPY_FLAGS_BIN 	= -j .text -j .data -O binary

###############################################################################
#	MAKEFILE RULES
###############################################################################

.PHONY: build program program_fuses poke clean erase hello

$(OUTDIR):
	mkdir -p ./$(OUTDIR)

hello:
	@echo $(SRC)
	@echo $(INC)

build: $(OUTDIR) $(PROGRAM).hex
	@echo
	@echo ">> Build Finished =)"

program: $(OUTDIR) $(PROGRAM).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH) $(AVRDUDE_WRITE_EEPROM)	

program_fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(SAFEMODE) $(AVRDUDE_WRITE_FUSES)

poke:
	$(AVRDUDE) $(AVRDUDE_FLAGS)

erase:
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_ERASE_FLASH)

clean:
	rm $(OUTDIR)/*
	rmdir $(OUTDIR)

# FILES -----------------------------------------------------------------------

%.elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o ./$(OUTDIR)/$@ $(addprefix ./$(OUTDIR)/,$^)
	@echo
	@echo " < SYSV SIZE REPORT >"
	@echo
	@$(CC_SIZE) $(CSIZE_FLAGS_SYS) ./$(OUTDIR)/$(PROGRAM).elf
	@$(CC_SIZE) $(CSIZE_FLAGS_AVR) ./$(OUTDIR)/$(PROGRAM).elf

%.hex: %.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS_HEX) ./$(OUTDIR)/$< ./$(OUTDIR)/$@

# Alternative assembly LISTING output format  
%.lst: %.elf
	$(OBJDUMP) $(OBJDUMP_FLAGS) ./$(OUTDIR)/$< > ./$(OUTDIR)/$@
# Alternative Motorola S-Record output format
%.srec: %.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS_SREC) ./$(OUTDIR)/$< ./$(OUTDIR)/$@
# Alternative Binary output format
%.bin: %.elf
	$(OBJCOPY) $(OBJCOPY_FLAGS_BIN) ./$(OUTDIR)/$< ./$(OUTDIR)/$@

# UTILITY RULES ---------------------------------------------------------------

# Dependency files 
%.dep: %.c $(OUTDIR)
	@echo " >> Creating DEPENDENCY file"
	$(CC) $(CFLAGS) -M -o ./$(OUTDIR)/$@ $<

# Preprocessed files
%.i: %.c $(OUTDIR)
	@echo " >> Creating PREPROCESSED file"
	$(CC) $(CFLAGS) -E -o ./$(OUTDIR)/$@ $<

# Assembly files
%.asm: %.c $(OUTDIR)
	@echo " >> Creating ASSEMBLY file"
	$(CC) $(CFLAGS) -S -o ./$(OUTDIR)/$@ $<

# Object files
%.o: %.c $(OUTDIR)
	@echo " >> Creating OBJECT file"
	$(CC) $(CFLAGS) -c -o ./$(OUTDIR)/$@ $<