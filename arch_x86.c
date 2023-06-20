#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <rtbc.h>

static void f_init(void);

static void f_global(const char *name);
static void f_const(const_t value);
static void f_data(const void *data, int length);

static void f_init_routine(int offset);
static void f_exit_routine(void);

static void f_load_const(const_t value);
static void f_load_local(int width, int offset);
static void f_push(int width);
static void f_pull(int width);
static void f_call(int offset);

static void f_zero_extend(int new_width, int old_width);
static void f_sign_extend(int new_width, int old_width);

static int  f_next(void);
static void f_label(int label);

static void f_jump(int label);
static void f_jump_z(int width, int label);
static void f_jump_nz(int width, int label);
static void f_jump_p(int width, int label);
static void f_jump_np(int width, int label);

static int label_count = 0;

const arch_t arch_x86 = (arch_t){
  .name = "x86",
  
  .data_width = 4,
  .point_width = 4,
  
  .is_big = 0,
  
  f_init,
  
  f_global,
  f_const,
  f_data,
  
  f_init_routine,
  f_exit_routine,
  
  f_load_const,
  f_load_local,
  f_push,
  f_pull,
  f_call,
  
  f_zero_extend,
  f_sign_extend,
  
  f_next,
  f_label,
  
  f_jump,
  f_jump_z,
  f_jump_nz,
  f_jump_p,
  f_jump_np,
};

void f_init(void) {
  printf("[bits 32]\n");
}

static int f_next(void) {
  return label_count++;
}

static void f_global(const char *name) {
  printf("\nglobal %s\n\n", name);
  printf("%s:\n", name);
}

static void f_const(const_t value) {
  if (value.is_data) {
    printf("  dd (DATA + %d)\n", value.offset);
  } else {
    int width = value.type.base_width;
    
    if (value.type.point_count) {
      width = 4;
    }
    
    if (width == 1) {
      printf("  db 0x%02X\n", value.ux & 0xFF);
    } else if (width <= 2) {
      printf("  dw 0x%04X\n", value.ux & 0xFFFF);
    } else if (width <= 4) {
      printf("  dd 0x%08X\n", value.ux & 0xFFFFFFFF);
    } else if (width <= 8) {
      printf("  dq 0x%016X\n", value.ux);
    }
  }
}

static void f_data(const void *data, int length) {
  const uint8_t *data_u8 = (const uint8_t *)(data);
  printf("  db ");
  
  for (int i = 0; i < length; i++) {
    if (i) {
      printf(", ");
    }
    
    printf("0x%02X", data_u8[i]);
  }
  
  putchar('\n');
}

static void f_init_routine(int offset) {
  printf("  mov ebp, esp\n");
  
  if (offset) {
    printf("  sub esp, %d\n", offset);
  }
}

static void f_exit_routine(void) {
  printf("  mov esp, ebp\n");
  printf("  ret\n");
}

static void f_load_const(const_t value) {
  if (value.is_data) {
    printf("  mov eax, (DATA + %d)\n", value.offset);
  } else {
    int width = value.type.base_width;
    
    if (value.type.point_count) {
      width = 4;
    }
    
    if (width == 1) {
      printf("  mov al, 0x%02X\n", value.ux & 0xFF);
    } else if (width <= 2) {
      printf("  mov ax, 0x%04X\n", value.ux & 0xFFFF);
    } else if (width <= 4) {
      printf("  mov eax, 0x%08X\n", value.ux & 0xFFFFFFFF);
    } else if (width <= 8) {
      printf("  mov edx, 0x%08X\n", (value.ux >> 32) & 0xFFFFFFFF);
      printf("  mov eax, 0x%08X\n", value.ux & 0xFFFFFFFF);
    }
  }
}

static void f_load_local(int width, int offset) {
  width = (width + 3) / 4;
  printf("  mov eax, [ebp + %d]\n", offset);
  
  if (width > 1) {
    printf("  mov edx, [ebp + %d]\n", offset + 4);
  }
}

static void f_push(int width) {
  width = (width + 3) / 4;
  
  if (width > 1) {
    printf("  push edx\n");
  }
  
  printf("  push eax\n");
}

static void f_pull(int width) {
  width = (width + 3) / 4;
  printf("  pop eax\n");
  
  if (width > 1) {
    printf("  pop edx\n");
  }
}

static void f_call(int offset) {
  printf("  push ebp\n");
  printf("  call eax\n");
  printf("  pop ebp\n");
  printf("  add esp, %d\n", offset);
}

static void f_zero_extend(int new_width, int old_width) {
  const char *names[] = {"al", "ax", "eax", "eax"};
  
  if (new_width < old_width) {
    return;
  }
  
  if (new_width > 4 && old_width <= 4) {
    printf("  xor edx, edx\n");
    printf("  movzx eax, %s\n", names[old_width - 1]);
  } else {
    printf("  movzx %s, %s\n", names[new_width - 1], names[old_width - 1]);
  }
}

static void f_sign_extend(int new_width, int old_width) {
  const char *names[] = {"al", "ax", "eax", "eax"};
  
  if (new_width < old_width) {
    return;
  }
  
  if (new_width > 4 && old_width <= 4) {
    printf("  mov edx, 0xFFFFFFFF\n");
    printf("  movsx eax, %s\n", names[old_width - 1]);
    printf("  cmp eax, 0x80000000\n");
    printf("  adc edx, 0\n");
  } else {
    printf("  movsx %s, %s\n", names[new_width - 1], names[old_width - 1]);
  }
}

static void f_label(int label) {
  printf("SUB_%d:\n", label);
}

static void f_jump(int label) {
  printf("  jmp SUB_%d\n", label);
}

static void f_jump_z(int width, int label) {
  width = (width + 3) / 4;
  
  if (width > 1) {
    int skip_label = f_next();
    
    printf("  test edx, edx\n");
    printf("  jnz SUB_%d\n", skip_label);
    
    f_jump_z(4, label);
    f_label(skip_label);
    
    return;
  }
  
  printf("  test eax, eax\n");
  printf("  jz SUB_%d\n", label);
}

static void f_jump_nz(int width, int label) {
  width = (width + 3) / 4;
  
  if (width > 1) {
    int skip_label = f_next();
    
    printf("  test edx, edx\n");
    printf("  jz SUB_%d\n", skip_label);
    
    f_jump_nz(4, label);
    f_label(skip_label);
    
    return;
  }
  
  printf("  test eax, eax\n");
  printf("  jnz SUB_%d\n", label);
}

static void f_jump_p(int width, int label) {
  width = (width + 3) / 4;
  
  if (width > 1) {
    int skip_label = f_next();
    
    printf("  cmp edx, 0\n");
    printf("  jl SUB_%d\n", skip_label);
    
    f_jump_p(4, label);
    f_label(skip_label);
    
    return;
  }
  
  printf("  cmp eax, 0\n");
  printf("  jge SUB_%d\n", label);
}

static void f_jump_np(int width, int label) {
  width = (width + 3) / 4;
  
  if (width > 1) {
    int skip_label = f_next();
    
    printf("  cmp edx, 0\n");
    printf("  jge SUB_%d\n", skip_label);
    
    f_jump_np(4, label);
    f_label(skip_label);
    
    return;
  }
  
  printf("  cmp eax, 0\n");
  printf("  jl SUB_%d\n", label);
}
