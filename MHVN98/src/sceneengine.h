#pragma once

#define SCENE_STATUS_RENDERTEXT    0x0001
#define SCENE_STATUS_WIPETEXT      0x0002
#define SCENE_STATUS_MAKING_CHOICE 0x0004
#define SCENE_STATUS_FINALEND      0x4000
#define SCENE_STATUS_ERROR         0x8000

#define SELECT_UP   (-1)
#define SELECT_DOWN   1

typedef struct
{
    unsigned short numScenes;
    unsigned short numChars;
    unsigned short curScene;
} SceneInfo;

extern SceneInfo sceneInfo;

//Sets up the scene engine, loading the first scene in
int SetupSceneEngine();
//Frees all dynamically allocated data associated with the scene engine
int FreeSceneEngine();
//Controls whether or not to process the scene script from above
void ControlProcess(unsigned char process);
//Switches choice to another option when deciding between options
void SwitchChoice(char dir);
//Commits choice and moves forward
void CommitChoice();
//Scene script process loop
int SceneDataProcess();
