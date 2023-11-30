#include "../include/bej_decoder.h"

#include <stdint.h>
#include <stdio.h>

#define NAME_MAX 255

enum BEJType
{
    BEJ_SET = 0,
    BEJ_ARRAY = 1,
    BEJ_INTEGER = 3,
    BEJ_STRING = 5
};

size_t fileSize(FILE *fp)
{
    long prevOfsset = ftell(fp);
    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, prevOfsset, SEEK_SET);
    return fsize;
}

uint64_t decodeNNInt(FILE *fp)
{
    uint64_t value = 0, bytes;
    bytes = fgetc(fp);
    fread(&value, bytes, 1, fp);
    return value;
}

void decodeEntry(FILE *fp, uint64_t seq, char *keyName)
{
    uint8_t dictEntry[10];
    fseek(fp, sizeof(dictEntry) * seq + 22, SEEK_SET);
    fread(dictEntry, sizeof(dictEntry), 1, fp);
    uint16_t childPointerOffset = *(uint16_t *)(dictEntry + 3);
    uint16_t childCount = *(uint16_t *)(dictEntry + 5);
    uint8_t nameLength = dictEntry[7];
    uint16_t nameOffset = *(uint16_t *)(dictEntry + 8);

    long prevOffset = ftell(fp);
    fseeko(fp, nameOffset, SEEK_SET);
    fread(keyName, nameLength, 1, fp);
    fseeko(fp, prevOffset, SEEK_SET);
}

void decodeTuple(FILE *dictStream, FILE *bejStream, FILE *jsonStream, int writeKeyName, size_t bejStreamSize, size_t objCount)
{
    size_t index = 0;
    while ((size_t)ftell(bejStream) < bejStreamSize && index < objCount)
    {
        uint64_t sequence = decodeNNInt(bejStream) >> 1;
        uint8_t format = fgetc(bejStream);
        uint64_t length = decodeNNInt(bejStream);

        char keyName[NAME_MAX];
        if (writeKeyName)
        {
            decodeEntry(dictStream, sequence, keyName);
            fprintf(jsonStream, "\"%s\":", keyName);
        }

        uint8_t formatType = format >> 4;
        if (formatType == BEJ_SET)
        {
            uint64_t count = decodeNNInt(bejStream);
            fputc('{', jsonStream);
            decodeTuple(dictStream, bejStream, jsonStream, 1, bejStreamSize, count);
            fputc('}', jsonStream);
        }
        if (formatType == BEJ_ARRAY)
        {
            uint64_t count = decodeNNInt(bejStream);
            fputc('[', jsonStream);
            for (size_t i = 0; i < count; i++)
            {
                decodeTuple(dictStream, bejStream, jsonStream, 0, bejStreamSize, 1);
                if (i < count - 1)
                    fputc(',', jsonStream);
            }

            fputc(']', jsonStream);
        }
        else if (formatType == BEJ_INTEGER)
        {
            int intValue = 0;
            fread(&intValue, length, 1, bejStream);
            fprintf(jsonStream, "%d", intValue);
        }
        else if (formatType == BEJ_STRING)
        {
            char stringValue[NAME_MAX];
            fread(stringValue, length, 1, bejStream);
            fprintf(jsonStream, "\"%s\"", stringValue);
        }

        if (index++ < objCount - 1)
            fputc(',', jsonStream);
    }
}

int bejDecode(char *dictionary, char **bejFiles, size_t fileCount)
{
    FILE *dictStream = fopen(dictionary, "rb");
    if (!dictStream)
    {
        printf("Failed to open dictionary\n");
        return 1;
    }

    uint8_t dictHeader[12];
    fread(dictHeader, sizeof(dictHeader), 1, dictStream);
    uint8_t versionTag = dictHeader[0];
    if (versionTag != 0x00)
    {
        printf("Wrong dictionary format\n");
        return 1;
    }

    for (size_t i = 0; i < fileCount; i++)
    {
        FILE *bejStream = fopen(bejFiles[i], "rb");
        if (!bejStream)
        {
            printf("Failed to open %s\n", bejFiles[i]);
            continue;
        }

        uint8_t bejHeader[7];
        fread(bejHeader, sizeof(bejHeader), 1, bejStream);
        uint32_t version = *(uint32_t *)bejHeader;
        uint8_t schemaClass = bejHeader[6];
        if ((version != 0xF1F0F000 && version != 0xF1F1F000) || schemaClass != 0x00)
        {
            printf("Wrong BEJ format in %s\n", bejFiles[i]);
            continue;
        }

        char fileName[NAME_MAX];
        snprintf(fileName, NAME_MAX, "%s.json", bejFiles[i]);
        FILE *jsonStream = fopen(fileName, "w");

        decodeTuple(dictStream, bejStream, jsonStream, 0, fileSize(bejStream), 1);

        fclose(jsonStream);
        fclose(bejStream);
    }

    fclose(dictStream);
    return 0;
}