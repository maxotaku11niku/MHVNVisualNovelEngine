/* MHVN98 - PC98 executable for the MHVN Visual Novel Engine
 * Copyright (c) 2023-2024 Maxim Hoxha
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Scene bytecode loader and interpreter
 */

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
//Signals to the scene processor that the user has responded and now is the time to move on
void EndUserWait();
//Processes any async actions, should be called every frame
void SceneAsyncActionProcess();
//Scene script process loop
int SceneDataProcess();
