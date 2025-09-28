# Fourier-Motzkin Elimination

A toy theorem prover for the field of rational numbers based on Fourier-Motzkin elimination. Made as part of the coursework for the Automated Reasoning course at the Faculty of Mathematics, University of Belgrade.

## Building

To build this project, you will need:
- CMake (min. version 3.23)
- A C++ compiler with C++20 support
- GNU Bison (min. version 3.2) installed on your system
- GNU Flex installed on your system

To build the project, position yourself to the `source/` directory and run `mkdir build && cd build && cmake .. && make`.

## Usage example

The `fourier-motzkin` executable reads a first-order formula from the standard input and then outputs the result (if the formula is a theorem or not in the field of rational numbers). The `examples/` directory contains a couple of examples of valid first-order formulas.

### Usage:

#### Example 1
```
$ cat ../../examples/transitivity.fmfol | ./fourier_motzkin 
========== [PROOF START] ==========
[FORMULA] !x.!y.!z.x<y & y<z => x<z
[CLOSED PRENEX] !x.!y.!z.~x<y | ~y<z | x<z
[VARIABLE ELIMINATION] Eliminating universally bound variable "z"
        Base formula (negated due to universal quantification): ~(~x<y | ~y<z | x<z)
        Base formula DNF: x<y & y<z & x>z | x<y & y<z & x=z
        New base formula (negated due to universal quantification): ~(0+x-y<0 & 0-x+y<0 | 0+x-y<0 & 0-x+y<0)
[VARIABLE ELIMINATION] Eliminating universally bound variable "y"
        Base formula (negated due to universal quantification): ~~(0+x-y<0 & 0-x+y<0 | 0+x-y<0 & 0-x+y<0)
        Base formula DNF: 0+x-y<0 & 0-x+y<0 | 0+x-y<0 & 0-x+y<0
        New base formula (negated due to universal quantification): ~(0<0 | 0<0)
[VARIABLE ELIMINATION] Eliminating universally bound variable "x"
        Base formula (negated due to universal quantification): ~~(0<0 | 0<0)
        Base formula DNF: 0<0 | 0<0
        New base formula (negated due to universal quantification): ~(0<0 | 0<0)
[QUANTIFIER FREE FORM] ~(0<0 | 0<0)
[RESULT] Formula is a theorem
=========== [PROOF END] ===========
```

#### Example 2
```
$ ./fourier_motzkin 
x > 0 & x < 0
========== [PROOF START] ==========
[FORMULA] x>0 & x<0
[CLOSED PRENEX] ?x.x>0 & x<0
[VARIABLE ELIMINATION] Eliminating existentially bound variable "x"
        Base formula: x>0 & x<0
        Base formula DNF: x>0 & x<0
        New base formula: 0<0
[QUANTIFIER FREE FORM] 0<0
[RESULT] Formula is not a theorem
=========== [PROOF END] ===========
```

## Paper

The `paper/` directory contains the `.tex` source and the produced `.pdf` for the accompanying seminary paper (written in Serbian).
