# batcher-sort
Implementation of Batcher's sort using MPI.

### Batcher
Usage:
```sh
./batcher <input.dat> <output.dat>
```

### Generator
Generates a file with random sequence.
Usage:
```sh
./generator <output.dat> <count of elements>
```

### Reader
Binary file reader.
Usage:
```sh
./reader <input.dat>
```

### Tester
Tests that file is sorted. You can use that program with source file which contains unsorted array. The tester will check that input file contains a full copy of sorted source array and nothing else.
Usage:
```sh
./tester <input.dat> [source.dat]
```

### QSort
Classic C qsort to compare with Batcher's sort.
Usage:
```sh
./qsort <input.dat> <output.dat>
```
