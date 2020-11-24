bool isOdd(int *i) {
    return (unsigned long) i & 1;
}

int main() {
    int *pointer = new int(2);
    return isOdd(pointer);
}
