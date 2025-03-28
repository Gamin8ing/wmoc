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
// I/O tokens
#define TK_PRINTINT 'p'
#define TK_PRINTCHAR 'H'
#define TK_READINT 'R'
#define TK_READCHAR 'h'
#define TK_INTO 'n'

#define WMOC_VERSION "1.0.0"

#define CHECK_LHS 0
#define CHECK_RHS 1
#define CHECK_CALL 2

/*
 * MISC functions
 */

static unsigned long line = 1;
char *buffer, *raw;
char *token;
char type;

int proc;  // to check if the curr procedure is int main in c or not
int depth; // nesting depth meter

FILE *output;

static void error(char *fmt, ...) {
  va_list args;

  ERED;
  fprintf(stderr, "wmoc: Error on line %lu \n", line);

  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  fputc('\n', stderr);
  ERESET;

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

/*
 * SYMBOL TABLE
 */

struct SymbolTable {
  int depth;
  char *name;
  int type;
  struct SymbolTable *next;
};

static struct SymbolTable *head;

static struct SymbolTable *createsymb() {
  struct SymbolTable *new;
  if ((new = (struct SymbolTable *)malloc(sizeof(struct SymbolTable))) ==
      NULL) {
    error("malloc failed");
  }
  new->next = NULL;

  return new;
}

static void initsymbtab(void) {
  struct SymbolTable *new = createsymb();

  new->type = TK_PROCEDURE;
  new->depth = 0;
  new->name = "main";

  head = new;
}

static void addsymb(int type) {
  struct SymbolTable *new = createsymb();

  struct SymbolTable *curr;
  // printf("adsymbols called with %s %c\n", token, type);

  curr = head;
  // printf("this is the pointer %p\n", curr);
  while (curr->next != NULL) {
    // printf("> %s %c\n", curr->name, curr->type);
    if (curr->type == type) {
      if (strcmp(curr->name, token) == 0) {
        if (curr->depth == depth - 1) {
          error("Duplicate symbols");
        }
      }
    }
    curr = curr->next;
  }

  // NOTE: we do depth-1 bec we increment the depth level immediately after the
  // check to make sure we don't have nested procedures and so the depth
  // variable is always one level higher than we're currently at

  new->type = type;
  new->depth = depth - 1;
  if ((new->name = strdup(token)) == NULL) {
    error("Malloc failed");
  }

  curr->next = new;
}

static void destsymb(void) {
  struct SymbolTable *curr, *prev;
again:
  curr = head;
  while (curr->next != NULL) {
    prev = curr;
    curr = curr->next;
  }

  if (curr->type != TK_PROCEDURE) {
    free(curr->name);
    free(curr);
    prev->next = NULL;
    goto again;
  }
}

/*
 * SEMANTICS
 */
static void symcheck(int check) {
  struct SymbolTable *curr, *found = NULL;

  // curr = head;
  // while (curr != NULL) {
  //   printf(">> %s %c\n", curr->name, curr->type);
  //   curr = curr->next;
  // }
  curr = head;

  // printf("SYMCHECK called %s ", token);
  while (curr != NULL) {
    if (strcmp(curr->name, token) == 0) {
      found = curr;
    }
    // printf("> %s: %s %c\n", token, curr->name, curr->type);
    curr = curr->next;
  }
  if (found == NULL) {
    error("Identifier %s not found", token);
  }
  // printf("%s %c \n", found->name, found->type);

  switch (check) {
  case CHECK_LHS:
    if (found->type != TK_VAR) {
      // printf("%s %c -< \n", found->name, found->type);
      error("Can only assign to type var: %s", token);
    }
    break;
  case CHECK_RHS:
    if (found->type == TK_PROCEDURE) {
      error("must not be a procedure: %s", token);
    }
    break;
  case CHECK_CALL:
    if (found->type != TK_PROCEDURE) {
      error("can only call a procedure: %s", token);
    }
    break;
  }
}

/*
 * LEXER
 */
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
  --raw;

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
  } else if (strcmp(token, "printInt") == 0) {
    return TK_PRINTINT;
  } else if (strcmp(token, "printChar") == 0) {
    return TK_PRINTCHAR;
  } else if (strcmp(token, "readInt") == 0) {
    return TK_READINT;
  } else if (strcmp(token, "readChar") == 0) {
    return TK_READCHAR;
  } else if (strcmp(token, "into") == 0) {
    return TK_INTO;
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

  // if (*raw != ' ' && *raw != '\t' && *raw != '\n' && *raw != ';') {
  //   error("Not a number");
  // }

  free(token);
  if ((token = malloc(len + 1)) == NULL) {
    error("Malloc failed");
  }
  for (int i = 0; i < len; i++) {
    token[i] = *(p++);
  }
  token[len] = '\0';

  --raw;
  // printf("end of the number is %c\n", *raw);
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

/*
 * Code Generator
 */

static void aout(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  (void)vfprintf(output, fmt, ap);
  va_end(ap);
}

static void cg_end(void) {
  aout("/* PL/0 Compiler WMOC v%s */\n", WMOC_VERSION);
}

static void cg_const(void) { aout("const long %s=", token); }

static void cg_symb(void) {
  switch (type) {
  case TK_IDENTIFIER:
  case TK_NUMBER:
    aout("%s", token);
    break;
  case TK_DIVIDE:
    aout("/");
    break;
  case TK_MULTIPLY:
    aout("*");
    break;
  case TK_ADD:
    aout("+");
    break;
  case TK_MINUS:
    aout("-");
    break;
  case TK_BEGIN:
    aout("{\n");
    break;
  case TK_END:
    aout(";\n}\n");
    break;
  case TK_IF:
    aout("if(");
    break;
  case TK_THEN:
  case TK_DO:
    aout(")");
    break;
  case TK_ODD:
    aout("(");
    break;
  case TK_WHILE:
    aout("while(");
    break;
  case TK_EQUAL:
    aout("==");
    break;
  case TK_COMMA:
    aout(",");
    break;
  case TK_ASSIGN:
    aout("=");
    break;
  case TK_NOTEQ:
    aout("!=");
    break;
  case TK_LESSER:
    aout("<");
    break;
  case TK_GREATER:
    aout(">");
    break;
  case TK_LPAREN:
    aout("(");
    break;
  case TK_RPAREN:
    aout(")");
  }
}

static void cg_semicolon(void) { aout(";\n"); }

static void cg_var(void) { aout("long %s;\n", token); }

static void cg_nl(void) { aout("\n"); }

static void cg_procedure(void) {
  if (proc == 0) {
    aout("int main(int argc, char **argv) ");
  } else {
    aout("void %s() ", token);
  }
  aout("{\n");
}

static void cg_epilogue(void) {
  aout(";\n");
  if (proc == 0) {
    aout("return 0;");
  }
  aout("\n}\n\n");
}

static void cg_call(void) { aout("%s();\n"); }

static void cg_odd(void) { aout(") & 1"); }

static void cg_printchar(void) {
  aout("(void) fprintf(stdout, \"%%c\", (unsigned char) %s);", token);
}

static void cg_printint(void) {
  aout("(void) fprintf(stdout, \"%%ld\\n\", (long) %s);\n", token);
}

static void cg_init(void) {
  aout("#include <stdio.h>\n");
  aout("static char __stdin[24];\n\n");
}

static void cg_readchar(void) {
  aout("(void) fprintf(stdout, \"> \";");
  aout("%s = (unsigned char) fgetc(stdin);", token);
}

static void cg_readint(void) {
  aout("(void) fprintf(stdout, \"> \";");
  aout("(void) fscanf(stdin, \"%%ld\", %s)", token);
}

/* PARSER */
// basically type is the current token and token is the current lexeme, yeah ik
// and raw is the pointer to the string received from the file

// next gives the next token
static void next() {
  type = lex();
  // printf("next token: %s %c\n", token, type);
  raw++;
}

// expect the next token as match
static void expect(char match) {
  if (match != type) {
    // printf("this is the error: %c %c \n", match, type);
    error("Syntax Error");
  }
  next();
}

static void expression();

static void factor() {
  if (type == TK_IDENTIFIER) {
    symcheck(CHECK_RHS);
    cg_symb();
    next();
  } else if (type == TK_NUMBER) {
    cg_symb();
    next();
  } else if (type == TK_LPAREN) {
    cg_symb();
    expect(TK_LPAREN);
    expression();
    if (type == TK_RPAREN) {
      cg_symb();
    }
    expect(TK_RPAREN);
  }
}

static void term() {
  // printf("a term\n");
  factor();
  while (type == TK_MULTIPLY || type == TK_DIVIDE) {
    cg_symb();
    next();
    factor();
  }
}

static void expression() {
  // printf("this is expression\n");
  if (type == TK_ADD || type == TK_MINUS) {
    cg_symb();
    next();
  }
  // printf("this is me %s %c\n", token, type);
  term();
  while (type == TK_ADD || type == TK_MINUS) {
    cg_symb();
    next();
    term();
  }
}

static void condition() {
  // printf("this is a condition\n");
  if (type == TK_ODD) {
    cg_symb();
    expect(TK_ODD);
    expression();
    cg_odd();
  } else {
    expression();
    switch (type) {
    case TK_EQUAL:
    case TK_NOTEQ:
    case TK_LESSER:
    case TK_GREATER:
      cg_symb();
      next();
      break;
    default:
      error("Invalid conditional");
    }
    expression();
  }
}

static void statement() {
  // printf("this is a statement\n");
  if (type == TK_IDENTIFIER) {
    symcheck(CHECK_LHS);
    cg_symb();
    expect(TK_IDENTIFIER);
    if (type == TK_ASSIGN)
      cg_symb();
    expect(TK_ASSIGN);
    expression();
  } else if (type == TK_CALL) {
    expect(TK_CALL);
    if (type == TK_IDENTIFIER) {
      symcheck(CHECK_CALL);
      cg_call();
    }

    expect(TK_IDENTIFIER);
  } else if (type == TK_BEGIN) {
    cg_symb();
    expect(TK_BEGIN);
    statement();
    while (type == TK_SEMICOLON) {
      cg_semicolon();
      expect(TK_SEMICOLON);
      statement();
    }
    if (type == TK_END) {
      cg_symb();
    }
    expect(TK_END);
  } else if (type == TK_IF) {
    cg_symb();
    expect(TK_IF);
    condition();
    if (type == TK_THEN) {
      cg_symb();
    }
    expect(TK_THEN);
    statement();
  } else if (type == TK_WHILE) {
    cg_symb();
    expect(TK_WHILE);
    condition();
    if (type == TK_DO) {
      cg_symb();
    }
    expect(TK_DO);
    statement();
  } else if (type == TK_PRINTINT) {
    expect(TK_PRINTINT);
    if (type == TK_IDENTIFIER || type == TK_NUMBER) {
      if (type == TK_IDENTIFIER) {
        symcheck(CHECK_RHS);
      }
      cg_printint();
    }
    if (type == TK_IDENTIFIER) {
      expect(TK_IDENTIFIER);
    } else if (type == TK_NUMBER) {
      expect(TK_NUMBER);
    } else {
      error("Expected an identifier or a number");
    }
  } else if (type == TK_PRINTCHAR) {
    expect(TK_PRINTCHAR);
    if (type == TK_IDENTIFIER || type == TK_NUMBER) {
      if (type == TK_IDENTIFIER) {
        symcheck(CHECK_RHS);
      }
      cg_printchar();
    }
    if (type == TK_IDENTIFIER) {
      expect(TK_IDENTIFIER);
    } else if (type == TK_NUMBER) {
      expect(TK_NUMBER);
    } else {
      error("Expected an identifier or a number");
    }
  } else if (type == TK_READINT) {
    expect(TK_READINT);
    expect(TK_INTO);
    if (type == TK_IDENTIFIER) {
      symcheck(CHECK_LHS);
      cg_readint();
    }
    expect(TK_IDENTIFIER);
  } else if (type == TK_READCHAR) {
    expect(TK_READCHAR);
    expect(TK_INTO);
    if (type == TK_IDENTIFIER) {
      symcheck(CHECK_LHS);
      cg_readchar();
    }
    expect(TK_IDENTIFIER);
  }
}

// for processing the blocks in the language
static void block() {
  // printf("this is a block \n");

  if (depth++ > 3) {
    error("Nesting depth increased");
  }
  // checking for const lines
  if (type == TK_CONST) {
    expect(TK_CONST);
    if (type == TK_IDENTIFIER) {
      addsymb(TK_CONST);
      cg_const();
    }
    expect(TK_IDENTIFIER);
    expect(TK_EQUAL);
    if (type == TK_NUMBER) {
      cg_symb();
      cg_semicolon();
    }
    expect(TK_NUMBER);
    while (type == TK_COMMA) {
      expect(TK_COMMA);
      if (type == TK_IDENTIFIER) {
        addsymb(TK_CONST);
        cg_const();
      }
      expect(TK_IDENTIFIER);
      expect(TK_EQUAL);
      if (type == TK_NUMBER) {
        cg_symb();
        cg_semicolon();
      }
      expect(TK_NUMBER);
    }
    expect(TK_SEMICOLON);
  }

  // now var lines
  if (type == TK_VAR) {
    expect(TK_VAR);
    if (type == TK_IDENTIFIER) {
      addsymb(TK_VAR);
      cg_var();
    }
    expect(TK_IDENTIFIER);
    while (type == TK_COMMA) {
      expect(TK_COMMA);
      if (type == TK_IDENTIFIER) {
        addsymb(TK_VAR);
        cg_var();
      }
      expect(TK_IDENTIFIER);
    }
    expect(TK_SEMICOLON);
    cg_nl();
  }

  while (type == TK_PROCEDURE) {
    proc = 1;
    expect(TK_PROCEDURE);
    if (type == TK_IDENTIFIER) {
      addsymb(TK_PROCEDURE);
      cg_procedure();
    }
    expect(TK_IDENTIFIER);
    expect(TK_SEMICOLON);
    block();
    expect(TK_SEMICOLON);
    proc = 0;
    destsymb();
  }
  if (proc == 0) {
    cg_procedure();
  }

  statement();
  cg_epilogue();

  if (--depth < 0) {
    error("nesting depth fell below 0, huh?");
  }
  // expect(TK_DOT);
}

static void parse(void) {
  // first enforcing the block . rule in the program
  cg_init();
  next();
  block();
  // printf("this is the last token %s\n", token);
  expect(TK_DOT);

  if (type != '\0') {
    printf("Extra tokens present at the end of the file\n");
  }

  cg_end();
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

  // opening the output file
  output = fopen(argv[2], "w");
  initsymbtab();

  parse();

  free(startpt);
  return 0;
}
