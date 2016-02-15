/*
 * Tests that file is sorted.
 * Nikita Belov <zodiac.nv@gmail.com>
 * 2016
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
    if (argc != 2 && argc != 3) {
        printf("Usage: tester <input.dat> [source.dat].\n");
        return 1;
    }
    
    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        printf("File '%s' can't be opened.\n", argv[1]);
        return 2;
    }
    
    int ret = 0;
    
    int count;
    fread(&count, sizeof(count), 1, input);
    
    if (argc == 2) {
        uint32_t first, second;
        fread(&first, sizeof(first), 1, input);
        
        for (int i = 1; i < count; i++) {
            fread(&second, sizeof(second), 1, input);
            if (first > second) {
                printf("Not sorted: %d > %d.\n", first, second);
                ret = 3;
                goto end;
            }
            first = second;
        }
    } else {
        FILE *source = fopen(argv[2], "rb");
        if (source == NULL) {
            printf("File '%s' can't be opened.\n", argv[2]);
            ret = 2;
            goto end;
        }
        
        int count_source;
        fread(&count_source, sizeof(count_source), 1, source);
        
        if (count != count_source) {
            printf("Incorrect number of elements: [input: %d] vs [source: %d].\n", count, count_source);
            ret = 3;
            goto end_block;
        }
        
        uint32_t *array = malloc((size_t)count_source * sizeof(*array));
        if (array == NULL) {
            printf("Failed to allocate %zu bytes.\n", (size_t)count_source * sizeof(*array));
            ret = 3;
            goto end_block;
        }
        fread(array, sizeof(*array), count_source, source);
        qsort(array, count_source, sizeof(*array), compare_uint32);
        
        for (int i = 0; i < count_source; i++) {
            uint32_t value;
            fread(&value, sizeof(value), 1, input);
            if (array[i] != value) {
                printf("Unsorted: [input: %u] vs [source: %u] at %d.\n", array[i], value, i);
                ret = 3;
                break;
            }
        }
        
        free(array);
        
        int c;
        if (!ret && (c = fgetc(input)) != EOF) {
            printf("Unsorted: file end is not reached.\n");
            ret = 3;
        }

    end_block:
        fclose(source);
    }
    
end:
    fclose(input);
    
    if (ret == 0) {
        printf("Sorted.\n");
    }
    
    return ret;
}
