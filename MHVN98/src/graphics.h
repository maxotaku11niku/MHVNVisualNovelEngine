#pragma once

typedef struct
{
    int x;
    int y;
} Vector2Int;

typedef struct
{
    Vector2Int pos;
    Vector2Int size;
} Rect2Int;

#define IMAGE_TYPE_NORMAL 0x00
#define IMAGE_TYPE_9SLICE 0x01
#define IMAGE_MEM_NORMAL  0x00
#define IMAGE_MEM_VRAM    0x02
#define IMAGE_ALIGN_FREE  0x00
#define IMAGE_ALIGN_FIXED 0x04
#define IMAGE_LOADED      0x80
#define IMAGE_DRAWN       0x40
#define IMAGE_DRAWREQ     0x20

typedef struct imginf
{
    Rect2Int boundRect;
    __far unsigned char* mask;
    __far unsigned char* plane0;
    __far unsigned char* plane1;
    __far unsigned char* plane2;
    __far unsigned char* plane3;
    struct imginf* children;
    unsigned short id;
    unsigned char layer;
    unsigned char flags;
} ImageInfo;

void UnloadImage(ImageInfo* img);
ImageInfo* LoadBGImage(unsigned int num);
void DoDrawRequests();
void LoadStd9SliceBoxIntoVRAM();
ImageInfo* RegisterTextBox(const Rect2Int* rect);
ImageInfo* RegisterCharNameBox(const Rect2Int* rect);
ImageInfo* RegisterChoiceBox(const Rect2Int* rect);
void Draw9SliceBoxInnerRegion(ImageInfo* img);
void InitialiseGraphicsSystem();
void FreeGraphicsSystem();
