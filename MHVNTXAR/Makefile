# Directories
TARGET	:= bin
BUILD	:= obj
SOURCES	:= src
# Find source files
export CFILES	:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))

.PHONY : win64rel

win64rel : $(SOURCES)/$(CFILES)
	gcc -o $(TARGET)/mhvntxar.exe -O3 -fwhole-program -Wl,-O,-s,--gc-sections $<