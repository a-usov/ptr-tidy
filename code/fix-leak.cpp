void foo(int x) {
    std::unique_ptr<Circle> p = std::make_unique<Circle>(Point{0,0},10);
    // ...
    if (x<0) throw Bad_x{}; // delete p inserted
    if (x==0) return; // delete p inserted
    // ...
    // delete p inserted
}