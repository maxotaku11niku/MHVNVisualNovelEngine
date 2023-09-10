# Directories
TARGET	:= bin
BUILD	:= obj
SOURCES	:= src
GENASM	:= genasm
# Find source files
export CFILES	:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export FULLCFILES	:= $(addprefix $(SOURCES)/,$(CFILES))
export GENASMFILES	:= $(addprefix $(GENASM)/,$(CFILES:.c=.asm))

.PHONY : win64rel, win64dbg, showasm

win64rel : $(FULLCFILES)
	gcc -o $(TARGET)/mhvnlink.exe -O3 -Wl,-O,-s,--gc-sections $^

win64dbg : $(FULLCFILES)
	gcc -o $(TARGET)/mhvnlink.exe -Og $^

showasm	: $(GENASMFILES)

$(GENASM)/%.asm	: $(SOURCES)/%.c
	gcc $< -S -o $@ -O3 -c