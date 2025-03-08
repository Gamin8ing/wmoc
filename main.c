#include <stdio.h>
#include <stdlib.h>
#include "utils.c" // my own utils functions libraary?
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// WMOC-PL0: Writing My Own Compiler for PL/0 language

// this is me writing my own compiler, following the blog at
// https://briancallahan.net/blog/20210814.html

/* PL/0 compiler
program		= block "." .
block		= [ "const" ident "=" number { "," ident "=" number } ";" ]
                  [ "var" ident { "," ident } ";" ]
                  { "procedure" ident ";" block ";" } statement .
statement	= [ ident ":=" expression
                  | "call" ident
                  | "begin" statement { ";" statement } "end"
                  | "if" condition "then" statement
                  | "while" condition "do" statement ] .
condition	= "odd" expression
                | expression ( "=" | "#" | "<" | ">" ) expression .
expression	= [ "+" | "-" ] term { ( "+" | "-" ) term } .
term		= factor { ( "*" | "/" ) factor } .
factor		= ident
                | number
                | "(" expression ")" .
ident		= "A-Za-z_" { "A-Za-z0-9_" } .
number		= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" .
*/

// -------------- DEFINES (TOKENS) -------------------
// TK_ -> means a token
#define TK_DOT "."
#define TK_IDENTIFIER "I"
#define TK_NUMBER "N"
#define TK_CONST "C"
#define TK_VAR "V"
#define TK_PROCEDURE "P"
#define TK_CALL "C"
#define TK_BEGIN "B"
#define TK_END "E"
#define TK_IF "i"
#define TK_THEN "T"
#define TK_WHILE "W"
#define TK_DO "D"
#define TK_ODD "O"
#define TK_COMMA ","
#define TK_SEMICOLON ";"
#define TK_MULTIPLY "*"
#define TK_DIVIDE "/"
#define TK_ADD "+"
#define TK_MINUS "-"
#define TK_EQUAL "="
#define TK_GREATER ">"
#define TK_LESSER "<"
#define TK_NOTEQ "#"  // ~=
#define TK_ASSIGN ":" // :=
#define TK_LPAREN "("
#define TK_RPAREN ")"

/*
 * MISC functions
 */

static unsigned long line = 1;
char *buffer;

static void error(char *fmt, ...) {
  va_list args;

  ERED;
  fprintf(stderr, "wmoc: Error on line %lu \n", line);

  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  fputc('\n', stderr);

  exit(1);
}

static void readin(char *filename) {
  int fd;
  struct stat file_stats;

  if (strrchr(filename, '.') == NULL) {
    error(".pl0 file extension required");
  }

  if (strcmp(strrchr(filename, '.'), ".pl0") != 0) {
    error(".pl0 file extension required");
  }

  fd = open(filename, O_RDONLY);
  if (fd == -1) {
    error("Error opening a file");
  }

  if (fstat(fd, &file_stats) == -1) {
    error("Error getting the size of the file");
  }

  buffer = malloc(sizeof(char) * (file_stats.st_size + 1));
  if (buffer == NULL) {
    error("Malloc error");
  }

  if (read(fd, buffer, file_stats.st_size) != file_stats.st_size) {
    error("Cant read the data");
  }
  buffer[file_stats.st_size] = '\0';

  close(fd);
}

/* LEXER */

/*
 * MAIN FUNC
 */

int main(int argc, char **argv) {
  if (argc < 2) {
    printerr("usage: wmoc file.pl0\n");
    exit(1);
  }

  char *startpt;
  // opening the file
  readin(argv[1]);
  startpt = buffer;

  parse();

  free(startpt);
  return 0;
}
