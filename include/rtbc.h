#ifndef __RTBC_H__
#define __RTBC_H__

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_LENGTH 15

// log.c

extern int f_do_debug;

void f_error(const char *format, ...);
void f_debug(const char *format, ...);

// arch.c

typedef struct arch_t arch_t;

struct arch_t {
  char name[MAX_LENGTH + 1];
  
  int max_width;   // Largest possible value type width.
  int point_width; // For pointers.
  int exit_width;  // For return values (a limit, not a minimum).
  
  int is_big; // High if big endian, little endian otherwise.
  
  void (*f_label)(const char *name);
};

// parse.c

typedef struct type_t type_t;

struct type_t {
  int base_width, base_signed;
  int point_count;
};

void f_parse_root(source_t *source);

// source.c

typedef struct source_t source_t;
typedef struct macro_t macro_t;
typedef struct word_t word_t;

struct source_t {
  char **files;
  int file_count;
  
  word_t *words;
  int word_index, word_count;
  
  macro_t *macros;
  int macro_count;
  
  char *data_buffer;
  int data_length;
};

struct macro_t {
  char name[MAX_LENGTH + 1];
  
  word_t *words;
  int word_count;
};

struct word_t {
  int type;
  int file, line, column;
  
  union {
    char name[MAX_LENGTH + 1];
    
    size_t ul;
    ssize_t l;
    size_t chr;
    int str; // Offset
  };
};

enum {
  w_invalid,
  w_comment,
  
  w_at,
  w_chr_slash,
  w_str_slash,
  
  // Literals:
  
  l_name,
  l_ul,
  l_l,
  l_chr,
  l_str,
  
  // Symbols:
  
  s_l_paren, // (
  s_r_paren, // )
  s_a_paren, // @(
  
  s_l_bracket, // [
  s_r_bracket, // ]
  s_a_bracket, // @[
  
  s_colon,     // :
  s_semicolon, // ;
  s_comma,     // ,
  s_exit,      // @;
  
  s_l_shift,  // <
  s_r_shift,  // >
  s_l_rotate, // @<
  s_r_rotate, // @>
  
  s_not, // !
  s_and, // &
  s_or,  // '\'
  s_xor, // ^
  
  s_add, // +
  s_sub, // -
  s_mul, // *
  s_div, // /
  s_mod, // %
  
  s_inc, // @+
  s_dec, // @-
  
  s_assign, // =
  
  // Keywords:
  
  k_ul,
  k_l,
  k_u8,
  k_u16,
  k_u32,
  k_u64,
  
  k_ifz,
  k_ifnz,
  k_ifp,
  k_ifnp,
  k_whz,
  k_whnz,
  k_whp,
  k_whnp,
  
  k_else,
  k_goto,
  k_break,
  k_next,
  
  k_only,
  k_use,
  k_macro,
  k_enum,
  
  // Other:
  
  w_count,           // Word count
  w_keywords = k_ul, // Keywords start
};

void f_source_load(source_t *source, const char *path);

#endif
