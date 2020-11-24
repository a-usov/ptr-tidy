int a = 0;

int *test(int *b) {
  a = *b;
  return b;
}

int main() {
  int b = 2;
  int *c = &a;
  test(&b);
}