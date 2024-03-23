# Directories
TARGET   := bin
BUILD    := obj
MAINSRC  := src
PLATFORM := src/platform
SOURCES  := $(MAINSRC) $(PLATFORM)
GENASM   := genasm
# Find source files
export ASMFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.asm)))
export CFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export HEADERS     := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.h)))
export OFILESC     := $(addprefix $(BUILD)/,$(CFILES:.c=.o))
export OFILESASM   := $(addprefix $(BUILD)/,$(ASMFILES:.asm=.o))
export OFILES      := $(OFILESC) $(OFILESASM)
export GENASMFILES := $(addprefix $(GENASM)/,$(CFILES:.c=.asm))
export ASMFILESWD  := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.asm))
export CFILESWD    := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
export HEADERSWD   := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.h))
export SOURCEFILES := $(ASMFILESWD) $(CFILESWD)

# Flags
export CFLAGSBASE       := -march=i286 -mcmodel=small
export CFLAGSRELEASE    := $(CFLAGSBASE) -O2
export CFLAGSDEBUG      := $(CFLAGSBASE) -O2
export CFLAGS           := $(CFLAGSBASE)
export LINKFLAGSBASE    := -Wl,-melf_i386_msdos_mz,-M,-static,--relax,-n,--no-dynamic-linker,--print-memory-usage
export LINKFLAGSRELEASE := $(LINKFLAGSBASE)
export LINKFLAGSDEBUG   := $(LINKFLAGSBASE)
export LINKFLAGS        := $(LINKFLAGSBASE)
export LINKLIBS         := -li86

# Define some variable overrides
default : CFLAGS = $(CFLAGSRELEASE)
debug : CFLAGS = $(CFLAGSDEBUG)

default : LINKFLAGS = $(LINKFLAGSRELEASE)
debug : LINKFLAGS = $(LINKFLAGSDEBUG)

.PHONY: default debug showasm clean cleanasm

all : default

#Builds an optmised executable, but it will be horrible for debugging purposes
default : $(BUILD) $(TARGET) wholeprog.o $(OFILESASM)
	ia16-elf-gcc $(CFLAGS) $(LINKFLAGS) -o $(TARGET)/MHVN98.EXE wholeprog.o $(OFILESASM) $(LINKLIBS)

#Build a debug-friendly version (at least, it's much easier to use a memory viewer to locate stuff, it's not your usual debug executable)
debug :  $(BUILD) $(TARGET) $(TARGET)/MHVN98.EXE

#Compiles the source files to assembly language for curiosity's sake
showasm : $(GENASM) $(GENASMFILES)

#Cleans the build directory
clean : $(BUILD) $(TARGET)
	rm -rf $(BUILD)
	rm -rf $(TARGET)
	mkdir -p $(BUILD)
	mkdir -p $(TARGET)

#Removes the generated assembly files
cleanasm : $(GENASM)
	rm -rf $(GENASM)

$(BUILD):
	mkdir -p $(BUILD)

$(PROCESS):
	mkdir -p $(PROCESS)

$(TARGET):
	mkdir -p $(TARGET)

$(GENASM):
	mkdir -p $(GENASM)

$(TARGET)/MHVN98.EXE : $(OFILES)
	ia16-elf-gcc $(CFLAGS) $(LINKFLAGS) -o $@ $^ $(LINKLIBS)

$(OFILES) : $(SOURCEFILES) $(HEADERSWD)

wholeprog.o : wholeprog.c $(CFILESWD) $(HEADERSWD)
	ia16-elf-gcc wholeprog.c -o wholeprog.o $(CFLAGS) -fwhole-program -c

$(BUILD)/%.o : $(MAINSRC)/%.asm
	ia16-elf-as $< -o $@ -O2

$(BUILD)/%.o : $(MAINSRC)/%.c
	ia16-elf-gcc $< -o $@ $(CFLAGS) -c

$(GENASM)/%.asm : $(MAINSRC)/%.c
	ia16-elf-gcc $< -S -o $@ $(CFLAGSRELEASE) -c

$(BUILD)/%.o : $(PLATFORM)/%.asm
	ia16-elf-as $< -o $@ -O2

$(BUILD)/%.o : $(PLATFORM)/%.c
	ia16-elf-gcc $< -o $@ $(CFLAGS) -c

$(GENASM)/%.asm : $(PLATFORM)/%.c
	ia16-elf-gcc $< -S -o $@ $(CFLAGSRELEASE) -c
