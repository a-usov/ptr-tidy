  void foo(int x) {
    Shape *p = new Circle{Point{0,0},10};
    // ...
    if (x<0) throw Bad_x{}; // potential leak
    if (x==0) return; // potential leak
    // ...
    delete p;
}