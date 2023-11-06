#define SCENE_STATUS_RENDERTEXT 0x0001
#define SCENE_STATUS_WIPETEXT 0x0002
#define SCENE_STATUS_FINALEND 0x4000
#define SCENE_STATUS_ERROR 0x8000

typedef struct
{
	unsigned short numScenes;
	unsigned short numChars;
	unsigned short curScene;
} SceneInfo;

extern SceneInfo sceneInfo;

//Sets up the scene engine, loading the first scene in
int setupSceneEngine();
//Frees all dynamically allocated data associated with the scene engine
int freeSceneEngine();
//Controls whether or not to process the scene script from above
void controlProcess(int process);
//Scene script process loop
int sceneDataProcess();