#include <stdbool.h>

//Does the actual archiving of the given text files into an archive object with the given filename.
int ArchiveText(const char* outputFilename, const char** inputFilenames, const int numInputFiles, const bool sjis);