//PMD interface macros
#pragma once

//INT 60 function 00 - Start Song
#define pmd_startsong() asm inline ("movb $0, %%ah\n\tint $96" : : : "ah")
//INT 60 function 01 - Stop Song
#define pmd_stopsong() asm inline ("movb $1, %%ah\n\tint $96" : : : "ah")
//INT 60 function 02 - Fade in/out
#define pmd_fade(amt) asm inline ("movb $2, %%ah\n\tint $96" : : "a" (amt))
//INT 60 function 03 - SSG Sound Effect Play
#define pmd_ssgeffectplay(num) asm inline ("movb $3, %%ah\n\tint $96" : : "a" (num))
//INT 60 function 04 - SSG Sound Effect Stop
#define pmd_ssgeffectstop() asm inline ("movb $4, %%ah\n\tint $96" : : : "ah")
//INT 60 function 05 - Get Measure Number
#define pmd_getmeasure(num) asm inline ("movb $5, %%ah\n\tint $96" : "a" (num) : )
//INT 60 function 06 - Get Song Data Pointer
#define pmd_getsongdataptr(seg, offset) asm inline ("movb $6, %%ah\n\tint $96\n\tmovw %%ds, %w1\n\tpush cs\n\tpop ds" : "=d" (offset), "=r" (seg) : : "ah")
//INT 60 function 07 - Get Instrument Data Pointer
#define pmd_getinstdataptr(seg, offset) asm inline ("movb $7, %%ah\n\tint $96\n\tmovw %%ds, %w1\n\tpush cs\n\tpop ds" : "=d" (offset), "=r" (seg) : : "ah")
//INT 60 function 08 - Get Overall Volume
#define pmd_gettotalvol(vol) asm inline ("movb $8, %%ah\n\tint $96\n\tmovb $0, %%ah" : "=a" (num) : )
//INT 60 function 09 - Get Driver Info
#define pmd_getdriverinfo(inf1, inf2) asm inline ("movb $9, %%ah\n\tint $96\n\t" : "=a" (inf1), "=d" (inf2) : )
//Supporting defines
#define PMD_TYPE_PMD     0x00
#define PMD_TYPE_PMDB2   0x01
#define PMD_TYPE_PMD86   0x02
#define PMD_TYPE_PMDL    0x03
#define PMD_TYPE_PMDPPZ  0x04
#define PMD_TYPE_PMDPPZE 0x05
#define PMD_TYPE_NOSOUND 0xFF
//INT 60 function 0A - Get Internal Status
#define pmd_getstatus(stat) asm inline ("movb $10, %%ah\n\tint $96" : "=a" (stat) : )
//Supporting defines
#define PMD_STATUS_LOOPNUM(stat) ((stat) & 0x00FF)
#define PMD_STATUS_MMLTILDEVALUE(stat) (((stat) & 0xFF00) >> 8)
//INT 60 function 0B - Get FM Effect Data Pointer
#define pmd_getfmfxdataptr(seg, offset) asm inline ("movb $11, %%ah\n\tint $96\n\tmovw %%ds, %w1\n\tpush cs\n\tpop ds" : "=d" (offset), "=r" (seg) : : "ah")
//INT 60 function 0C - FM Sound Effect Play
#define pmd_fmeffectplay(num) asm inline ("movb $12, %%ah\n\tint $96" : : "a" (num))
//INT 60 function 0D - FM Sound Effect Stop
#define pmd_fmeffectstop() asm inline ("movb $13, %%ah\n\tint $96" : : : "ah")
//INT 60 function 0E - Get PCM Table Pointer
#define pmd_getpcmtableptr(seg, offset) asm inline ("movb $14, %%ah\n\tint $96\n\tmovw %%ds, %w1\n\tpush cs\n\tpop ds" : "=d" (offset), "=r" (seg) : : "ah")
//INT 60 function 0F - PCM Sound Effect Play
#define pmd_pcmeffectplay(num, freq, volpan) asm inline ("movb $15, %%ah\n\tint $96" : : "a" (num), "d" (freq), "c" (volpan))
//INT 60 function 10 - Get Work Area Pointer
#define pmd_getworkareaptr(seg, offset) asm inline ("movb $16, %%ah\n\tint $96\n\tmovw %%ds, %w1\n\tpush cs\n\tpop ds" : "=d" (offset), "=r" (seg) : : "ah")
//INT 60 function 11 - Get Playing FM Sound Effect
#define pmd_getplayingfmfx(num) asm inline ("movb $17, %%ah\n\tint $96\n\tmovb $0, %%ah" : "=a" (num) : )
//INT 60 function 12 - Get Playing PCM Sound Effect
#define pmd_getplayingpcmfx(num) asm inline ("movb $18, %%ah\n\tint $96\n\tmovb $0, %%ah" : "=a" (num) : )
//INT 60 function 13 - Set TimerB Interrupt
#define pmd_setinttimerb(seg, offset) asm inline ("movb $19, %%ah\n\tmovw %w0, %%ds\n\tint $96\n\tmovw %%cs, %w0\n\tmovw %w0, %%ds" : "+r" (seg) : "d" (offset))
//INT 60 function 14 - Set TimerA Interrupt
#define pmd_setinttimera(seg, offset) asm inline ("movb $20, %%ah\n\tmovw %w0, %%ds\n\tint $96\n\tmovw %%cs, %w0\n\tmovw %w0, %%ds" : "+r" (seg) : "d" (offset))
//INT 60 function 15 - Get Playing SSG Sound Effect
#define pmd_getplayingssgfx(num) asm inline ("movb $21, %%ah\n\tint $96" : "=a" (num) : )
//INT 60 function 16 - Get Joystick Status
#define pmd_getjoystatus(stat) asm inline ("movb $22, %%ah\n\tint $96" : "=a" (stat) : )
//Supporting defines
#define PMD_JOYSTICK1_STATUS_UP    0x0001
#define PMD_JOYSTICK1_STATUS_DOWN  0x0002
#define PMD_JOYSTICK1_STATUS_LEFT  0x0004
#define PMD_JOYSTICK1_STATUS_RIGHT 0x0008
#define PMD_JOYSTICK1_STATUS_B     0x0010
#define PMD_JOYSTICK1_STATUS_A     0x0020
#define PMD_JOYSTICK2_STATUS_UP    0x0100
#define PMD_JOYSTICK2_STATUS_DOWN  0x0200
#define PMD_JOYSTICK2_STATUS_LEFT  0x0400
#define PMD_JOYSTICK2_STATUS_RIGHT 0x0800
#define PMD_JOYSTICK2_STATUS_B     0x1000
#define PMD_JOYSTICK2_STATUS_A     0x2000
//INT 60 function 17 - Get PPSDRV Compatibility Mode Status
#define pmd_getppsdrvmode(stat) asm inline ("movb $23, %%ah\n\tint $96\n\tmovb $0, %%ah" : "=a" (stat) : )
//INT 60 function 18 - Set PPSDRV Compatibility Mode
#define pmd_setppsdrvmode(stat) asm inline ("movb $24, %%ah\n\tint $96" : : "a" (stat))
//Supporting defines
#define PMD_PPSDRV_OFF 0
#define PMD_PPSDRV_ON  1
//INT 60 function 19 - Set Overall Volume
#define pmd_settotalvol(vol) asm inline ("movb $25, %%ah\n\tint $96" : : "a" (vol))
//INT 60 function 1A - Pause
#define pmd_pause() asm inline ("movb $26, %%ah\n\tint $96" : : : "ah")
//INT 60 function 1B - Unpause
#define pmd_unpause() asm inline ("movb $27, %%ah\n\tint $96" : : : "ah")
//INT 60 function 1C - Set Measure Number
#define pmd_setmeasure(result, num) asm inline ("movb $28, %%ah\n\tint $96\n\tmovb $0, %%ah" : "=a" (result) : "d" (num))
//Supporting defines
#define PMD_SETMEASURE_ERROR_OUTOFBOUNDS 1
#define PMD_SETMEASURE_ERROR_SONGSTOPPED 2
//INT 60 function 1D - Get Song Info Pointer
#define pmd_getsonginfoptr(type, seg, offset) asm inline ("movb $29, %%ah\n\tint $96\n\tmovw %%ds, %w1\n\tpush cs\n\tpop ds" : "=d" (offset), "=r" (seg) : "a" (type))
//Supporting defines
#define PMD_SONGINFO_PPZFILE  0xFE
#define PMD_SONGINFO_PPSFILE  0xFF
#define PMD_SONGINFO_PCMFILE  0x00
#define PMD_SONGINFO_TITLE    0x01
#define PMD_SONGINFO_COMPOSER 0x02
#define PMD_SONGINFO_ARRANGER 0x03
#define PMD_SONGINFO_MEMO(n) (0x04 + (n))
//INT 60 function 1E - Part Mask
#define pmd_partmask(num) asm inline ("movb $30, %%ah\n\tint $96" : : "a" (num))
//Supporting defines
#define PMD_PART_A    0
#define PMD_PART_B    1
#define PMD_PART_C    2
#define PMD_PART_D    3
#define PMD_PART_E    4
#define PMD_PART_F    5
#define PMD_PART_G    6
#define PMD_PART_H    7
#define PMD_PART_I    8
#define PMD_PART_J    9
#define PMD_PART_K    10
#define PMD_PART_C2   11
#define PMD_PART_C3   12
#define PMD_PART_C4   13
#define PMD_PART_RHY  14
#define PMD_PART_FMFX 15
#define PMD_PART_PPZ1 16
#define PMD_PART_PPZ2 17
#define PMD_PART_PPZ3 18
#define PMD_PART_PPZ4 19
#define PMD_PART_PPZ5 20
#define PMD_PART_PPZ6 21
#define PMD_PART_PPZ7 22
#define PMD_PART_PPZ8 23
//INT 60 function 1F - Get TimerB Interrupt Vector
#define pmd_getinttimerb(seg, offset) asm inline ("movb $31, %%ah\n\tint $96\n\tmovw %%ds, %w1\n\tpush cs\n\tpop ds" : "=d" (offset), "=r" (seg) : : "ah")
//INT 60 function 20 - Get TimerA Interrupt Vector
#define pmd_getinttimera(seg, offset) asm inline ("movb $32, %%ah\n\tint $96\n\tmovw %%ds, %w1\n\tpush cs\n\tpop ds" : "=d" (offset), "=r" (seg) : : "ah")
//INT 60 function 21 - Get Song File Name Pointer
#define pmd_getfilenameptr(seg, offset) asm inline ("movb $33, %%ah\n\tint $96\n\tmovw %%ds, %w1\n\tpush cs\n\tpop ds" : "=d" (offset), "=r" (seg) : : "ah")
//INT 60 function 22 - Get Buffer Sizes
#define pmd_getbufsize(size1, size2) asm inline ("movb $34, %%ah\n\tint $96" : "=a" (size1), "=d" (size2) : )

