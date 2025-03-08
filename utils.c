#include <stdio.h>

// Error macro, to tell there is an error, make sure to include <stdio.h>
#define ERROR(err) fprintf(stderr, "\033[1;31mERROR! \033[0m%s\n", err)

// Warning macro, to warn the user, requires <stdio.h>
#define WARN(warning) fprintf(stderr, "\033[1;33mWARNING! \033[0m%s\n", warning)

// color utils

// yellow colour
#define YELLOW printf("\033[0;33m")

// red colour, should mostly be used for error only
#define RED printf("\033[0;31m")

// green color, should be used for success messages
#define GREEN printf("\033[0;32m")

// blue color
#define BLUE printf("\033[0;34m")

// black color
#define BLACK printf("\033[0;30m")

// purple color
#define PURPLE printf("\033[0;35m")

// cyan color
#define CYAN printf("\033[0;36m")

// white color
#define WHITE printf("\033[0;37m")

// reset color
#define RCOLOR printf("\033[0m")

// bold color
#define BOLD printf("\033[1m")

// underline color
#define UNDERLINE printf("\033[4m")

// blink color
#define BLINK printf("\033[5m")

// reverse color
#define REVERSE printf("\033[7m")

// hidden color
#define HIDDEN printf("\033[8m")

// clear the screen
#define CLEAR printf("\033[2J")

// clear the line
#define CLEARLINE printf("\033[2K")

// reset the color
#define RESET printf("\033[0m")

// ------------- ERRROR HANDLING -------------------
// red in stderr
#define ERED fprintf(stderr, "\033[31m")

// reset in stderr
#define ERESET fprintf(stderr, "\033[0m")

void printerr(char *s) {
  ERED;
  fprintf(stderr, "%s", s);
  ERESET;
}
