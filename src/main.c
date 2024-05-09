static int static_declared_int;
static int static_defined_int = 1;

double declared_double;
double defined_double = 1.0;

static const int const_defined_int = 42;

typedef unsigned short int my_uint16;

struct my_struct {
  int a;
  int b;
} declared_my_struct;

union my_union {
  short int a;
  int b;
} declared_my_union;

int main() {

  my_uint16 local_defined_uint16 = 1;

  declared_my_struct.a = 10;
  declared_my_struct.b = 20;

  declared_my_union.b = 30;

  static_declared_int = 1;
  declared_double = static_declared_int + static_defined_int + defined_double +
                    local_defined_uint16 + declared_my_struct.a +
                    declared_my_union.b + const_defined_int;

  while (1) {
  }

  return 0;
}