#pragma once

#include "platform/x86interrupt.h"

INTFUNC void VsyncInterrupt();

extern volatile unsigned char vsynced;
