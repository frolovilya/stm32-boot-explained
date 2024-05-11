/**
 * Main purpose of this code is just to demonstrate how different variables are
 * loaded and what memory sections are being used.
 */

// .bss (RAM)
static int static_declared_int;
// .data (FLASH -> RAM)
static int static_defined_int = 1;

// .bss (RAM)
double declared_double;
// .data (FLASH -> RAM)
double defined_double = 1.0;

// .bss (RAM)
struct my_struct {
  int a;
  int b;
} declared_my_struct;

// .bss (RAM)
union my_union {
  short int a;
  int b;
} declared_my_union;

// .text (FLASH)
int main() {

  // ._user_heap_stack (RAM)
  unsigned short int local_defined_int = 1;

  // stays in .bss
  declared_my_struct.a = 10;
  declared_my_struct.b = 20;

  // stays in .bss
  declared_my_union.b = 30;

  // stays in .bss
  static_declared_int = 1;
  // stays in .bss
  declared_double = static_declared_int + static_defined_int + defined_double +
                    local_defined_int + declared_my_struct.a +
                    declared_my_union.b;

  while (1) {
  }

  return 0;
}