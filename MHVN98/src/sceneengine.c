#include "x86strops.h"
#include "filehandling.h"
#include "pc98_egc.h"
#include "rootinfo.h"
#include "sceneengine.h"
#include "textengine.h"

#define VMFLAG_Z 0x01
#define VMFLAG_N 0x02
#define VMFLAG_TEXTINBOX 0x40
#define VMFLAG_PROCESS 0x80

SceneInfo sceneInfo;
unsigned char curSceneData[1024];
unsigned short curSceneDataPC;
unsigned char vmFlags;
int returnStatus;
unsigned int curCharNum;
unsigned int nextTextNum;
char curCharName[64];
char* curTextArray[256];
char sceneTextBuffer[1024];
int sdHandle;

int loadNewScene(unsigned int sceneNum)
{
    if (sceneNum == sceneInfo.curScene) return 0; //Don't change scene if scene number isn't going to change
    int realReadLen;
    unsigned long curfilepos;
    unsigned long scenedatpos;
    int result = openFile(rootInfo.sceneDataPath, FILE_OPEN_READ, &sdHandle);
    if (result)
	{
		writeString("Error! Could not find scene data file!", 168, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
		return result; //Error handler
	}
    seekFile(sdHandle, FILE_SEEK_ABSOLUTE, 4 + 4 * sceneNum, &curfilepos);
    readFile(sdHandle, 4, &scenedatpos, &realReadLen);
    seekFile(sdHandle, FILE_SEEK_ABSOLUTE, 4 + 4 * sceneInfo.numScenes + scenedatpos, &curfilepos);
    readFile(sdHandle, 1024, curSceneData, &realReadLen);
    closeFile(sdHandle);
    result = loadSceneText(sceneNum, sceneTextBuffer, curTextArray);
    if (result) return result;
    curSceneDataPC = 0;
    returnStatus = 0;
    curCharNum = 0xFFFF;
    vmFlags = VMFLAG_PROCESS;
    return 0;
}

int setupSceneEngine()
{
    int realReadLen;
    int result = openFile(rootInfo.sceneDataPath, FILE_OPEN_READ, &sdHandle);
	if (result)
	{
		writeString("Error! Could not find scene data file!", 168, 184, FORMAT_SHADOW | FORMAT_FONT_DEFAULT | FORMAT_COLOUR(0xF), 0);
		return result; //Error handler
	}
	readFile(sdHandle, 4, smallFileBuffer, &realReadLen);
	sceneInfo.numScenes = *((unsigned short*)(smallFileBuffer));
	sceneInfo.numChars = *((unsigned short*)(smallFileBuffer + 0x02));
    //sceneTextBuffer = (char*)0x0010000; //temporary, also relies on being in unreal mode
    sceneInfo.curScene = 0xFFFF;
    closeFile(sdHandle);
    return loadNewScene(0);
}

void controlProcess(int process)
{
    if (process) vmFlags |= VMFLAG_PROCESS;
    else vmFlags &= ~VMFLAG_PROCESS;
}

void clearTextBox()
{
    clearLinesEGC(textBoxtY, textBoxbY - textBoxtY + 1);
    egc_patdatandreadmode(EGC_PATTERNSOURCE_FGCOLOUR);
    egc_rwmode(EGC_WRITE_ROPSHIFT | EGC_SOURCE_CPU | EGC_ROP((EGC_ROP_SRC & EGC_ROP_PAT) | ((~EGC_ROP_SRC) & EGC_ROP_DST)));
    egc_bitaddrbtmode(EGC_BLOCKTRANSFER_FORWARD);
    egc_bitlen(32);
}

void clearCharacterName()
{
    clearLinesEGC(60, 17);
    egc_patdatandreadmode(EGC_PATTERNSOURCE_FGCOLOUR);
    egc_rwmode(EGC_WRITE_ROPSHIFT | EGC_SOURCE_CPU | EGC_ROP((EGC_ROP_SRC & EGC_ROP_PAT) | ((~EGC_ROP_SRC) & EGC_ROP_DST)));
    egc_bitaddrbtmode(EGC_BLOCKTRANSFER_FORWARD);
    egc_bitlen(32);
}

int sceneDataProcess()
{
    unsigned char curOpcode;
    int result = 0;
    while (vmFlags & VMFLAG_PROCESS)
    {
        returnStatus = 0;
        curOpcode = curSceneData[curSceneDataPC++];
        switch (curOpcode)
        {
        case 0x00: //gotoscene
            unsigned int sNum = *((unsigned short*)(curSceneData + curSceneDataPC));
            if (sNum == 0xFFFF) //scene number FFFF is a proxy for the end of the whole VN
            {
                vmFlags &= ~VMFLAG_PROCESS;
                returnStatus |= SCENE_STATUS_FINALEND;
                break;
            }
            result = loadNewScene(sNum);
            if (result)
            {
                returnStatus = SCENE_STATUS_ERROR | result;
                vmFlags &= ~VMFLAG_PROCESS;
                break;
            }
            curSceneDataPC += 2;
            break;
        case 0x01: //jmp
            curSceneDataPC += 2;
            vmJump:
            curSceneDataPC += *((unsigned short*)(curSceneData + curSceneDataPC - 2));
            break;
        case 0x02: //jz, je
            curSceneDataPC += 2;
            if (vmFlags & VMFLAG_Z) goto vmJump;
            else break;
        case 0x03: //jnz, jne
            curSceneDataPC += 2;
            if (vmFlags & VMFLAG_Z) break;
            else goto vmJump;
        case 0x04: //jn, jl
            curSceneDataPC += 2;
            if (vmFlags & VMFLAG_N) goto vmJump;
            else break;
        case 0x05: //jp, jge
            curSceneDataPC += 2;
            if (vmFlags & VMFLAG_N) break;
            else goto vmJump;
        case 0x06: //jle
            curSceneDataPC += 2;
            if (vmFlags & (VMFLAG_Z | VMFLAG_N)) goto vmJump;
            else break;
        case 0x07: //jg
            curSceneDataPC += 2;
            if (vmFlags & (VMFLAG_Z | VMFLAG_N)) break;
            else goto vmJump;
        case 0x11: //text
            if (vmFlags & VMFLAG_TEXTINBOX)
            {
                curSceneDataPC--; //compensation
                goto delText;
            }
            nextTextNum = *((unsigned short*)(curSceneData + curSceneDataPC));
            curSceneDataPC += 2;
        case 0x10: //nexttext
            if (vmFlags & VMFLAG_TEXTINBOX)
            {
                curSceneDataPC--; //compensation
                goto delText;
            }
            startAnimatedStringToWrite(curTextArray[nextTextNum], textBoxlX, textBoxtY, rootInfo.defFormatNormal);
            nextTextNum++;
            vmFlags |= VMFLAG_TEXTINBOX;
            vmFlags &= ~VMFLAG_PROCESS;
            returnStatus |= SCENE_STATUS_RENDERTEXT;
            break;
        case 0x12: //charname
            unsigned int charNum = *((unsigned short*)(curSceneData + curSceneDataPC));
            if (charNum != curCharNum)
            {
                clearCharacterName();
                if (charNum != 0xFFFF)
                {
                    result = loadCurrentCharacterName(charNum, curCharName);
                    if (result)
                    {
                        returnStatus = SCENE_STATUS_ERROR | result;
                        vmFlags &= ~VMFLAG_PROCESS;
                        break;
                    }
                    writeString(curCharName, 60, 60, rootInfo.defFormatCharName, 0);
                }
                curCharNum = charNum;
            }
            curSceneDataPC += 2;
            break;
        case 0x1F: //deltext
            delText:
            clearTextBox();
            vmFlags &= ~VMFLAG_TEXTINBOX;
            vmFlags &= ~VMFLAG_PROCESS;
            returnStatus |= SCENE_STATUS_WIPETEXT; //for later, when text box wiping involves an animation
            break;
        default: //illegal/unimplemented opcode
            break;
        }
    }
    return returnStatus;
}