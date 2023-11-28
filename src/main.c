#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define NAME_MAX 255

#define TYPE_SET 0
#define TYPE_ARRAY 1
#define TYPE_INTERGER 3
#define TYPE_ENUM 4
#define TYPE_STRING 5
#define TYPE_BOOLEAN 7

typedef uint16_t bejTupleS;

typedef struct bejTupleF
{
    uint8_t dbFlag : 1;
    uint8_t roFlag : 1;
    uint8_t nullableFlag : 1;
    uint8_t reservedFlag : 1;
    uint8_t type : 4;
} bejTupleF;

typedef uint16_t bejTupleL;

typedef struct DHeader
{
    uint8_t version;
    uint8_t flags;
    uint16_t entryCount;
    uint32_t schemaVersion;
    uint32_t dictionarySize;
} DHeader;

typedef struct DEntry
{
    bejTupleF format;
    uint16_t seqNumber;
    uint16_t childPointerOffset;
    uint16_t childCount;
    uint8_t nameLength;
    uint16_t nameOffset;
} Dentry;

void jsonBegin(FILE *fp)
{
    fputs("{\n", fp);
}

void jsonEnd(FILE *fp)
{
    fseeko(fp, -1, SEEK_CUR);
    fputs("\n}", fp);
}

void jsonBoolean(FILE *fp, const char *key, const char *value)
{
    fprintf(fp, "\"%s\": %s,", key, value);
}

void jsonInteger(FILE *fp, const char *key, int value)
{
    fprintf(fp, "\"%s\": %d,", key, value);
}

void jsonString(FILE *fp, const char *key, const char *value)
{
    fprintf(fp, "\"%s\": \"%s\",", key, value);
}

void jsonSetBegin(FILE *fp, const char *key)
{
    fprintf(fp, "\"%s\": {", key);
}

void jsonSetEnd(FILE *fp)
{
    fseeko(fp, -1, SEEK_CUR);
    fputs("},", fp);
}

int processTuple(FILE *fdct, FILE *fbej, FILE *fjson, size_t childOffset)
{
    uint8_t sflTuple[5];
    if (!fread(sflTuple, sizeof(sflTuple), 1, fbej))
        return 0;

    bejTupleS seqNum = *(bejTupleS *)sflTuple;
    bejTupleF format = *(bejTupleF *)(sflTuple + 2);
    bejTupleL length = *(bejTupleL *)(sflTuple + 3) >> 8;

    uint8_t entry[10];
    fseeko(fdct, childOffset, SEEK_SET);
    fread(&entry, sizeof(entry), 1, fdct);

    Dentry dEntry;
    dEntry.format = *(bejTupleF *)entry;
    dEntry.seqNumber = *(bejTupleS *)(entry + 1);
    dEntry.childPointerOffset = *(uint16_t *)(entry + 3);
    dEntry.childCount = *(uint16_t *)(entry + 5);
    dEntry.nameLength = entry[7];
    dEntry.nameOffset = *(uint16_t *)(entry + 8);

    char keyName[NAME_MAX];
    long prevOffset = ftell(fdct);
    fseeko(fdct, dEntry.nameOffset, SEEK_SET);
    fread(keyName, dEntry.nameLength, 1, fdct);
    fseeko(fdct, prevOffset, SEEK_SET);

    // printf("name offsset: %d\n", dEntry.nameOffset);
    // printf("len: %d\n", length);

    int vInt;
    char vString[NAME_MAX];
    switch (format.type)
    {
    case TYPE_BOOLEAN:
        fread(&vInt, length, 1, fbej);
        jsonBoolean(fjson, keyName, vInt ? "true" : "false");
        break;
    case TYPE_INTERGER:
        fread(&vInt, length, 1, fbej);
        jsonInteger(fjson, keyName, vInt);
        break;
    case TYPE_ENUM:
    case TYPE_STRING:
        fread(&vString, length, 1, fbej);
        jsonString(fjson, keyName, vString);
        break;
    case TYPE_ARRAY:
    case TYPE_SET:
        jsonSetBegin(fjson, keyName);
        printf("co %d\n", dEntry.childPointerOffset);
        processTuple(fdct, fbej, fjson, dEntry.childPointerOffset);
        jsonSetEnd(fjson);
        break;
    }

    return 1;
}

void bejDecode(char *dictionary, char **bejFiles, size_t count)
{
    FILE *fdct = fopen(dictionary, "rb");
    if (!fdct)
    {
        printf("Failed to open dictionary\n");
        return;
    }

    DHeader dHeader;
    fread(&dHeader, sizeof(DHeader), 1, fdct);

    for (size_t i = 0; i < count; i++)
    {
        FILE *fbej = fopen(bejFiles[i], "rb");
        if (!fbej)
        {
            printf("Failed to open %s\n", bejFiles[i]);
            continue;
        }

        char fileName[NAME_MAX];
        snprintf(fileName, NAME_MAX, "%s.json", bejFiles[i]);
        FILE *fjson = fopen(fileName, "w");

        jsonBegin(fjson);

        uint16_t childPointerOffset = sizeof(DHeader);
        for (;;)
        {
            if(!processTuple(fdct, fbej, fjson, childPointerOffset))
                break;

            uint8_t entry[10];
            fread(&entry, sizeof(entry), 1, fdct);
            childPointerOffset = *(uint16_t *)(entry + 3);
        }

        jsonEnd(fjson);

        fclose(fjson);
        fclose(fbej);
    }

    fclose(fdct);
}

int main(int argc, char **argv)
{
    if (argc > 2)
        bejDecode(argv[1], argv + 2, argc - 2);
    else if (argc == 2)
        printf("No bej files specified\n");
    else
        printf("USAGE: bejdecoder <dictionary> <input bej files>\n");

    return 0;
}