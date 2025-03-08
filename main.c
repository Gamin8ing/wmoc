#include <cmath>
#include <ctype.h>
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
#define TK_DOT '.'
#define TK_IDENTIFIER 'I'
#define TK_NUMBER 'N'
#define TK_CONST 'C'
#define TK_VAR 'V'
#define TK_PROCEDURE 'P'
#define TK_CALL 'c'
#define TK_BEGIN 'B'
#define TK_END 'E'
#define TK_IF 'i'
#define TK_THEN 'T'
#define TK_WHILE 'W'
#define TK_DO 'D'
#define TK_ODD 'O'
#define TK_COMMA ','
#define TK_SEMICOLON ';'
#define TK_MULTIPLY '*'
#define TK_DIVIDE '/'
#define TK_ADD '+'
#define TK_MINUS '-'
#define TK_EQUAL '='
#define TK_GREATER '>'
#define TK_LESSER '<'
#define TK_NOTEQ '#'  // ~=
#define TK_ASSIGN ':' // :=
#define TK_LPAREN '('
#define TK_RPAREN ')'

/*
 * MISC functions
 */

static unsigned long line = 1;
char *buffer, *raw;
char *token;
char type;

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
  raw = buffer;

  close(fd);
}

/* LEXER */
void comment() {
  char ch;
  while ((ch = *raw++) != '}') {
    if (ch == '\0') {
      error("undetermined comment");
    }

    if (ch == '\n')
      line++;
  }
}

static int identifier() {
  char *ptemp;
  ptemp = raw;
  int len = 0;
  while (isalnum(*raw) || *raw == '_') {
    len++;
    raw++;
  }
  free(token);
  if ((token = malloc(len + 1)) == NULL) {
    error("Malloc failed");
  }
  for (int i = 0; i < len; i++) {
    token[i] = *ptemp++;
  }
  token[len] = '\0';

  if (strcmp(token, "if") == 0) {
    return TK_IF;
  } else if (strcmp(token, "do") == 0) {
    return TK_DO;
  } else if (strcmp(token, "then") == 0) {
    return TK_THEN;
  } else if (strcmp(token, "while") == 0) {
    return TK_WHILE;
  } else if (strcmp(token, "odd") == 0) {
    return TK_ODD;
  } else if (strcmp(token, "const") == 0) {
    return TK_CONST;
  } else if (strcmp(token, "call") == 0) {
    return TK_CALL;
  } else if (strcmp(token, "procedure") == 0) {
    return TK_PROCEDURE;
  } else if (strcmp(token, "var") == 0) {
    return TK_VAR;
  } else if (strcmp(token, "begin") == 0) {
    return TK_BEGIN;
  } else if (strcmp(token, "end") == 0) {
    return TK_END;
  } else {
    return TK_IDENTIFIER;
  }
}

static int number() {
  char *p;
  p = raw;
  int len = 0;
  while (isdigit(*raw) || *raw == '_') {
    raw++;
    len++;
  }

  if (*raw != ' ' && *raw != '\t' && *raw != '\n') {
    error("Not a number");
  }

  free(token);
  if ((token = malloc(len + 1)) == NULL) {
    error("Malloc failed");
  }
  for (int i = 0; i < len; i++) {
    token[i] = *p++;
  }
  token[len] = '\0';

  return TK_NUMBER;
}

static int lex() {
redo:

  while (*raw == ' ' || *raw == '\t' || *raw == '\n') {
    if (*raw++ == '\n')
      line++;
  }

  if (*raw == '{') {
    comment();
    goto redo;
  }

  if (isalpha(*raw) || *raw == '_') {
    return identifier();
  }

  if (isdigit(*raw)) {
    return number();
  }

  switch (*raw) {
  case '*':
  case '/':
  case '+':
  case ';':
  case '-':
  case '(':
  case ')':
  case '<':
  case '>':
  case '.':
  case ',':
  case '=':
  case '#':
    return (*raw);
  case ':':
    if (*(++raw) != '=') {
      error("Invalid token ':%c'", *raw);
    } else {
      return TK_ASSIGN;
    }
  case '\0':
    return 0;
  default:
    error("Unidentified token");
  }
  return 0;
}

/* PARSER */
static void next() {
  type = lex();
  raw++;
}

static void expect(char match) {
  if (match != type) {
    error("Syntax Error");
  }
  next();
}

static void parser() {
  while ((type = lex()) != 0) {
    raw++;
    (void)fprintf(stdout, "%lu|%d\t", line, type);
    switch (type) {
    case TK_IDENTIFIER:
    case TK_NUMBER:
    case TK_CONST:
    case TK_VAR:
    case TK_PROCEDURE:
    case TK_CALL:
    case TK_BEGIN:
    case TK_END:
    case TK_IF:
    case TK_THEN:
    case TK_WHILE:
    case TK_DO:
    case TK_ODD:
      (void)fprintf(stdout, "%s", token);
      break;
    case TK_DOT:
    case TK_EQUAL:
    case TK_COMMA:
    case TK_SEMICOLON:
    case TK_MINUS:
    case TK_NOTEQ:
    case TK_LESSER:
    case TK_GREATER:
    case TK_ADD:
    case TK_MULTIPLY:
    case TK_DIVIDE:
    case TK_LPAREN:
    case TK_RPAREN:
      (void)fputc(type, stdout);
      break;
    case TK_ASSIGN:
      (void)fputs(":=", stdout);
    }
    (void)fputc('\n', stdout);
  }
}

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

  parser();

  free(startpt);
  return 0;
}
