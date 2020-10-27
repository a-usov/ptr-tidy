; ModuleID = 'ptr.cpp'
source_filename = "ptr.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind optnone
define i32 @main() #0 {
entry:
  %a = alloca i32, align 4
  %b = alloca i32*, align 8
  %c = alloca i32, align 4
  store i32 2, i32* %a, align 4
  store i32* %a, i32** %b, align 8
  %0 = load i32*, i32** %b, align 8
  %1 = load i32, i32* %0, align 4
  %2 = load i32*, i32** %b, align 8
  %3 = load i32, i32* %2, align 4
  %add = add nsw i32 %1, %3
  store i32 %add, i32* %c, align 4
  ret i32 0
}

; Function Attrs: noinline nounwind optnone
define i32* @_Z3asdv() #1 {
entry:
  %foo = alloca i32, align 4
  %c = alloca i32, align 4
  %b = alloca i32*, align 8
  store i32 4, i32* %c, align 4
  %0 = load i32, i32* %c, align 4
  %add = add nsw i32 2, %0
  store i32 %add, i32* %foo, align 4
  store i32* %foo, i32** %b, align 8
  %1 = load i32*, i32** %b, align 8
  ret i32* %1
}

attributes #0 = { noinline norecurse nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+cx8,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+cx8,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 11.0.0 (https://github.com/llvm/llvm-project.git 176249bd6732a8044d457092ed932768724a6f06)"}
