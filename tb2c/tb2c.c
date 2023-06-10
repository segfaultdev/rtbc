#include <stdio.h>

// Types

typedef struct tb_source_t tb_source_t;
typedef struct tb_arch_t tb_arch_t;

typedef struct tb_word_t tb_word_t;
typedef struct tb_type_t tb_type_t;

struct tb_source_t {
  char **file_names;
  int file_count;
  
  tb_word_t *word_stack;
  int word_count, word_limit;
  
  
};

struct tb_arch_t {
  const char *name;
  int width; // In bytes
};

struct tb_word_t {
  int type;
  int file, line, column;
  
  union {
    char name[12];
    
    size_t ul;
    ssize_t l;
    char chr;
    int str; // Offset
  };
};

struct tb_type_t {
  int width; // In bytes, -1 means sizeof(ulong)
  
  int is_const, is_signed;
  
  tb_type_t *childs;
  int child_count;
};

// Enums

enum {
  w_invalid,
  
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
  
  s_l_shift,  // <
  s_r_shift,  // >
  s_l_rotate, // <<
  s_r_rotate, // >>
  
  s_not, // !
  s_and, // &
  s_or,  // \
  s_xor, // ^
  
  s_add, // +
  s_sub, // -
  s_mul, // *
  s_div, // /
  s_mod, // %
  
  s_inc, // ++
  s_dec, // --
  
  s_assign,    // =
  s_chr_quote, // '
  s_str_quote, // "
  
  // Keywords:
  
  k_const,
  k_ulong,
  k_long,
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
  k_give,
  
  k_once,
  k_include,
  k_macro,
  k_enum,
};

enum {
  t_ul,
  t_l,
  t_u8,
  t_u16,
  t_u32,
  t_u64,
};

// Consts

const tb_arch_t archs[] = {
  (tb_arch_t){
    .name = "SC/MP",
    .width = 2,
  },
  
  (tb_arch_t){
    .name = "6502",
    .width = 2,
  },
  
  (tb_arch_t){
    .name = "x86",
    .width = 4,
  },
  
  (tb_arch_t){
    .name = "x86_64",
    .width = 8,
  },
};

const char *words[] = {
  "ul", "l", "u8", "u16", "u32", "u64",
  "ifz", "ifnz", "ifp", "ifnp", "whz", "whnz", "whp", "whnp",
  "else", "goto", "break", "next", "give",
  "once", "use", "macro", "enum",
};

const tb_type_t types[] = {
  (tb_type_t){
    .width = -1,
    
    .is_const = 0,
    .is_signed = 0,
    
    .childs = NULL,
    .child_count = 0,
  },
  
  (tb_type_t){
    .width = -1,
    
    .is_const = 0,
    .is_signed = 1,
    
    .childs = NULL,
    .child_count = 0,
  },
  
  (tb_type_t){
    .width = 1,
    
    .is_const = 0,
    .is_signed = 0,
    
    .childs = NULL,
    .child_count = 0,
  },
  
  (tb_type_t){
    .width = 2,
    
    .is_const = 0,
    .is_signed = 0,
    
    .childs = NULL,
    .child_count = 0,
  },
  
  (tb_type_t){
    .width = 4,
    
    .is_const = 0,
    .is_signed = 0,
    
    .childs = NULL,
    .child_count = 0,
  },
  
  (tb_type_t){
    .width = 8,
    
    .is_const = 0,
    .is_signed = 0,
    
    .childs = NULL,
    .child_count = 0,
  },
};

// Globals

// Functions

static void tb_type_push(tb_type_t *type, tb_type_t child) {
  type->childs = realloc(type->childs, (type->child_count + 1) * sizeof(tb_type_t));
  type->childs[type->child_count++] = child;
}

static void tb_type_free(tb_type_t type) {
  for (int i = 0; i < type.child_count; i++) {
    tb_type_free(type.childs[i]);
  }
  
  if (type.child_count) {
    free(type.childs);
  }
}

static tb_type_t tb_type_clone(tb_type_t type) {
  // TODO
}

// Main

int main(void) {
  
}
