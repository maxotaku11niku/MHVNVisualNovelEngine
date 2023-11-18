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
# Flags
export CFLAGS	:= -nostdlib -m16 -O2 -march=i386 -mtune=i386
# Targets
all: $(PROCESS) $(BUILD) $(TARGET) $(TARGET)/MHVN98.COM

.PHONY: showasm

showasm	: $(GENASM) $(GENASMFILES)

$(BUILD):
	mkdir -p $(BUILD)

$(PROCESS):
	mkdir -p $(PROCESS)

$(TARGET):
	mkdir -p $(TARGET)

$(GENASM):
	mkdir -p $(GENASM)

	
$(TARGET)/MHVN98.COM	: $(PROCESS)/mhvn98.o
	objcopy -O binary $< $@

#Apply some lovely whole program optimisation
$(PROCESS)/mhvn98.o	: $(OFILES)
	gcc $(CFLAGS) -flto -Wa,--32 -Wl,-Tdoscom.lds,-mi386pe,-M,-static,--relax,-n,--no-dynamic-linker,--print-memory-usage -o $(PROCESS)/mhvn98.o $^

$(BUILD)/%.o: $(SOURCES)/%.asm
	as $< -o $@ -march=i386 -mtune=i386 --32

$(BUILD)/%.o: $(SOURCES)/%.c
	gcc $< -o $@ $(CFLAGS) -flto -c -Wa,--32

$(GENASM)/%.asm	: $(SOURCES)/%.c
	gcc $< -S -o $@ $(CFLAGS) -c