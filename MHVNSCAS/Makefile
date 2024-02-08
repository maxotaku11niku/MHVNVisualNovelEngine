# Directories
TARGET  := bin
EXE     := mhvnscas
BUILD   := obj
SOURCES := src
GENASM  := genasm
# Find source files
export CFILES       := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export FULLCFILES   := $(addprefix $(SOURCES)/,$(CFILES))
export CPPFILES     := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
export FULLCPPFILES := $(addprefix $(SOURCES)/,$(CPPFILES))
export HEADERS      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.h)))
export FULLHEADERS  := $(addprefix $(SOURCES)/,$(HEADERS))
export GENASMFILES  := $(addprefix $(GENASM)/,$(CFILES:.c=.asm) $(CPPFILES:.cpp=.asm))
export OFILESC      := $(CFILES:.c=.o)
export OFILESCPP    := $(CPPFILES:.cpp=.o)
export OFILES       := $(OFILESC) $(OFILESCPP)
export FULLOFILES   := $(addprefix $(BUILD)/,$(OFILES))
# Flags and libraries
export CFLAGSBASE       :=
export CFLAGSDEBUG      := $(CFLAGSBASE) -Og
export CFLAGSRELEASE    := $(CFLAGSBASE) -O3 -flto
export CFLAGS           := $(CFLAGSBASE)
export CXXFLAGS         :=
export LINKFLAGSBASE    :=
export LINKFLAGSDEBUG   := $(LINKFLAGSBASE)
export LINKFLAGSRELEASE := $(LINKFLAGSBASE) -Wl,-s
export LINKFLAGS        := $(LINKFLAGSBASE)
export LINKLIBS         :=

# Define some variable overrides
default : CFLAGS = $(CFLAGSRELEASE)
native : CFLAGS = $(CFLAGSRELEASE) -march=native
debug : CFLAGS = $(CFLAGSDEBUG)

default : LINKFLAGS = $(LINKFLAGSRELEASE)
native : LINKFLAGS = $(LINKFLAGSRELEASE)
debug : LINKFLAGS = $(LINKFLAGSDEBUG)

.PHONY : default native debug showasm clean cleanasm install-linux

all : default

#Builds for the current host OS, but assumes a basic variant of the current processor architecture
default : $(BUILD) $(TARGET) $(TARGET)/$(EXE)

#Builds specifically for this machine
native : $(BUILD) $(TARGET) $(TARGET)/$(EXE)

#Build a debug-friendly version
debug : $(BUILD) $(TARGET) $(TARGET)/$(EXE)

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

#Copies output executable to /usr/local/bin/ (with confirmation)
install-linux : $(TARGET)/$(EXE)
	sudo cp $^ /usr/local/bin/

$(BUILD) :
	mkdir -p $(BUILD)

$(TARGET) :
	mkdir -p $(TARGET)

$(GENASM) :
	mkdir -p $(GENASM)

$(TARGET)/$(EXE) : $(FULLOFILES)
	g++ $(CFLAGS) $(CXXFLAGS) $(LINKFLAGS) -o $@ $^ $(LINKLIBS)

$(FULLOFILES) : $(FULLCFILES) $(FULLCPPFILES) $(FULLHEADERS)

$(BUILD)/%.o : $(SOURCES)/%.c
	gcc -c $(CFLAGS) -o $@ $<

$(BUILD)/%.o : $(SOURCES)/%.cpp
	g++ -c $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(GENASM)/%.asm	: $(SOURCES)/%.c
	gcc -c -O3 -S -o $@ $<

$(GENASM)/%.asm	: $(SOURCES)/%.cpp
	g++ -c -O3 $(CXXFLAGS) -S -o $@ $<
