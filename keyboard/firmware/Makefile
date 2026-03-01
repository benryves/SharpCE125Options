###############################################################################
# Makefile for the project SharpCE125-Keyboard
###############################################################################

## General Flags
PROJECT = keyboard
MCU = attiny84
F_CPU = 8000000
TARGET = $(PROJECT).elf
CC = avr-gcc
AVRDUDE = avrdude -p $(MCU)

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=$(F_CPU)UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=$(PROJECT).map

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

COMPILE = $(CC) $(INCLUDES) $(CFLAGS)

OBJECTS = main.o sharp.o keyboard.o

## Build
all: $(TARGET) $(PROJECT).elf $(PROJECT).hex size

## Compile
.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(PROJECT).elf dep $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss $(PROJECT).map

flash: all
	$(AVRDUDE) -U flash:w:$(PROJECT).hex:i


fuses:
ifeq ($(MCU),attiny4313)
	$(AVRDUDE) -U lfuse:w:0xe4:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
endif

ifneq (,$(filter $(MCU),attiny84 attiny44))
	$(AVRDUDE) -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m 
endif
	
## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

