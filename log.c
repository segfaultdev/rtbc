#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <rtbc.h>

int f_do_debug = 0;

void f_error(const char *format, ...) {
  va_list args;
  va_start(args, format); 
  
  fprintf(stderr, "Error: ");
  vfprintf(stderr, format, args);
  
  va_end(args);
  exit(1);
}

void f_debug(const char *format, ...) {
  if (!f_do_debug) {
    return;
  }
  
  va_list args;
  va_start(args, format); 
  
  vfprintf(stderr, format, args);
  va_end(args);
}
