#include <rtbc.h>

#define f_parse_error(format, word, ...) f_error("(At '%s', line %d, column %d) " format, source->files[word.file], word.line, word.column, __VA_ARGS__)

static int expect(source_t *source, int type, word_t *word) {
  if (source->word_index == source->word_count) {
    return 0;
  }
  
  if (source->words[source->word_index].type == type) {
    if (word) {
      *word = source->words[source->word_index];
    }
    
    source->word_index++;
    return 1;
  }
  
  return 0;
}

int f_parse_type(const arch_t *arch, source_t *source, type_t *type) {
  if (expect(source, k_ul)) {
    type->base_width = arch->max_width;
    type->base_signed = 0;
  } else if (expect(source, k_l)) {
    type->base_width = arch->max_width;
    type->base_signed = 1;
  } else if (expect(source, k_u8)) {
    type->base_width = 1;
    type->base_signed = 0;
  } else if (expect(source, k_u16)) {
    type->base_width = 2;
    type->base_signed = 0;
  } else if (expect(source, k_u32)) {
    type->base_width = 4;
    type->base_signed = 0;
  } else if (expect(source, k_u64)) {
    type->base_width = 8;
    type->base_signed = 0;
  } else {
    return 0;
  }
  
  type->point_count = 0;
  
  while (expect(source, s_mul)) {
    type->point_count++;
  }
  
  return 1;
}

void f_parse_root(const arch_t *arch, source_t *source) {
  type_t type;
  word_t word;
  
  for (;;) {
    if (f_parse_type(arch, source, &type)) {
      if (!expect(source, l_name, &word)) {
        f_parse_error();
      }
    }
  }
}
