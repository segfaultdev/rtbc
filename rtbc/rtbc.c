#include <stdio.h>
#include <rtbc.h>

int main(void) {
  f_do_debug = 1;
  
  source_t source = (source_t){
    .files = NULL,
    .file_count = 0,
    
    .words = NULL,
    .word_index = 0,
    .word_count = 0,
  };
  
  f_source_load(&source, "../test.tbc");
  
  return 0;
}
