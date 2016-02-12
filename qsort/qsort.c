/*
 * Classic C qsort.
 * Nikita Belov <zodiac.nv@gmail.com>
 * 2016
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int compare_uint32(const void *a, const void *b)
{
    if (*(uint32_t *)a < *(uint32_t *)b) {
        return -1;
    } else if (*(uint32_t *)a == *(uint32_t *)b) {
        return 0;
    } else {
        return 1;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: qsort <input.dat> <output.dat>.\n");
        return 1;
    }
    
    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        printf("File '%s' can't be opened.\n", argv[1]);
        return 2;
    }

    clock_t clocks = clock();
    
    int count;
    fread(&count, sizeof(count), 1, input);

    uint32_t *array = malloc((size_t)count * sizeof(*array));
    if (array == NULL) {
        printf("Failed to allocate %zu bytes.\n", (size_t)count * sizeof(*array));
        return 3;
    }
    fread(array, sizeof(*array), count, input);
    
    clocks = clock() - clocks;
    printf("Read time: %lf\n", (double)clocks / CLOCKS_PER_SEC);
    
    fclose(input);
    
    clocks = clock();
    qsort(array, count, sizeof(*array), compare_uint32);
    clocks = clock() - clocks;
    printf("Sort time: %lf\n", (double)clocks / CLOCKS_PER_SEC);
    
    FILE *output = fopen(argv[2], "wb");
    if (output == NULL) {
        printf("File '%s' can't be opened.\n", argv[2]);
        return 4;
    }
    
    clocks = clock();
    fwrite(&count, sizeof(count), 1, output);
    fwrite(array, sizeof(*array), count, input);
    clocks = clock() - clocks;
    printf("Write time: %lf\n", (double)clocks / CLOCKS_PER_SEC);
    
    fclose(output);
    
    free(array);
    
    return 0;
}
