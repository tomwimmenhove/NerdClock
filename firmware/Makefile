TARGET_NAME = main
BUILD_DIR = build
SRC_DIRS = src
CPP = avr-g++
DEVICE = atmega328p
THE_DUDE = avrdude
#THE_DUDEARGS = -q -q -p$(DEVICE) -cusbasp -Pusb
THE_DUDEARGS = -q -q -patmega328p -carduino -P/dev/ttyUSB0 -b57600 -D

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CPPFLAGS = -g -DF_CPU=16000000 -Wall -Os -Werror -Wextra -mmcu=$(DEVICE) -std=c++11 -MMD -MP
LDFLAGS  = -Wl,-Map,$(BUILD_DIR)/$(TARGET_NAME).map

all: $(BUILD_DIR)/$(TARGET_NAME).hex

$(BUILD_DIR)/$(TARGET_NAME).hex: $(BUILD_DIR)/$(TARGET_NAME).elf
	avr-objcopy -j .text -j .data -O ihex $(BUILD_DIR)/$(TARGET_NAME).elf $(BUILD_DIR)/$(TARGET_NAME).hex

$(BUILD_DIR)/$(TARGET_NAME).elf: $(OBJS)
	$(CPP) $(LDFLAGS) $(CPPFLAGS) $(OBJS) -o $(BUILD_DIR)/$(TARGET_NAME).elf

flash: $(BUILD_DIR)/$(TARGET_NAME).hex
	$(THE_DUDE) $(THE_DUDEARGS) -Uflash:w:$(BUILD_DIR)/$(TARGET_NAME).hex

$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CPP) $(CPPFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p

