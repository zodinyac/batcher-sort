/*
 * Generates a file with random sequence.
 * Nikita Belov <zodiac.nv@gmail.com>
 * 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: generator <output.dat> <count>.\n");
        return 1;
    }
    
    int count = strtol(argv[2], NULL, 10);
    
    FILE *output = fopen(argv[1], "wb");
    if (output == NULL) {
        printf("File '%s' can't be opened.\n", argv[1]);
        return 2;
    }
    
    srand(time(NULL));
    
    fwrite(&count, sizeof(count), 1, output);
    for (int i = 0; i < count; i++) {
        int value = rand();
        fwrite(&value, sizeof(value), 1, output);
    }
    
    fclose(output);
    
    return 0;
}
