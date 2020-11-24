int *globalPointer;

void escape() {
    int *pointer = new int(0);
    globalPointer = pointer;
    // ...
}

int main() {
    // ...
    escape();
    // ...
}
