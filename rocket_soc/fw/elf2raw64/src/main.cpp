#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <cstring>
#include "stdtypes.h"
#include "elfreader.h"

void printHelp()
{
    printf("This tool is a property of GNSS Sensor Limited.\n");
    printf("Any information maybe requested at chief@gnss-sensor.com\n\n");
    printf("Use the following arguments:\n");
    printf("    -r    generate raw image file (default)\n");
    printf("    -h    generate ROM array file in HEX format\n");
    printf("    -f    define fixed image size in Bytes, otherwise the size will be computed\n");
    printf("    -l    bytes per line (with -h only). Default 16 bytes/128 bits.\n");
    printf("    -o    output file name\n");
    printf("Example\n");
    printf("    elf2raw input_file_name -h -f 192168 -l 8 -o output_file_name\n");
}

int main(int argc, char* argv[])
{
    if (argc < 4) {
        printHelp();
        return 0;
    }

    enum EOutputFormat {Format_RAW_IMAGE, Format_ROMHEX};
    EOutputFormat outfmt = Format_RAW_IMAGE;
    uint32 uiFixedSizeBytes = 0;
    uint32 uiBytesPerLine = 16;
    int infile_index = 1;
    int outfile_index = 3;
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i], "-r") == 0) {         // generate raw image file
            outfmt = Format_RAW_IMAGE;
        } else if (strcmp(argv[i], "-h") == 0) {  // generate rom hex file
            outfmt = Format_ROMHEX;
        } else if (strcmp(argv[i], "-f") == 0) {  // fixed size in bytes
            uiFixedSizeBytes = (uint32)strtol(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "-l") == 0) {  // bytes per hex-line
            uiBytesPerLine = (uint32)strtol(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "-o") == 0) {  // output file name
            outfile_index = ++i;
        } else {
            infile_index = i;
        }
    };

    std::string arg1(argv[infile_index]);
    std::string arg3(argv[outfile_index]);
    std::string in = std::string(arg1.begin(), arg1.end());
    std::string out = std::string(arg3.begin(), arg3.end());

    ElfReader elf(in.c_str());
    if (elf.isOpened() == 0) {
        printf("File %s not found\n", in.c_str());
        return 0;
    }

    switch (outfmt) {
    case Format_RAW_IMAGE:
        elf.writeRawImage(out.c_str(), uiFixedSizeBytes);
        break;
    case Format_ROMHEX:
        elf.writeRomHexArray(out.c_str(), uiBytesPerLine, uiFixedSizeBytes);
        break;
    default:
        printHelp();
    }
    return 0;
}

