#include "../include/bej_decoder.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NAME_MAX 255 // Max JSON key and BEJ string length.

enum BEJType
{
    BEJ_SET = 0,
    BEJ_ARRAY = 1,
    BEJ_INTEGER = 3,
    BEJ_STRING = 5
};

typedef struct DictEntry
{
    uint8_t format;
    uint16_t seqNum;
    uint16_t childPointerOffset;
    uint16_t childCount;
    uint8_t nameLength;
    uint16_t nameOffset;
} DictEntry;

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

DictEntry *loadDictSubset(FILE *dictStream, size_t childOffset, int childCount)
{
    DictEntry *dictSubset = (DictEntry *)malloc(abs(childCount) * sizeof(DictEntry));

    if (childCount == -1)
        fseek(dictStream, 12, SEEK_SET);
    else
        fseek(dictStream, childOffset, SEEK_SET);

    size_t i = 0;
    do
    {
        uint8_t dictEntry[10];
        fread(dictEntry, sizeof(dictEntry), 1, dictStream);
        (dictSubset + i)->format = dictEntry[0];
        (dictSubset + i)->seqNum = *(uint16_t *)(dictEntry + 1);
        (dictSubset + i)->childPointerOffset = *(uint16_t *)(dictEntry + 3);
        (dictSubset + i)->childCount = *(uint16_t *)(dictEntry + 5);
        (dictSubset + i)->nameLength = dictEntry[7];
        (dictSubset + i)->nameOffset = *(uint16_t *)(dictEntry + 8);
        i++;
    } while (i < childCount && childCount != -1);

    return dictSubset;
}

DictEntry decodeDictEntry(FILE *dictStream, DictEntry *dictSubset, size_t subsetSize, uint64_t seq, char *keyName)
{
    for (size_t i = 0; i < subsetSize; i++)
    {
        if (dictSubset[i].seqNum == seq)
        {
            long prevOffset = ftell(dictStream);
            fseek(dictStream, dictSubset[i].nameOffset, SEEK_SET);
            fread(keyName, dictSubset[i].nameLength, 1, dictStream);
            fseek(dictStream, prevOffset, SEEK_SET);
            return dictSubset[i];
        }
    }

    return (DictEntry){};
}

void decodeTuple(FILE *dictStream, FILE *bejStream, FILE *jsonStream, int writeKeyName, size_t bejStreamSize,
                 size_t objCount, DictEntry *dictSubset, size_t subsetSize)
{
    size_t currObj = 0;
    while ((size_t)ftell(bejStream) < bejStreamSize && currObj < objCount)
    {
        uint64_t sequence = decodeNNInt(bejStream) >> 1; // Skip annotation dictionary
        uint8_t format = fgetc(bejStream);
        uint64_t length = decodeNNInt(bejStream);

        char keyName[NAME_MAX];
        DictEntry entry = decodeDictEntry(dictStream, dictSubset, subsetSize, sequence, keyName);
        if (writeKeyName)
        {
            fprintf(jsonStream, "\"%s\":", keyName);
        }

        uint8_t formatType = format >> 4;
        if (formatType == BEJ_SET)
        {
            uint64_t elemCount = decodeNNInt(bejStream);
            fputc('{', jsonStream);
            decodeTuple(dictStream, bejStream, jsonStream, 1, bejStreamSize, elemCount, loadDictSubset(dictStream, entry.childPointerOffset, entry.childCount), entry.childCount);
            fputc('}', jsonStream);
        }
        if (formatType == BEJ_ARRAY)
        {
            uint64_t elemCount = decodeNNInt(bejStream);
            fputc('[', jsonStream);
            for (size_t i = 0; i < elemCount; i++)
            {
                decodeTuple(dictStream, bejStream, jsonStream, 0, bejStreamSize, 1, dictSubset, subsetSize);
                if (i < elemCount - 1)
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

        if (currObj++ < objCount - 1)
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
        if ((version != 0xF1F0F000 && version != 0xF1F1F000) || schemaClass != 0x00) // Decode only MAJOR schema
        {
            printf("Wrong BEJ format in %s\n", bejFiles[i]);
            fclose(bejStream);
            continue;
        }

        char fileName[NAME_MAX];
        snprintf(fileName, NAME_MAX, "%s.json", bejFiles[i]);
        FILE *jsonStream = fopen(fileName, "w");

        size_t bejSize = fileSize(bejStream);
        decodeTuple(dictStream, bejStream, jsonStream, 0, bejSize, 1, loadDictSubset(dictStream, 0, -1), 1);

        fclose(jsonStream);
        fclose(bejStream);
    }

    fclose(dictStream);
    return 0;
}