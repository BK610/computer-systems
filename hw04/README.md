# Assignment
Write a C program that sorts a sequence of strings, in ASCII order (as you'd get using the "strcmp" library function).

## Details

* The number of strings to sort will be provided as the first argument to your program.
* The strings will be provided one per line on standard input. (i.e. read with fgets(..., ..., stdin))
* The sorted strings should be output one per line on standard output.
* You are free to use any standard C library functions to write your program - including qsort, which will sort for you.
* Do not use insertion sort.
* See hints.txt for some extra ideas.
* The clang-check tool should show no warnings / errors on your source.
* Running your program under valgrind should show no memory leaks.

## Sample Session
```
bash$ cat sample.txt 
underplays
iguana
pontiff
flown
ambassador
revolutionized
remorselessly
reactivates
plenty
introduces
bash$ wc -l sample.txt 
10 sample.txt
bash$ ./sort-strings 10 < sample.txt 
ambassador
flown
iguana
introduces
plenty
pontiff
reactivates
remorselessly
revolutionized
underplays
bash$ 
```
