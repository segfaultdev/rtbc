#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <rtbc.h>

#define _f_parse_error(format, word, ...) f_error("(At '%s', line %d, column %d) " format, source->files[word.file], word.line, word.column, __VA_ARGS__)
#define f_parse_error(format, ...) _f_parse_error(format, __VA_ARGS__, 0)

#define last_word (source->words[source->word_index - 1])
#define curr_word (source->words[source->word_index])

int f_type_size(const arch_t *arch, type_t type) {
  int width = type.base_width;
  
  if (type.point_count) {
    width = arch->point_width;
  }
  
  return width;
}

static int expect(source_t *source, int type, word_t *word) {
  if (source->word_index == source->word_count) {
    return 0;
  }
  
  if (curr_word.type == type) {
    if (word) {
      *word = curr_word;
    }
    
    source->word_index++;
    return 1;
  }
  
  return 0;
}

static void skip_block(source_t *source) {
  for (;;) {
    if (expect(source, s_l_paren, NULL) || expect(source, s_a_paren, NULL)) {
      skip_block(source);
    } else if (expect(source, s_r_paren, NULL)) {
      return;
    } else if (source->word_index == source->word_count) {
      f_parse_error("Expected closing parenthesis.\n", last_word);
    } else {
      source->word_index++;
    }
  }
}

int f_parse_type(const arch_t *arch, source_t *source, type_t *type) {
  if (expect(source, k_us, NULL)) {
    type->base_width = arch->data_width;
    type->base_signed = 0;
  } else if (expect(source, k_s, NULL)) {
    type->base_width = arch->data_width;
    type->base_signed = 1;
  } else if (expect(source, k_ul, NULL)) {
    type->base_width = arch->point_width;
    type->base_signed = 0;
  } else if (expect(source, k_l, NULL)) {
    type->base_width = arch->point_width;
    type->base_signed = 1;
  } else if (expect(source, k_u8, NULL)) {
    type->base_width = 1;
    type->base_signed = 0;
  } else if (expect(source, k_u16, NULL)) {
    type->base_width = 2;
    type->base_signed = 0;
  } else if (expect(source, k_u32, NULL)) {
    type->base_width = 4;
    type->base_signed = 0;
  } else if (expect(source, k_u64, NULL)) {
    type->base_width = 8;
    type->base_signed = 0;
  } else {
    return 0;
  }
  
  type->point_count = 0;
  
  while (expect(source, s_mul, NULL)) {
    type->point_count++;
  }
  
  return 1;
}

static const_t cast(const arch_t *arch, type_t type, const_t value) {
  int type_width = f_type_size(arch, type);
  int width = f_type_size(arch, value.type);
  
  if (value.is_data && type_width < width) {
    f_error("Cannot cast DATA-relative address to smaller size.\n");
  }
  
  if (type_width > width) {
    value.ux &= (((uint64_t)(1)) << (width * 8)) - 1;
    int sign_bit = (value.ux >> (width * 8 - 1)) & 1;
    
    if (value.type.base_signed && type.base_signed && sign_bit) {
      value.ux |= ((~((uint64_t)(0))) << (width * 8));
    }
  }
  
  value.type = type;
  return value;
}

static type_t type_max(const arch_t *arch, type_t type_a, type_t type_b) {
  if (type_a.point_count && type_b.point_count) {
    if (type_a.base_width != type_b.base_width ||
        type_a.base_signed != type_b.base_signed ||
        type_a.point_count != type_b.point_count) {
      f_error("Cannot operate with two pointers of different type.\n");
    }
    
    return type_a;
  } else if (type_a.point_count) {
    return type_a;
  } else if (type_b.point_count) {
    return type_b;
  }
  
  return (type_t){
    .base_width = (type_a.base_width > type_b.base_width ? type_a.base_width : type_b.base_width),
    .base_signed = (type_a.base_signed && type_b.base_signed),
    
    .point_count = 0,
  };
}

