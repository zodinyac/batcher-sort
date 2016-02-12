/*
 * Tests that file is sorted.
 * Nikita Belov <zodiac.nv@gmail.com>
 * 2016
 */

#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: tester <input.dat>.\n");
        return 1;
    }
      
    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        printf("File '%s' can't be opened.\n", argv[1]);
        return 2;
    }
    
    int count;
    fread(&count, sizeof(count), 1, input);
    
    uint32_t first, second;
    fread(&first, sizeof(first), 1, input);
    
    for (int i = 1; i < count; i++) {
        fread(&second, sizeof(second), 1, input);
        if (first > second) {
            printf("Not sorted: %d > %d.\n", first, second);
            return 3;
        }
        first = second;
    }
    
    fclose(input);
    
    printf("Sorted.\n");
    
    return 0;
}
