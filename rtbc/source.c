#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <rtbc.h>

#define f_source_error(format, ...) f_error("(At '%s', line %d, column %d) " format, path, word.line, word.column, __VA_ARGS__)

static const char *word_types[] = {
  "W_INVALID",
  "W_COMMENT",
  "W_AT",
  "W_CHR_SLASH",
  "W_STR_SLASH",
  "L_NAME",
  "L_UL",
  "L_L",
  "L_CHR",
  "L_STR",
  "S_L_PAREN",
  "S_R_PAREN",
  "S_A_PAREN",
  "S_L_BRACKET",
  "S_R_BRACKET",
  "S_A_BRACKET",
  "S_COLON",
  "S_SEMICOLON",
  "S_COMMA",
  "S_EXIT",
  "S_L_SHIFT",
  "S_R_SHIFT",
  "S_L_ROTATE",
  "S_R_ROTATE",
  "S_NOT",
  "S_AND",
  "S_OR",
  "S_XOR",
  "S_ADD",
  "S_SUB",
  "S_MUL",
  "S_DIV",
  "S_MOD",
  "S_INC",
  "S_DEC",
  "S_ASSIGN",
  "K_UL",
  "K_L",
  "K_U8",
  "K_U16",
  "K_U32",
  "K_U64",
  "K_IFZ",
  "K_IFNZ",
  "K_IFP",
  "K_IFNP",
  "K_WHZ",
  "K_WHNZ",
  "K_WHP",
  "K_WHNP",
  "K_ELSE",
  "K_GOTO",
  "K_BREAK",
  "K_NEXT",
  "K_ONLY",
  "K_USE",
  "K_MACRO",
  "K_ENUM",
};

