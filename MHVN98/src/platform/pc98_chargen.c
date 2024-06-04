//PC98 character generator interface
//Maxim Hoxha 2023-2024

static inline void SetJISCode(unsigned short code)
{
    __asm volatile (
        "outb %%al, $0xA1\n\t"
        "xchg %%al, %%ah\n\t"
        "outb %%al, $0xA3\n\t"
        "xchg %%al, %%ah\n\t"
    : : "a" (code));
}

static inline unsigned char GetCharacterRAMData(unsigned char addr)
{
    volatile register unsigned char data __asm("%al");
    data = addr;
    __asm volatile (
        "outb %%al, $0xA5\n\t"
        "inb $0xA9, %%al\n\t"
    : "+a" (data));
    return data;
}

static inline void SetCharacterRAMData(unsigned char addr, unsigned char data)
{
    __asm volatile (
        "movb %b0, %%al\n\t"
        "outb %%al, $0xA5\n\t"
        "movb %b1, %%al\n\t"
        "outb %%al, $0xA9\n\t"
    : : "rmi" (addr), "rmi" (data) : "%al");
}

void GetCharacterData(unsigned short code, unsigned long* buffer)
{
    SetJISCode(code); //Put JIS code in
    if (code & 0x00FF) //Fullwidth JIS code
    {
        for (unsigned char i = 0; i < 16; i++)
        {
            unsigned int rowhalf = GetCharacterRAMData(i);
            unsigned int row = (rowhalf << 8); //Read in right half
            rowhalf = GetCharacterRAMData(i | 0x20);
            row |= rowhalf; //Read in left half
            buffer[i] = row;
        }
    }
    else //Halfwidth JIS code
    {
        for (unsigned char i = 0; i < 16; i++)
        {
            unsigned long row = GetCharacterRAMData(i); //Read in left half only (both halves are the same, we're technically reading from the right side though)
            buffer[i] = row;
        }
    }
}

void GetCharacterDataEditFriendly(unsigned short code, unsigned long* buffer)
{
    SetJISCode(code); //Put JIS code in
    if (code & 0x00FF) //Fullwidth JIS code
    {
        for (unsigned char i = 0; i < 16; i++)
        {
            unsigned int row = GetCharacterRAMData(i); //Read in right half
            unsigned int rowhalf = GetCharacterRAMData(i | 0x20);
            row |= (rowhalf << 8); //Read in left half
            ((unsigned int*)(&buffer[i]))[0] = 0;
            ((unsigned int*)(&buffer[i]))[1] = row;
        }
    }
    else //Halfwidth JIS code
    {
        for (unsigned char i = 0; i < 16; i++)
        {
            unsigned long row = GetCharacterRAMData(i); //Read in left half only (both halves are the same, we're technically reading from the right side though)
            buffer[i] = (row << 24);
        }
    }
}

void SetCharacterData(unsigned short code, const unsigned long* buffer)
{
    SetJISCode(code); //Put JIS code in
    for (unsigned char i = 0; i < 16; i++)
    {
        unsigned long row = buffer[i];
        SetCharacterRAMData(i, (row >> 8) & 0x000000FF); //Write right half
        SetCharacterRAMData(i | 0x20, row & 0x000000FF); //Write left half
    }
}

/* Part of unicode.c now
void SwapCharDataFormats(unsigned long* buffer, int bits32)
{
    if (!bits32) //Also pack to 16 bits per character row, one way operation
    {
        for (unsigned char i = 0; i < 16; i++)
        {
            unsigned short row = ((unsigned short*)buffer)[2*i + 1];
            unsigned short temprow = (row & 0x00FF) << 8;
            ((unsigned short*)buffer)[i] = temprow | ((row & 0xFF00) >> 8);
        }
    }
    else
    {
        for (unsigned char i = 0; i < 16; i++)
        {
            unsigned long row = buffer[i];
            unsigned long temprow = (row & 0x000000FF) << 24;
            temprow |= (row & 0x0000FF00) << 8;
            temprow |= (row & 0x00FF0000) >> 8;
            buffer[i] = temprow | ((row & 0xFF000000) >> 24);
        }
    }
}
*/

unsigned short SjisToInternalCode(const unsigned short code)
{
    const unsigned char firstByte = code >> 8;
    const unsigned char secondByte = code & 0x00FF;
    const unsigned char isEvenUpperByte = secondByte >= 0x9F;
    const unsigned char outputLower = secondByte - 0x1F - (secondByte >= 0x7F) - 0x5E * isEvenUpperByte;
    const unsigned char outputUpper = (firstByte - 0x70 - 0x40 * (firstByte >= 0xA0)) * 2 - 0x21 + isEvenUpperByte; //Internal upper bytes are JIS-X-0208 upper bytes - 0x20
    return (outputUpper << 8) + outputLower;
}
