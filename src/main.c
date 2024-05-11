/**
 * Main purpose of this code is just to demonstrate how different variables are
 * loaded and what memory sections are being used.
 */

// .bss (RAM)
static int static_bss_int;
// .data (FLASH -> RAM)
static int static_data_int = 1;

// .bss (RAM)
double bss_double;
// .data (FLASH -> RAM)
double data_double = 1.0;

// .bss (RAM)
struct my_struct {
  int a;
  int b;
} bss_my_struct;

// .bss (RAM)
union my_union {
  short int a;
  int b;
} bss_my_union;

// .text (FLASH)
int main() {

  // ._user_heap_stack (RAM)
  unsigned short int stack_int = 1;

  // stays in .bss
  bss_my_struct.a = 10;
  bss_my_struct.b = 20;

  // stays in .bss
  bss_my_union.b = 30;

  // stays in .bss
  static_bss_int = 1;
  // stays in .bss
  bss_double = static_bss_int + static_data_int + data_double +
                    stack_int + bss_my_struct.a +
                    bss_my_union.b;

  while (1) {
  }

  return 0;
}