static const_t f_parse_const_0(const arch_t *arch, source_t *source) {
  word_t word;
  
  if (expect(source, l_name, &word)) {
    f_parse_error("Constant expressions cannot contain lvalues, found '%s'.\n", word, word.name);
  } else if (expect(source, l_ux, &word) || expect(source, l_x, &word) || expect(source, l_chr, &word)) {
    int min_width = 0;
    
    while (word.ux >= (((uint64_t)(1)) << min_width)) {
      min_width++;
    }
    
    if (word.type == l_x) {
      min_width++;
    }
    
    min_width = (min_width + 7) / 8;
    
    return (const_t){
      .type = (type_t){
        .base_width = min_width,
        .base_signed = (word.type == l_x),
        
        .point_count = 0,
      },
      
      .is_data = 0,
      .ux = word.ux,
    };
  }
  
  f_parse_error("Expected constant expression.\n", curr_word);
}

static const_t f_parse_const(const arch_t *arch, source_t *source) {
  return f_parse_const_0(arch, source);
}

static void f_cast(const arch_t *arch, source_t *source, type_t old_type, type_t new_type) {
  int old_width = f_type_size(arch, old_type);
  int new_width = f_type_size(arch, new_type);
  
  if (old_type.point_count || new_type.point_count) {
    old_type.base_signed = 0;
  }
  
  if (new_width > old_width) {
    if (old_type.base_signed && new_type.base_signed) {
      arch->f_sign_extend(new_width, old_width);
    } else {
      arch->f_zero_extend(new_width, old_width);
    }
  }
}

static type_t f_parse_expr_0(const arch_t *arch, source_t *source) {
  type_t type;
  word_t word;
  
  if (expect(source, l_name, &word)) {
    // TODO
  } else if (expect(source, l_ux, &word) || expect(source, l_x, &word) || expect(source, l_chr, &word)) {
    int min_width = 1;
    
    while (word.ux >= (((uint64_t)(1)) << min_width)) {
      min_width++;
    }
    
    if (word.type == l_x) {
      min_width++;
    }
    
    min_width = (min_width + 7) / 8;
    
    type_t type = (type_t){
      .base_width = min_width,
      .base_signed = (word.type == l_x),
      
      .point_count = 0,
    };
    
    arch->f_load_const((const_t){
      .type = type,
      
      .is_data = 0,
      .ux = word.ux,
    });
    
    return type;
  } else if (expect(source, s_l_paren, NULL)) {
    if (f_parse_type(arch, source, &type)) {
      // Cast!
    } else {
      // Just a regular parenthesized expression...
    }
  }
  
  f_parse_error("Expected expression.\n", curr_word);
}

static int f_parse_expr(const arch_t *arch, source_t *source, type_t exit_type, int exit_label, int in_root) {
  type_t type = f_parse_expr_0(arch, source);
  
  if (expect(source, s_semicolon, NULL)) {
    return exit_label;
  } else if (expect(source, s_exit, NULL)) {
    f_cast(arch, source, type, exit_type);
    
    if (in_root) {
      return -2;
    }
    
    if (exit_label < 0) {
      exit_label = arch->f_next();
    }
    
    arch->f_jump(exit_label);
    return exit_label;
  }
  
  f_parse_error("Expected semicolon or exit after local statement.\n", curr_word);
}