//Used when PMD.COM is loaded, like if there was only a 26K soundboard
typedef struct
{
    unsigned short otherWorkPtr;
    unsigned short FM1WorkPtr;
    unsigned short FM2WorkPtr;
    unsigned short FM3WorkPtr;
    unsigned short FM3ExtBWorkPtr;
    unsigned short FM3ExtCWorkPtr;
    unsigned short FM3ExtDWorkPtr;
    unsigned short SSG1WorkPtr;
    unsigned short SSG2WorkPtr;
    unsigned short SSG3WorkPtr;
    unsigned short PCMWorkPtr;
    unsigned short rhythmWorkPtr;
    unsigned short FMEffectWorkPtr;
} PMDWorkArea;

//Used when PMDB2.COM/PMDVA.COM/PMD86.COM is loaded, like if there was an 86 soundboard
typedef struct
{
    unsigned short otherWorkPtr;
    unsigned short FM1WorkPtr;
    unsigned short FM2WorkPtr;
    unsigned short FM3WorkPtr;
    unsigned short FM4WorkPtr;
    unsigned short FM5WorkPtr;
    unsigned short FM6WorkPtr;
    unsigned short SSG1WorkPtr;
    unsigned short SSG2WorkPtr;
    unsigned short SSG3WorkPtr;
    unsigned short PCMWorkPtr;
    unsigned short rhythmWorkPtr;
    unsigned short FM3ExtBWorkPtr;
    unsigned short FM3ExtCWorkPtr;
    unsigned short FM3ExtDWorkPtr;
    unsigned short FMEffectWorkPtr;
} PMD86WorkArea;

