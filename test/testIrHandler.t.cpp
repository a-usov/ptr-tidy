#include "IrHandler.h"
#include "gtest/gtest.h"

TEST(TestIrHandler, testValidInput) {
  llvm::StringRef a = R"(
; ModuleID = 'main.cpp'
source_filename = "main.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind optnone
define i32 @main() #0 {
entry:
  ret i32 0
}

attributes #0 = { noinline norecurse nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+cx8,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
)";

//  IrHandler irHandler{llvm::MemoryBufferRef{a, "main.cpp"}};
//
//  ASSERT_TRUE(irHandler.getModule().has_value());
}

TEST(TestIrHandler, testInvalidInput) {
  llvm::StringRef a = R"(foo)";

//  IrHandler irHandler{llvm::MemoryBufferRef{a, "main.cpp"}};
//
//  ASSERT_FALSE(irHandler.getModule().has_value());
}
