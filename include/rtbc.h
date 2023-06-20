#ifndef __RTBC_H__
#define __RTBC_H__

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_LENGTH 15

typedef struct source_t source_t;
typedef struct macro_t macro_t;
typedef struct word_t word_t;

typedef struct context_t context_t;
typedef struct entry_t entry_t;
typedef struct const_t const_t;
typedef struct enum_t enum_t;
typedef struct type_t type_t;

typedef struct arch_t arch_t;

// log.c

extern int f_do_debug;

void f_error(const char *format, ...);
void f_debug(const char *format, ...);

// source.c

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
    
    uint64_t ux;
    int64_t x;
    uint64_t chr;
    uint64_t str; // Offset (in DATA)
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
  l_ux,
  l_x,
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
  
  k_us,
  k_s,
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
  k_break,
  k_next,
  
  k_only,
  k_use,
  k_macro,
  
  // Other:
  
  w_count,           // Word count
  w_keywords = k_us, // Keywords start
};

void f_source_load(source_t *source, const char *path);

// parse.c

struct type_t {
  int base_width, base_signed;
  int point_count;
};

struct const_t {
  type_t type;
  int is_data;
  
  union {
    uint64_t ux;
    int64_t x;
    uint64_t offset; // Offset (in DATA) -> Results in value being (DATA + offset)
  };
};

struct entry_t {
  char name[MAX_LENGTH + 1];
  type_t type;
  
  union {
    int is_routine;
    int offset; // Negative offsets are locals, positive ones are arguments, 0 is our exit pointer.
  };
};

struct context_t {
  entry_t *globals;
  int global_count;
  
  entry_t *locals;
  int local_count;
  
  enum_t *enums;
  int enum_count;
};

int  f_type_size(const arch_t *arch, type_t type);
void f_parse_root(const arch_t *arch, source_t *source);

// Architecture stuff

struct arch_t {
  char name[MAX_LENGTH + 1];
  
  int data_width;  // Data bus / return size (used for us and s).
  int point_width; // Pointer size (used for ul and l).
  
  int is_big; // High if big endian, little endian otherwise.
  
  void (*f_init)(void);
  
  void (*f_global)(const char *name);
  void (*f_const)(const_t value);
  void (*f_data)(const void *data, int length);
  
  void (*f_init_routine)(int offset);
  void (*f_exit_routine)(void);
  
  void (*f_load_const)(const_t value);
  void (*f_load_local)(int width, int offset);
  void (*f_push)(int width);
  void (*f_pull)(int width);
  void (*f_call)(int offset);
  
  void (*f_zero_extend)(int new_width, int old_width);
  void (*f_sign_extend)(int new_width, int old_width);
  
  int  (*f_next)(void);
  void (*f_label)(int label);
  
  void (*f_jump)(int label);
  void (*f_jump_z)(int width, int label);
  void (*f_jump_nz)(int width, int label);
  void (*f_jump_p)(int width, int label);
  void (*f_jump_np)(int width, int label);
};

#endif
