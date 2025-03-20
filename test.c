#include <stdio.h>
static char __stdin[24];

void hello() {
  long i;
  long x;

  {
    i = 0;
    x = 72;
    (void)fprintf(stdout, "%c", (unsigned char)x);
    ;
    x = x + 29;
    (void)fprintf(stdout, "%c", (unsigned char)x);
    ;
    x = x + 7;
    while (i < 2) {
      (void)fprintf(stdout, "%c", (unsigned char)x);
      ;
      i = i + 1;
    };
    x = x + 3;
    (void)fprintf(stdout, "%c", (unsigned char)x);
    ;
    (void)fprintf(stdout, "%c", (unsigned char)44);
    ;
    (void)fprintf(stdout, "%c", (unsigned char)32);
    ;
    x = x + 8;
    (void)fprintf(stdout, "%c", (unsigned char)x);
    ;
    x = x - 8;
    (void)fprintf(stdout, "%c", (unsigned char)x);
    ;
    x = x + 3;
    (void)fprintf(stdout, "%c", (unsigned char)x);
    ;
    x = x - (3 * 2);
    (void)fprintf(stdout, "%c", (unsigned char)x);
    ;
    x = x - 8;
    (void)fprintf(stdout, "%c", (unsigned char)x);
    ;
    x = x / 3;
    (void)fprintf(stdout, "%c", (unsigned char)x);
    ;
    (void)fprintf(stdout, "%c", (unsigned char)10);
    ;
  };
}

int main(int argc, char **argv) {
  hello();
  ;
  return 0;
}

/* PL/0 Compiler WMOC v1.0.0 */
