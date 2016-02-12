/*
 * Binary file reader.
 * Nikita Belov <zodiac.nv@gmail.com>
 * 2016
 */

#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: reader <input.dat>.\n");
        return 1;
    }
      
    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        printf("File '%s' can't be opened.\n", argv[1]);
        return 2;
    }
    
    int count;
    fread(&count, sizeof(count), 1, input);
    
    printf("Count: %d.\n\n", count);
    
    uint32_t value;
    for (int i = 0; i < count; i++) {
        fread(&value, sizeof(value), 1, input);
        printf("%u ", value);
    }
    printf("\n");
    
    fclose(input);
    
    return 0;
}
