int main() {
  int a = 2;
  int d = 4;
  int *b = &a;
  int *e= &d;
  int c = *b + *e;
}

int *asd() {
  int foo;
  int bar = 4;
  foo = 2 + bar;
  int *foobar = &foo;
  return foobar;
}

int *test2() {
  int* random = new int;
  *random = 2;
  return random;
}
