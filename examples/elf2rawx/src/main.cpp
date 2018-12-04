#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <cstring>
#include "stdtypes.h"
#include "elfreader.h"

void printHelp() {
    printf("This tool is a property of GNSS Sensor Limited.\n");
    printf("Any information maybe requested at sergeykhbr@gmail.com\n\n");
    printf("Use the following arguments:\n");
    printf("    -r    generate raw image file (default)\n");
    printf("    -h    generate ROM array file in HEX format\n");
    printf("    -f    define fixed image size in Bytes, otherwise "
           "the size will be computed\n");
    printf("    -l    bytes per line (with -h only). Default 16 bytes/128 "
           "bits.\n");
    printf("    -o    output file name1\n");
    printf("    -o    output file name0 <optional>\n");
    printf("Example\n");
    printf("    elf2rawx input_file_name -h -f 192168 -l 8 -o "
           "output_file_name\n");
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printHelp();
        return 0;
    }

    enum EOutputFormat {Format_RAW_IMAGE, Format_ROMHEX};
    EOutputFormat outfmt = Format_RAW_IMAGE;
    int iFixedSizeBytes = 0;
    int iBytesPerLine = 16;
    int infile_index = 1;
    AttributeType outFiles(Attr_List);
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i], "-r") == 0) {         // generate raw image file
            outfmt = Format_RAW_IMAGE;
        } else if (strcmp(argv[i], "-h") == 0) {  // generate rom hex file
            outfmt = Format_ROMHEX;
        } else if (strcmp(argv[i], "-f") == 0) {  // fixed size in bytes
            iFixedSizeBytes = strtol(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "-l") == 0) {  // bytes per hex-line
            iBytesPerLine = strtol(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "-o") == 0) {  // output file name
            AttributeType filename;
            filename.make_string(argv[++i]);
            outFiles.add_to_list(&filename);
        } else {
            infile_index = i;
        }
    };

    std::string arg1(argv[infile_index]);
    std::string in = std::string(arg1.begin(), arg1.end());

    ElfReader elf(in.c_str());
    if (elf.loadableSectionTotal() == 0) {
        printf("elf2rawx error: can't load file %s\n", in.c_str());
        return 0;
    }

    switch (outfmt) {
    case Format_RAW_IMAGE:
        elf.writeRawImage(&outFiles, iFixedSizeBytes);
        break;
    case Format_ROMHEX:
        elf.writeRomHexArray(&outFiles,
                             iBytesPerLine, iFixedSizeBytes);
        break;
    default:
        printHelp();
    }
    return 0;
}

