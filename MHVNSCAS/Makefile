# Directories
TARGET	:= bin
BUILD	:= obj
SOURCES	:= src
GENASM	:= genasm
# Find source files
export CFILES	:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export FULLCFILES	:= $(addprefix $(SOURCES)/,$(CFILES))
export CPPFILES	:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
export FULLCPPFILES	:= $(addprefix $(SOURCES)/,$(CPPFILES))
export GENASMFILESC	:= $(addprefix $(GENASM)/,$(CFILES:.c=.asm))
export GENASMFILESCPP	:= $(addprefix $(GENASM)/,$(CPPFILES:.cpp=.asm))

.PHONY : win64rel, win64dbg, showasm

all : $(FULLCFILES) $(FULLCPPFILES)
	g++ -o $(TARGET)/mhvnscas.exe -O3 -march=native -Wl,-O,-s,--gc-sections $^

win64rel : $(FULLCFILES) $(FULLCPPFILES)
	g++ -o $(TARGET)/mhvnscas.exe -O3 -Wl,-O,-s,--gc-sections $^

win64dbg : $(FULLCFILES) $(FULLCPPFILES)
	g++ -o $(TARGET)/mhvnscas.exe -Og $^

showasm	: $(GENASMFILESC) $(GENASMFILESCPP)

$(GENASM)/%.asm	: $(SOURCES)/%.c
	gcc $< -S -o $@ -O3 -c

$(GENASM)/%.asm	: $(SOURCES)/%.cpp
	g++ $< -S -o $@ -O3 -c