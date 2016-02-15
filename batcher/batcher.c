/*
 * Implementation of Batcher's sort using MPI.
 * Nikita Belov <zodiac.nv@gmail.com>
 * 2016
 */

#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <string.h>

#include "array.h"

typedef struct {
    int a;
    int b;
} pair_t;
array(pair_t) comparators;

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

int max_value(array(int) array)
{
    int max = 0, max_first = 1;
    for (int i = 0; i < array_size(array); i++) {
        if (max_first || array[i] > max) {
            max = array[i];
            max_first = 0;
        }
    }
    return max;
}

void swap_ptr(void *ptr1_ptr, void *ptr2_ptr)
{
    void **ptr1 = (void **)ptr1_ptr;
    void **ptr2 = (void **)ptr2_ptr;
    
    void *tmp = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = tmp;
}

void S(array(int) procs_up, array(int) procs_down)
{
    int proc_count = array_size(procs_up) + array_size(procs_down);
    if (proc_count == 1) {
        return;
    } else if (proc_count == 2) {
        array_push(&comparators, ((pair_t){ procs_up[0], procs_down[0] }));
        return;
    }
    
    array(int) procs_up_odd = array_new(array_size(procs_up) / 2 + array_size(procs_up) % 2, int);
    array(int) procs_down_odd = array_new(array_size(procs_down) / 2 + array_size(procs_down) % 2, int);
    array(int) procs_up_even = array_new(array_size(procs_up) / 2, int);
    array(int) procs_down_even = array_new(array_size(procs_down) / 2, int);
    array(int) procs_result = array_new(array_size(procs_up) + array_size(procs_down), int);
    
    for (int i = 0; i < array_size(procs_up); i++) {
        if (i % 2) {
            array_push(&procs_up_even, procs_up[i]);
        } else {
            array_push(&procs_up_odd, procs_up[i]);
        }
    }
    for (int i = 0; i < array_size(procs_down); i++) {
        if (i % 2) {
            array_push(&procs_down_even, procs_down[i]);
        } else {
            array_push(&procs_down_odd, procs_down[i]);
        }
    }
    
    S(procs_up_odd, procs_down_odd);
    S(procs_up_even, procs_down_even);
    
    array_concatenate(&procs_result, procs_up, procs_down);

    for (int i = 1; i + 1 < array_size(procs_result); i += 2) {
        array_push(&comparators, ((pair_t){ procs_result[i], procs_result[i + 1] }));
    }
    
    array_delete(&procs_up_odd);
    array_delete(&procs_down_odd);
    array_delete(&procs_up_even);
    array_delete(&procs_down_even);
    array_delete(&procs_result);
}

void B(array(int) procs)
{
    if (array_size(procs) == 1) {
        return;
    }
    
    array(int) procs_up = array_new(array_size(procs) / 2, int);
    array(int) procs_down = array_new(array_size(procs) / 2 + array_size(procs) % 2, int);
    
    array_copy(procs_up, procs, 0, array_size(procs_up));
    array_copy(procs_down, procs, array_size(procs_up), array_size(procs_down));
    
    B(procs_up);
    B(procs_down);
    S(procs_up, procs_down);
    
    array_delete(&procs_up);
    array_delete(&procs_down);
}

