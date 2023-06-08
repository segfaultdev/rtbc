#include <stdio.h>

// Types

typedef struct tb_arch_t tb_arch_t;

typedef struct tb_word_t tb_word_t;
typedef struct tb_type_t tb_type_t;

struct tb_arch_t {
  const char *name;
  int width; // In bytes
};

struct tb_word_t {
  
};

struct tb_type_t {
  int width; // In bytes, -1 means sizeof(ulong)
  
  int is_const, is_signed;
  
  tb_type_t *childs;
  int child_count;
};

// Enums

enum {
  tb_type_ulong,
  tb_type_long,
  
  tb_type_u8,
  tb_type_u16,
  tb_type_u32,
  tb_type_u64,
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
