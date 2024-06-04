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
export OBJFILESTXA	:= EN_GB.otxa
export OBJFILESSCX	:= SCENE.odat
export OBJFILES	:= $(addprefix $(BUILD)/, $(OBJFILESTXA) $(OBJFILESSCX))

all : $(OBJFILES)
	tools/mhvnlink -o $(TARGET) -d masterdesc.txt -s $(BUILD)/SCENE.odat -t $(BUILD)/EN_GB.otxa -bg $(BUILD)/BGIMAGE.odat -spr $(BUILD)/SPRITE.odat -m $(BUILD)/MUSIC.odat -sfx $(BUILD)/SOUNDFX.odat -sys $(BUILD)/SYSTEM.odat

$(BUILD)/SCENE.odat : $(SCENEFILES)
	tools/mhvnscas -o $@ $^

$(BUILD)/EN_GB.otxa : $(GBENGLISHFILES)
	tools/mhvntxar -o $@ $^

$(BUILD)/JA_JP.otxa : $(JAPANESEFILES)
	tools/mhvntxar -o $@ $^
