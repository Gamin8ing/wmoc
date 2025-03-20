# WMOC - Writing My Own Compiler
**WMOC** is a learning project about compiler design and construction. It’s based on **PL/0**, formulated by [Niklaus Wirth](https://en.wikipedia.org/wiki/Niklaus_Wirth) as a minimal educational language.

### PL/0
PL/0 (or pl0) is an "educational programming language" as stated by [Rosetta Code](https://rosettacode.org/wiki/Category:PL/0). The syntax rule of the language can be represented in the EBNF form as:

```
program		= block "." .
block		= [ "const" ident "=" number { "," ident "=" number } ";" ]
                  [ "var" ident { "," ident } ";" ]
                  { "procedure" ident ";" block ";" } statement .
statement	= [ ident ":=" expression
                  | "call" ident
                  | "begin" statement { ";" statement } "end"
                  | "if" condition "then" statement
                  | "while" condition "do" statement ] .
                  | "printInt" ident
                  | "printChar" into ident
                  | "readInt" into ident
condition	= "odd" expression
                | expression ( "=" | "#" | "<" | ">" ) expression .
expression	= [ "+" | "-" ] term { ( "+" | "-" ) term } .
term		= factor { ( "*" | "/" ) factor } .
factor		= ident
                | number
                | "(" expression ")" .
ident		= "A-Za-z_" { "A-Za-z0-9_" } .
number		= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" .
```


## Main Flow
In this first version, **WMOC** works as a front end. It generates **intermediate C code** from `.pl0` source files. You can then compile the generated C code with any C compiler (like `gcc`).

```
Source Code -> Lexer -> Parser -> Semantic Analyser -> C Code Generation
```

## Installation

1. Download the executable `wmoc` from [releases.](https://github.com/Gamin8ing/wmoc/releases/tag/v1.0.0)
2. Run:
```bash
./wmoc <your-.pl0-file> <output.c-filename
```
3. This will generate a `.c` file. Then compile and run it with:
```bash
gcc <output.c> && ./a.out
```

## Build from source

1. Clone the repository:
```bash
git clone https://github.com/Gamin8ing/wmoc.git
cd wmoc
```
2. Build using:
```bash
make
```
3. An executable named `wmoc` will be created.
4. To use it:
```bash
./wmoc <your-file.pl0> <output.c>
```
5. Then compile and run it with:
```bash
gcc <output.c> && ./a.out
```

## Example Usage
An example `.pl0` file is included in the source code (example.pl0).
### Command
```bash
./wmoc example.pl0 example.c
gcc example.c -o example && ./example

```

## Issues
To report bugs or request features, please open an issue using our issue template mentioned in [CONTRIBUTION.md](https://github.com/Gamin8ing/wmoc/blob/v1.0.0/CONTRIBUTION.md). We appreciate feedback and contributions! 


