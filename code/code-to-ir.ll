; Function Attrs: noinline norecurse nounwind optnone
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 4, i32* %a, align 4
  %0 = load i32, i32* %a, align 4
  ret i32 %0
}