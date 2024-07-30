# Directories
TARGET   := out
BUILD    := obj
SCENESRC := scene
IMAGES   := images
ENGBTEXT := en_GB
JAJPTEXT := ja_JP
# Handle filenames
export SCENEFILES	:= $(foreach dir,$(SCENESRC),$(wildcard $(dir)/*.sca))
export GBENGLISHFILES	:= $(foreach dir,$(ENGBTEXT),$(wildcard $(dir)/*.txt))
export JAPANESEFILES	:= $(foreach dir,$(JAJPTEXT),$(wildcard $(dir)/*.txt))
export OBJFILESTXA	:= EN_GB.otxa JA_JP.otxa
export OBJFILESSCX	:= SCENE.odat
export OBJFILESIMG	:= BGIMAGE.odat SPRITE.odat
export OBJFILES	:= $(addprefix $(BUILD)/, $(OBJFILESTXA) $(OBJFILESSCX) $(OBJFILESIMG))

all : $(OBJFILES)
	tools/mhvnlink -o $(TARGET) -d masterdesc.txt -s $(BUILD)/SCENE.odat -t $(BUILD)/EN_GB.otxa -t $(BUILD)/JA_JP.otxa -bg $(BUILD)/BGIMAGE.odat -spr $(BUILD)/SPRITE.odat -m $(BUILD)/MUSIC.odat -sfx $(BUILD)/SOUNDFX.odat -sys $(BUILD)/SYSTEM.odat

$(BUILD)/SCENE.odat : $(SCENEFILES)
	tools/mhvnscas -o $@ $^

$(BUILD)/BGIMAGE.odat : $(IMAGES)/bglist.txt
	tools/mhvnimgp -o $@ -b $^

$(BUILD)/SPRITE.odat : $(IMAGES)/sprlist.txt
	tools/mhvnimgp -o $@ -s $^

$(BUILD)/EN_GB.otxa : $(GBENGLISHFILES)
	tools/mhvntxar -o $@ $^

$(BUILD)/JA_JP.otxa : $(JAPANESEFILES)
	tools/mhvntxar -o $@ $^
