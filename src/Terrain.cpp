#include "Terrain.h"

#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <stdlib.h>
#include <string.h>


/* code modified from OGLDEV */
char* ReadBinaryFile(const char* pFilename, int& size)
{
    FILE* f = NULL;

    errno_t err = fopen_s(&f, pFilename, "rb");

    if (!f) {
        char buf[256] = { 0 };
        strerror_s(buf, sizeof(buf), err);
        exit(0);
    }

    struct stat stat_buf;
    int error = stat(pFilename, &stat_buf);

    if (error) {
        char buf[256] = { 0 };
        strerror_s(buf, sizeof(buf), err);
        return NULL;
    }

    size = stat_buf.st_size;

    char* p = (char*)malloc(size);
    assert(p);

    size_t bytes_read = fread(p, 1, size, f);

    if (bytes_read != size) {
        char buf[256] = { 0 };
        strerror_s(buf, sizeof(buf), err);
        exit(0);
    }

    fclose(f);
    return p;
}


void Terrain::PrintFloat() {
    for (int y = 0 ; y < terrainSize ; y++) {
        printf("%d: ", y);
        for (int x = 0 ; x < terrainSize ; x++) {
            float f = (float)heightMap[y * terrainSize + x];
            printf("%.6f ", f);
        }
        printf("\n");
    }
}



void Terrain::LoadFromFile (const char *f) {
    int size = 0;
    unsigned char *p = (unsigned char *)ReadBinaryFile(f, size);

    //assert(size % sizeof(float) == 0);
    terrainSize = sqrtf(size / sizeof(float));

    heightMap = (float *) p;

    PrintFloat();
    
}
