#include "pc98_keyboard.h"

unsigned char prevKeyStatus[16];
unsigned char keyChangeStatus[16];

void UpdatePrevKeyStatus()
{
    const unsigned __far int* kstat = (const unsigned __far int*)KEY_STATUS;
    unsigned int* pkstat = (unsigned int*)prevKeyStatus;
    unsigned int* dkstat = (unsigned int*)keyChangeStatus;
    for(int i = 0; i < 8; i++)
    {
        const unsigned int kspart = kstat[i];
        dkstat[i] = kspart ^ pkstat[i];
        pkstat[i] = kspart;
    }
}
