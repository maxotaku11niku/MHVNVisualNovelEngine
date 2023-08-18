# Directories
TARGET	:= bin
BUILD	:= obj
PROCESS	:= objp
SOURCES	:= src
GENASM	:= genasm
# Find source files
export ASMFILES		:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.asm)))
export CFILES	:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export OFILESC	:= $(addprefix $(BUILD)/,$(CFILES:.c=.o))
export OFILESASM	:= $(addprefix $(BUILD)/,$(ASMFILES:.asm=.o))
export OFILES	:= $(addprefix $(BUILD)/,$(ASMFILES:.asm=.o) $(CFILES:.c=.o))
export GENASMFILES	:= $(addprefix $(GENASM)/,$(CFILES:.c=.asm))
export SOURCEFILES	:= $(addprefix $(SOURCES)/,$(ASMFILES) $(CFILES))
export CFILESWD	:= $(addprefix $(SOURCES)/,$(CFILES))
export ASMFILESWD	:= $(addprefix $(SOURCES)/,$(ASMFILES))
# Targets
all: $(PROCESS) $(BUILD) $(TARGET)/MHVN98.COM

.PHONY: showasm

showasm	: $(GENASMFILES)

$(BUILD):
	[ -d $(BUILD) ] || mkdir -p $(BUILD)
	make -f $(CURDIR)/Makefile

$(PROCESS):
	[ -d $(PROCESS) ] || mkdir -p $(PROCESS)
	make -f $(CURDIR)/Makefile

$(TARGET):
	[ -d $(TARGET) ] || mkdir -p $(TARGET)
	make -f $(CURDIR)/Makefile

	
$(TARGET)/MHVN98.COM	: $(PROCESS)/mhvn98.o
	objcopy -O binary $< $@

$(PROCESS)/mhvn98.o	: $(OFILES)
	ld -T doscom.lds -m i386pe -M -o $(PROCESS)/mhvn98.o $^

$(BUILD)/%.o: $(SOURCES)/%.asm
	as $< -o $@ -march=i386 -mtune=i386 --32

$(BUILD)/%.o: $(SOURCES)/%.c
	gcc $< -o $@ -nostdlib -m16 -Os -march=i386 -mtune=i386 -c -Wa,--32

$(GENASM)/%.asm	: $(SOURCES)/%.c
	gcc $< -S -o $@ -nostdlib -m16 -Os -march=i386 -mtune=i386 -c