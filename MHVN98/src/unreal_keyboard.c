#include "unreal_keyboard.h"

unsigned char prevKeyStatus[16];
unsigned char keyChangeStatus[16];

void UpdatePrevKeyStatus()
{
    const unsigned long* kstat = (const unsigned long*)key_status_relptr;
    unsigned long* pkstat = (unsigned long*)prevKeyStatus;
    unsigned long* dkstat = (unsigned long*)keyChangeStatus;
    for(int i = 0; i < 4; i++)
    {
        const unsigned long kspart = kstat[i];
        dkstat[i] = kspart ^ pkstat[i];
        pkstat[i] = kspart;
    }
}