void batcher(int proc_count)
{
    array(int) procs = array_new(proc_count, int);
    for (int i = 0; i < array_size(procs); i++) {
        procs[i] = i;
    }
    B(procs);
    array_delete(&procs);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: batcher <input.dat> <output.dat>.\n");
        return 1;
    }
    
    MPI_Status status;
    int ret;
    double read_time, sort_time, write_time;
    
    MPI_Init(&argc, &argv);
    
    int rank, proc_count;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_count);
    
    if (proc_count <= 1) {
        printf("Processors count must be > 1.\n");
        MPI_Finalize();
        return 1;
    }

    comparators = array_new(ARRAY_VECTOR, pair_t);
    batcher(proc_count);
    
    MPI_File input;
    ret = MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &input);
    if (ret != MPI_SUCCESS) {
        if (rank == 0) {
            printf("File '%s' can't be opened.\n", argv[1]);
        }
        MPI_Finalize();
        return 2;
    }
    
    int elems_count;
    MPI_File_read(input, &elems_count, 1, MPI_UNSIGNED, &status);
    
    int elems_count_new = elems_count + (elems_count % proc_count ? proc_count - elems_count % proc_count : 0);
    int elems_per_proc_count = elems_count_new / proc_count;

    array(uint32_t) elems_result = array_new(elems_per_proc_count, uint32_t);
    array(uint32_t) elems_current = array_new(elems_per_proc_count, uint32_t);
    array(uint32_t) elems_temp = array_new(elems_per_proc_count, uint32_t);
    
    read_time = MPI_Wtime();
    
    MPI_File_seek_shared(input, sizeof(elems_count), MPI_SEEK_SET);
    MPI_File_read_ordered(input, elems_result, elems_per_proc_count, MPI_UNSIGNED, &status);
   
    read_time = MPI_Wtime() - read_time;
    
    MPI_File_close(&input);

    sort_time = MPI_Wtime();
    qsort(elems_result, elems_per_proc_count, array_item_size(elems_result), compare_uint32);
    for (int i = 0; i < array_size(comparators); i++) {
        pair_t comparator = comparators[i];
        if (rank == comparator.a) {
            MPI_Send(elems_result, elems_per_proc_count, MPI_UNSIGNED, comparator.b, 0, MPI_COMM_WORLD);
            MPI_Recv(elems_current, elems_per_proc_count, MPI_UNSIGNED, comparator.b, 0, MPI_COMM_WORLD, &status);
            
            for (int res_index = 0, cur_index = 0, tmp_index = 0; tmp_index < elems_per_proc_count; tmp_index++) {
                uint32_t result = elems_result[res_index];
                uint32_t current = elems_current[cur_index];
                if (result < current) {
                    elems_temp[tmp_index] = result;
                    res_index++;
                } else {
                    elems_temp[tmp_index] = current;
                    cur_index++;
                }
            }
            
            swap_ptr(&elems_result, &elems_temp);
        } else if (rank == comparator.b) {
            MPI_Recv(elems_current, elems_per_proc_count, MPI_UNSIGNED, comparator.a, 0, MPI_COMM_WORLD, &status);
            MPI_Send(elems_result, elems_per_proc_count, MPI_UNSIGNED, comparator.a, 0, MPI_COMM_WORLD);
            
            int start = elems_per_proc_count - 1;
            for (int res_index = start, cur_index = start, tmp_index = start; tmp_index >= 0; tmp_index--) {
                uint32_t result = elems_result[res_index];
                uint32_t current = elems_current[cur_index];
                if (result > current) {
                    elems_temp[tmp_index] = result;
                    res_index--;
                } else {
                    elems_temp[tmp_index] = current;
                    cur_index--;
                }
            }
            
            swap_ptr(&elems_result, &elems_temp);
        }
    }
    sort_time = MPI_Wtime() - sort_time;

    MPI_File output;
    ret = MPI_File_open(MPI_COMM_WORLD, argv[2], MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &output);
    if (ret != MPI_SUCCESS) {
        if (rank == 0) {
            printf("File '%s' can't be opened.\n", argv[2]);
        }
        MPI_Finalize();
        return 3;
    }
    MPI_File_set_size(output, 0);
    
    int skip = elems_count_new - elems_count;
    int print_offset = (skip / elems_per_proc_count == rank) * (skip % elems_per_proc_count);
    int print_count = (skip / elems_per_proc_count <= rank) * elems_per_proc_count - print_offset;
    
    write_time = MPI_Wtime();

    MPI_File_write_ordered(output, &elems_count, rank == 0, MPI_UNSIGNED, &status);
    MPI_File_write_ordered(output, (unsigned char *)elems_result + print_offset * array_item_size(elems_result), print_count, MPI_UNSIGNED, &status);
    
    write_time = MPI_Wtime() - write_time;
    
    MPI_File_close(&output);
    
    if (rank == 0) {
        printf("Read time: %lf\n", read_time);
        printf("Sort time: %lf\n", sort_time);
        printf("Write time: %lf\n", write_time);
    }

    array_delete(&comparators);
    array_delete(&elems_result);
    array_delete(&elems_current);
    array_delete(&elems_temp);
    
    MPI_Finalize();
    
    return 0;
}
