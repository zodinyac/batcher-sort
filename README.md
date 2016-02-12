# batcher-sort
Implementation of Batcher's sort using MPI.

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
Tests that file is sorted.
Usage:
```sh
./tester <input.dat>
```

### QSort
Classic C qsort to compare with Batcher's sort.
Usage:
```sh
./qsort <input.dat> <output.dat>
```
