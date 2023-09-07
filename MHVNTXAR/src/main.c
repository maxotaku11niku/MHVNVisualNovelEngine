#include <stdio.h>

int main(int argc, char** argv)
{
    printf("MHVNTXAR - MHVN text archiver tool - Version 0.0.1 - Maxim Hoxha 2023");

    if (argc < 2)
    {
        printf("You haven't specified any files or options!");
    }
    for (int i = 0; i < argc; i++)
    {
        printf("%s\n", argv[i]);
    }
    return 0;
}