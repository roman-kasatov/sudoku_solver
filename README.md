# sudoku_solver
Solve almost any-size sudoku fast with Donald Knuth's Algorithm X

### How to use
Compile `main.cpp` and run with command line

Standard input file with sudoku is "input.txt"\
File must be in the same folder with executable

### Input file format
First line contains one number - the **size** of field\
Next goes the field where gaps are 0 and normal values are numbers from 1 to **size**

#### Examples for input file are
- `input.txt` 9\*9 sudoku with many solutions
- `in16b16.txt` 16\*16 sudoku with only one solution (classic sudoku)

### Find all solutions
Sudoku can have many different solutions and therefore it's inconvenient when they are all printed.\
That's why the program finds first solution and stops as default.\
But if you want to find them all (or check if there is truly only one) use\
**`algorithmX(matrix, table);`**\
instead of\
**`algorithmX(matrix, table, true);`**\
in method **main**