void f_source_load(source_t *source, const char *path) {
  FILE *file = fopen(path, "r");
  int file_id = source->file_count++;
  
  if (!file) {
    f_error("Cannot open file: '%s'\n", path);
  }
  
  source->files = realloc(source->files, source->file_count * sizeof(char *));
  source->files[file_id] = strdup(path);
  
  int is_once = 1;
  
  for (int i = 0; i < file_id; i++) {
    if (!strcmp(source->files[i], source->files[file_id])) {
      is_once = 0;
      break;
    }
  }
  
  int file_line = 1, file_column = 1;
  
  word_t word = (word_t){
    .type = w_invalid,
    .file = file_id,
    
    .line = file_line,
    .column = file_column,
  };
  
  int last_only = -1, last_use = -1;
  int only_not = 0;
  
  ssize_t temp = 0; // Length for names and strings, bases for numbers, etc.
  int reuse_chr = 0, done = 0;
  
  const char *digits = "0123456789ABCDEF";
  char chr;
  
  f_debug("File '%s':\n", path);
  
  for (;;) {
    if (word.type != w_invalid && done) {
      // Name-to-keyword section:
      
      if (word.type == l_name) {
        word.name[temp] = '\0';
        
        for (int i = w_keywords; i < w_count; i++) {
          if (!strcmp(word.name, word_types[i] + 2)) {
            word.type = i;
            break;
          }
        }
      }
      
      // Word debugging section:
      
      f_debug("  [%-11s", word_types[word.type]);
      
      if (word.type == l_name) {
        f_debug(": %s", word.name);
      } else if (word.type == l_ul) {
        f_debug(": %lu", word.ul);
      } else if (word.type == l_l) {
        f_debug(": %ld", word.l);
      } else if (word.type == l_chr) {
        f_debug(": '%c'", (uint8_t)(word.chr));
      } else if (word.type == l_str) {
        f_debug(": \"%s\"", source->data_buffer + word.str);
      }
      
      f_debug("]\n");
      
      // Check for "only (macro), !(macro), ...;":
      
      if (last_only >= 0) {
        if (word.type == s_comma) {
          only_not = 0;
        } else if (word.type == s_semicolon) {
          source->word_count = last_only;
          done = 0;
          
          last_only = -1;
          only_not = 0;
        } else if (word.type == s_not && !only_not) {
          only_not = 1;
        } else if (word.type == l_name) {
          int valid = 0;
          
          for (int i = 0; i < source->macro_count; i++) {
            if (!strcmp(source->macros[i].name, word.name)) {
              valid = 1;
              break;
            }
          }
          
          if (only_not) {
            valid = !valid;
          }
          
          if (!valid) {
            source->word_count = last_only;
            break;
          }
        } else {
          f_error("Expected identifier or valid symbol, found %s.\n", word_types[word.type]);
        }
      } else if (word.type == k_only) {
        last_only = source->word_count;
      }
      
      // Check for "use (path);":
      
      if (last_use >= 0) {
        if (word.type == l_str) {
          source->word_count = last_use;
          done = 0;
          
          char *path_copy = strdup(source->data_buffer + word.str);
          
          f_source_load(source, path_copy);
          free(path_copy);
          
          last_use = source->word_count;
        } else if (word.type == s_comma) {
          // Actually, just don't do anything here :p
        } else if (word.type == s_semicolon) {
          source->word_count = last_only;
          done = 0;
          
          last_use = -1;
        } else {
          f_error("Expected path or valid symbol, found %s.\n", word_types[word.type]);
        }
      } else if (word.type == k_use) {
        last_use = source->word_count;
      }
      
      // Word storing section (reuse done as an inner flag):
      
      if (done) {
        source->words = realloc(source->words, (source->word_count + 1) * sizeof(word_t));
        source->words[source->word_count++] = word;
        
        done = 0;
      }
      
      word = (word_t){
        .type = w_invalid,
        .file = file_id,
        
        .line = file_line,
        .column = file_column,
      };
    }
    
    if (reuse_chr) {
      reuse_chr = 0;
    } else {
      if (feof(file)) {
        break;
      } else {
        chr = fgetc(file);
        
        if (chr == '\n') {
          file_column = 1;
          file_line++;
        } else {
          file_column++;
        }
      }
    }
    
    if (word.type == w_invalid) {
      if (isalpha(chr) || chr == '_' || chr == '$') {
        word.type = l_name;
        
        temp = 0;
        word.name[temp++] = toupper(chr);
      } else if (isdigit(chr)) {
        word.type = l_l;
        word.l = (chr - '0');
        
        if (chr == '0') {
          word.type = l_ul;
          temp = 8;
        } else {
          temp = 10;
        }
      } else if (chr == '\'') {
        word.type = l_chr;
        word.chr = '\0';
      } else if (chr == '"') {
        word.type = l_str;
        word.str = source->data_length;
      } else if (chr == '(') {
        word.type = s_l_paren;
        done = 1;
      } else if (chr == ')') {
        word.type = s_r_paren;
        done = 1;
      } else if (chr == '[') {
        word.type = s_l_bracket;
        done = 1;
      } else if (chr == ']') {
        word.type = s_r_bracket;
        done = 1;
      } else if (chr == ':') {
        word.type = s_colon;
        done = 1;
      } else if (chr == ';') {
        word.type = s_semicolon;
        done = 1;
      } else if (chr == ',') {
        word.type = s_comma;
        done = 1;
      } else if (chr == '<') {
        word.type = s_l_shift;
        done = 1;
      } else if (chr == '>') {
        word.type = s_r_shift;
        done = 1;
      } else if (chr == '!') {
        word.type = s_not;
        done = 1;
      } else if (chr == '&') {
        word.type = s_and;
        done = 1;
      } else if (chr == '\\') {
        word.type = s_or;
        done = 1;
      } else if (chr == '^') {
        word.type = s_xor;
        done = 1;
      } else if (chr == '+') {
        word.type = s_add;
        done = 1;
      } else if (chr == '-') {
        word.type = s_sub;
        done = 1;
      } else if (chr == '*') {
        word.type = s_mul;
        done = 1;
      } else if (chr == '/') {
        word.type = s_div;
        done = 1;
      } else if (chr == '%') {
        word.type = s_mod;
        done = 1;
      } else if (chr == '=') {
        word.type = s_assign;
        done = 1;
      } else if (chr == '#') {
        word.type = w_comment;
      } else if (chr == '@') {
        word.type = w_at;
      }
    } else if (word.type == w_comment) {
      if (chr == '\n') {
        word.type = w_invalid;
      }
    } else if (word.type == l_name) {
      if (isalnum(chr) || chr == '_' || chr == '$') {
        if (temp >= MAX_NAME_LENGTH) {
          f_source_error("Identifiers can only be %d characters long, found '%c'.\n", MAX_NAME_LENGTH, chr);
        }
        
        word.name[temp++] = toupper(chr);
      } else {
        reuse_chr = 1;
        done = 1;
      }
    } else if (word.type == l_ul || word.type == l_l) {
      const char *ptr = strchr(digits, toupper(chr));
      
      if (ptr) {
        word.l = word.l * temp + (ssize_t)(ptr - digits);
      } else {
        if (!word.l && temp == 8 && toupper(chr) == 'X') {
          temp = 16;
        } else if (toupper(chr) == 'U') {
          word.type = l_ul;
          done = 1;
        } else if (isalnum(chr)) {
          f_source_error("Expected base %ld digit, found '%c'.\n", temp, chr);
        } else {
          reuse_chr = 1;
          done = 1;
        }
      }
    } else if (word.type == l_chr) {
      if (chr == '\\') {
        word.type = w_chr_slash;
        temp = 0;
      } else if (chr == '\'') {
        done = 1;
      } else {
        word.chr = (word.chr << 8) | (size_t)(chr);
      }
    } else if (word.type == l_str) {
      if (chr == '\\') {
        word.type = w_str_slash;
        temp = 0;
        
        source->data_buffer = realloc(source->data_buffer, source->data_length + 1);
        source->data_buffer[source->data_length] = '\0';
      } else if (chr == '"') {
        source->data_buffer = realloc(source->data_buffer, source->data_length + 1);
        source->data_buffer[source->data_length++] = '\0';
        
        done = 1;
      } else {
        source->data_buffer = realloc(source->data_buffer, source->data_length + 1);
        source->data_buffer[source->data_length++] = chr;
      }
    } else if (word.type == w_chr_slash || word.type == w_str_slash) {
      uint8_t last_chr, next_chr;
      int slash_done = 0;
      
      if (word.type == w_chr_slash) {
        last_chr = (uint8_t)(word.chr & 0xFF);
      } else if (word.type == w_str_slash) {
        last_chr = source->data_buffer[source->data_length];
      }
      
      if (temp == 0) {
        if (toupper(chr) == 'B') {
          next_chr = '\b';
          slash_done = 1;
        } else if (toupper(chr) == 'E') {
          next_chr = '\e';
          slash_done = 1;
        } else if (toupper(chr) == 'N') {
          next_chr = '\n';
          slash_done = 1;
        } else if (toupper(chr) == 'R') {
          next_chr = '\r';
          slash_done = 1;
        } else if (toupper(chr) == 'T') {
          next_chr = '\t';
          slash_done = 1;
        } else if (toupper(chr) == 'X') {
          next_chr = '\0';
          temp = 16;
        } else if (chr == '0') {
          next_chr = '\0';
          temp = 8;
        } else if (isdigit(chr)) {
          next_chr = chr - '0';
          temp = 10;
        } else {
          next_chr = chr;
          slash_done = 1;
        }
      } else {
        const char *ptr = strchr(digits, toupper(chr));
        
        if (ptr) {
          last_chr = last_chr * temp + (size_t)(ptr - digits);
          word.chr = (word.chr & ~((size_t)(0xFF))) | last_chr;
        } else {
          slash_done = 1;
          reuse_chr = 1;
        }
      }
      
      if (word.type == w_chr_slash) {
        if (!reuse_chr) {
          word.chr = (word.chr << 8) | (size_t)(next_chr);
        }
        
        if (slash_done) {
          word.type = l_chr;
        }
      } else if (word.type == w_str_slash) {
        if (!reuse_chr) {
          source->data_buffer[source->data_length] = (char)(next_chr);
        }
        
        if (slash_done) {
          word.type = l_str;
          source->data_length++;
        }
      }
    } else if (word.type == w_at) {
      if (chr == '(') {
        word.type = s_a_paren;
        done = 1;
      } else if (chr == '[') {
        word.type = s_a_bracket;
        done = 1;
      } else if (chr == ';') {
        word.type = s_exit;
        done = 1;
      } else if (chr == '<') {
        word.type = s_l_rotate;
        done = 1;
      } else if (chr == '>') {
        word.type = s_r_rotate;
        done = 1;
      } else if (chr == '+') {
        word.type = s_inc;
        done = 1;
      } else if (chr == '-') {
        word.type = s_dec;
        done = 1;
      } else {
        f_source_error("Expected double-char symbol, found '@%c'.\n", chr);
      }
    }
  }
  
  fclose(file);
}