static void f_parse_routine(const arch_t *arch, source_t *source, context_t *context, type_t exit_type, const char *name) {
  int arg_offset = arch->point_width; // Shift one pointer forward (return address!).
  int local_offset = 0;
  
  type_t type;
  word_t word;
  
  context->locals = NULL;
  context->local_count = 0;
  
  if (f_type_size(arch, exit_type) > arch->data_width) {
    f_parse_error("Return values cannot be larger than %d bytes.\n", last_word, arch->data_width);
  }
  
  for (;;) {
    if (expect(source, s_r_paren, NULL)) {
      break;
    } else if (expect(source, s_comma, NULL)) {
      if (!context->local_count) {
        f_parse_error("Unexpected comma.\n", last_word);
      }
    }
    
    if (!f_parse_type(arch, source, &type)) {
      f_parse_error("Expected proper type.\n", curr_word);
    }
    
    if (expect(source, l_name, &word)) {
      for (int i = 0; i < context->local_count; i++) {
        if (!strcmp(context->locals[i].name, word.name)) {
          f_parse_error("Argument '%s' already exists.\n", word, word.name);
        }
      }
      
      entry_t entry = (entry_t){
        .type = type,
        .offset = arg_offset,
      };
      
      strcpy(entry.name, word.name);
      
      context->locals = realloc(context->locals, (context->local_count + 1) * sizeof(entry_t));
      context->locals[context->local_count++] = entry;
    }
    
    arg_offset += f_type_size(arch, type);
  }
  
  if (expect(source, s_semicolon, NULL)) {
    // TODO: We *might* try to make something out of this? (header momento)
    
    free(context->locals);
    return;
  }
  
  if (expect(source, s_colon, NULL)) {
    if (!expect(source, s_l_paren, NULL)) {
      f_parse_error("Expected opening parenthesis in local declaration.\n", curr_word);
    }
    
    for (;;) {
      if (expect(source, s_r_paren, NULL)) {
        break;
      } else if (expect(source, s_comma, NULL)) {
        if (!arg_offset) {
          f_parse_error("Unexpected comma.\n", last_word);
        }
      }
      
      if (!f_parse_type(arch, source, &type)) {
        f_parse_error("Expected proper type.\n", curr_word);
      }
      
      local_offset += f_type_size(arch, type);
      
      if (expect(source, l_name, &word)) {
        for (int i = 0; i < context->local_count; i++) {
          if (!strcmp(context->locals[i].name, word.name)) {
            f_parse_error("Local '%s' already exists.\n", word, word.name);
          }
        }
        
        entry_t entry = (entry_t){
          .type = type,
          .offset = -local_offset,
        };
        
        strcpy(entry.name, word.name);
        
        context->locals = realloc(context->locals, (context->local_count + 1) * sizeof(entry_t));
        context->locals[context->local_count++] = entry;
      }
    }
  }
  
  arch->f_global(name);
  arch->f_init_routine(local_offset);
  
  int exit_label = -1;
  
  if (!expect(source, s_a_paren, NULL)) {
    f_parse_error("Expected opening code block in function declaration.\n", curr_word);
  }
  
  for (;;) {
    if (expect(source, s_r_paren, NULL)) {
      break;
    }
    
    exit_label = f_parse_expr(arch, source, exit_type, exit_label, 1);
    
    if (exit_label == -2) {
      skip_block(source);
      break;
    }
  }
  
  if (exit_label >= 0) {
    arch->f_label(exit_label);
  }
  
  arch->f_exit_routine();
}

static void f_parse_global(const arch_t *arch, source_t *source, context_t *context, type_t type, const char *name) {
  const_t value = (const_t){
    .type = type,
    .is_data = 0,
    
    .ux = 0,
  };
  
  if (expect(source, s_assign, NULL)) {
    value = cast(arch, type, f_parse_const(arch, source));
  }
  
  arch->f_global(name);
  arch->f_const(value);
}

void f_parse_root(const arch_t *arch, source_t *source) {
  context_t context = (context_t){
    .globals = NULL,
    .global_count = 0,
    
    .locals = NULL,
    .local_count = 0,
    
    .enums = NULL,
    .enum_count = 0,
  };
  
  type_t type;
  word_t word;
  
  arch->f_init();
  
  while (source->word_index < source->word_count) {
    if (f_parse_type(arch, source, &type)) {
      if (!expect(source, l_name, &word)) {
        f_parse_error("Expected identifier after type.\n", curr_word);
      }
      
      if (expect(source, s_l_paren, NULL)) {
        f_parse_routine(arch, source, &context, type, word.name);
      } else {
        for (;;) {
          f_parse_global(arch, source, &context, type, word.name);
          
          if (expect(source, s_comma, NULL)) {
            if (!expect(source, l_name, &word)) {
              f_parse_error("Expected identifier after comma.\n", curr_word);
            }
          } else {
            break;
          }
        }
      }
    } else {
      
    }
    
    if (!expect(source, s_semicolon, NULL)) {
      f_parse_error("Expected semicolon after global statement.\n", curr_word);
    }
  }
  
  if (source->data_length) {
    arch->f_global("DATA");
    arch->f_data(source->data_buffer, source->data_length);
  }
}
