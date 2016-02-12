/*
 * Implementation of Batcher's sort using MPI.
 * Nikita Belov <zodiac.nv@gmail.com>
 * 2016
 */

#ifndef ARRAY_H
#define ARRAY_H

#include <stdint.h>
#include <stdlib.h>

/* Use: array(typename) array = array_new(ARRAY_VECTOR, typename). */
#define ARRAY_VECTOR -1

/* Use to indicate these arrays. */
#define array(type) type *

/* Internal use only. */
typedef struct {
    /* first sizeof(void *) bytes is pointer to this struct,
     * next bytes are user data
     */
    void *data;
    
    int type;
    int used;
    int allocated;
    int item_size;
} array_t;

/* For convenience. */
typedef unsigned char uchar;

/* Internal use only. */
void *array_get_data(array_t *array)
{
    return (uchar *)array->data + sizeof(void *);
}

/* Internal use only. */
array_t *array_get_ptr(void *array_data)
{
    return (array_t *)(*(uintptr_t *)((uchar *)array_data - sizeof(void *)));
}

/* Use: array(typename) array = array_new(count, typename). */
#define array_new(count, type) \
    array_new_func(count, sizeof(type))

/* Internal use only. */
void *array_new_func(int size, int item_size)
{
    array_t *array = calloc(sizeof(*array), 1);
    if (array == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(*array));
        exit(4);
    }
    
    if (size == ARRAY_VECTOR) {
        size = 0;
        array->type = ARRAY_VECTOR;
    }
    
    size_t allocate = (size_t)sizeof(void *) + size * item_size;
    array->data = calloc(allocate, 1);
    if (array->data == NULL) {
        printf("Failed to allocate %zu bytes.\n", allocate);
        exit(4);
    }
    
    array->used = 0;
    array->allocated = size;
    array->item_size = item_size;
    
    *(uintptr_t *)array->data = (uintptr_t)array;
    
    return array_get_data(array);
}

/* Use: array_concatenate(&result_array, array1, array2). */
void array_concatenate(void *result_data_ptr, void *array1_data, void *array2_data)
{
    void **result_data = (void **)result_data_ptr;
    
    array_t *result = array_get_ptr(*result_data);
    array_t *array1 = array_get_ptr(array1_data);
    array_t *array2 = array_get_ptr(array2_data);
    
    int item_size = result->item_size;
    size_t allocate = (size_t)sizeof(void *) + (array1->allocated + array2->allocated) * item_size;
    result->data = realloc(result->data, allocate);
    if (result->data == NULL) {
        printf("Failed to allocate %zu bytes.\n", allocate);
        exit(4);
    }
    
    memcpy((uchar *)result->data + sizeof(void *),
            array1_data, array1->allocated * item_size);
    memcpy((uchar *)result->data + sizeof(void *) + array1->allocated * item_size,
            array2_data, array2->allocated * item_size);
            
    result->allocated = array1->allocated + array2->allocated;
    *result_data = array_get_data(result);
}

/* Use: array_copy(destination_array, source_array, index of first item, count of items). */
void array_copy(void *destination_data, void *source_data, int first, int count)
{
    int item_size = array_get_ptr(destination_data)->item_size;
    memcpy(destination_data, (uchar *)source_data + first * item_size, count * item_size);
}

/* Use: array_delete(&array). */
void array_delete(void *array_data_ptr)
{
    void **array_data = (void **)array_data_ptr;
    array_t *array = array_get_ptr(*array_data);
    free(array->data);
    free(array);
    *array_data = NULL;
}

/* Internal use only. */
void array_expand(void *array_data_ptr)
{
    void **array_data = (void **)array_data_ptr;
    array_t *array = array_get_ptr(*array_data);
    if (array->used < array->allocated) {
        return;
    }
    if (array->allocated == 0) {
        array->allocated = 16;
    } else {
        array->allocated *= 2;
    }
    size_t allocate = (size_t)sizeof(void *) + array->allocated * array->item_size;
    array->data = realloc(array->data, allocate);
    if (array->data == NULL) {
        printf("Failed to allocate %zu bytes.\n", allocate);
        exit(4);
    }
    *array_data = array_get_data(array);
}

/* Use: int item_size = array_item_size(array). */
int array_item_size(void *array_data)
{
    return array_get_ptr(array_data)->item_size;
}

/* Use: array_push(&array, value). */
#define array_push(array_data, value)                                         \
    do {                                                                      \
        array_expand(array_data);                                             \
        (*array_data)[array_get_ptr(*array_data)->used++] = value;            \
    } while(0)

/* Use: int size = array_size(array). */
int array_size(void *array_data)
{
    array_t *array = array_get_ptr(array_data);
    if (array->type == ARRAY_VECTOR) {
        return array->used;
    } else {
        return array->allocated;
    }
}

#endif /* ARRAY_H */
