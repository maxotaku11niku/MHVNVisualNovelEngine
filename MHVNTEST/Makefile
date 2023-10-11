# Directories
TARGET	:= out
BUILD	:= obj
SCENESRC	:= scene
ENGBTEXT	:= en_GB
JAJPTEXT	:= ja_JP
# Handle filenames
export SCENEFILES	:= $(foreach dir,$(SCENESRC),$(wildcard $(dir)/*.sca))
export GBENGLISHFILES	:= $(foreach dir,$(ENGBTEXT),$(wildcard $(dir)/*.txt))
export JAPANESEFILES	:= $(foreach dir,$(JAJPTEXT),$(wildcard $(dir)/*.txt))
export OBJFILESTXA	:= EN_GB.otxa JA_JP.otxa
export OBJFILESSCX	:= SCENE.odat
export OBJFILES	:= $(addprefix $(BUILD)/, $(OBJFILESTXA) $(OBJFILESSCX))

#MHVNLINK's flags haven't been defined yet
all : OBJFILES
	tools/mhvnlink

$(BUILD)/SCENE.odat : SCENEFILES
	tools/mhvnscas -o $@ $^

$(BUILD)/EN_GB.otxa : GBENGLISHFILES
	tools/mhvntxar -o $@ $^

$(BUILD)/JA_JP.otxa : JAPANESEFILES
	tools/mhvntxar -o $@ -s $^