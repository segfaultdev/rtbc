#include <stdio.h>
#include <rtbc.h>

extern const arch_t arch_x86;

int main(void) {
  // f_do_debug = 1;
  
  source_t source = (source_t){
    .files = NULL,
    .file_count = 0,
    
    .words = NULL,
    .word_index = 0,
    .word_count = 0,
    
    .macros = NULL,
    .macro_count = 0,
    
    .data_buffer = NULL,
    .data_length = 0,
  };
  
  f_source_load(&source, "test.tbc");
  
  f_parse_root(&arch_x86, &source);
  
  /*
  arch->f_label("MAIN");
  arch->f_push_label("DATA", 0);
  arch->f_call_label("PUTS");
  arch->f_pull(arch->point_width);
  arch->f_exit();
  */
  
  return 0;
}
