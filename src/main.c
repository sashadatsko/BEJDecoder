#include <stdio.h>
#include "../include/bej_decoder.h"

int main(int argc, char **argv)
{
    if (argc > 2)
        return bejDecode(argv[1], argv + 2, argc - 2);
    else if (argc == 2)
        printf("No bej files specified\n");
    else
        printf("USAGE: bejdecoder <dictionary> <input bej files>\n");

    return 1;
}