//Used when PMDPPZ.COM/PMDPPZE.COM is loaded, which uses 8 PCM channels apparently
typedef struct
{
    unsigned short otherWorkPtr;
    unsigned short FM1WorkPtr;
    unsigned short FM2WorkPtr;
    unsigned short FM3WorkPtr;
    unsigned short FM4WorkPtr;
    unsigned short FM5WorkPtr;
    unsigned short FM6WorkPtr;
    unsigned short SSG1WorkPtr;
    unsigned short SSG2WorkPtr;
    unsigned short SSG3WorkPtr;
    unsigned short PCMWorkPtr;
    unsigned short rhythmWorkPtr;
    unsigned short FM3ExtBWorkPtr;
    unsigned short FM3ExtCWorkPtr;
    unsigned short FM3ExtDWorkPtr;
    unsigned short PPZ1WorkPtr;
    unsigned short PPZ2WorkPtr;
    unsigned short PPZ3WorkPtr;
    unsigned short PPZ4WorkPtr;
    unsigned short PPZ5WorkPtr;
    unsigned short PPZ6WorkPtr;
    unsigned short PPZ7WorkPtr;
    unsigned short PPZ8WorkPtr;
    unsigned short FMEffectWorkPtr;
} PMDPPZWorkArea;

//Every part of the song uses this structure, even if there are some redundancies in the structure for a particular part (e.g. what are you gonna do with FM-related field in an SSG channel?)
typedef struct
{
    unsigned short curPlayAddress; //Decrement 1 to get the real address
    unsigned short returnAddress; //Decrement 1 to get the real address
    unsigned char noteRemainLen;
    unsigned char noteGateTime;
    unsigned short pitch;
    short detune;
    short LFOAmount;
    unsigned short globalPortamento;
    unsigned short localPortamento;
    unsigned short portamentoRemainder;
    unsigned char volume;
    signed char semitoneShift;
    unsigned char LFOcurDelay;
    unsigned char LFOcurSpeed;
    unsigned char LFOcurStep;
    unsigned char LFOcurTime;
    unsigned char LFOsetDelay;
    unsigned char LFOsetSpeed;
    unsigned char LFOsetStep;
    unsigned char LFOsetTime;
} PMDSongPartWorkArea;

//Miscellaneous other work data, common to all PMD types
typedef struct
{
    unsigned short songDataAddress; //Decrement 1 to get the real address
    unsigned short instDataAddress;
    unsigned short fmfxDataAddress;
    unsigned short fmIOPortDisp1;
    unsigned short fmIOPortDisp2;
    unsigned short fmIOPortInternal1;
    unsigned short fmIOPortInternal2;
} PMDOtherWorkArea;
