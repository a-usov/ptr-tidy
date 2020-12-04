int test(int *b) {
    return 2;
}

int main() {
    int a = 9;
    int b = 2;
    int *c = &b;
    test(&b);
}
