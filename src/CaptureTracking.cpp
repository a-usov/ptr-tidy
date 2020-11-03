//===--- CaptureTracking.cpp - Determine whether a pointer is captured ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains routines that help determine which pointers are captured.
// A pointer value is captured if the function makes a copy of any part of the
// pointer that outlives the call.  Not being captured means, more or less, that
// the pointer is only dereferenced and not stored in a global.  Returning part
// of the pointer as the function return value may or may not count as capturing
// the pointer, depending on the context.
//
//===----------------------------------------------------------------------===//

#include "CaptureTracking.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

/// The default value for MaxUsesToExplore argument. It's relatively small to
/// keep the cost of analysis reasonable for clients like BasicAliasAnalysis,
/// where the results can't be cached.
/// TODO: we should probably introduce a caching CaptureTracking analysis and
/// use it where possible. The caching version can use much higher limit or
/// don't have this cap at all.
static cl::opt<unsigned> DefaultMaxUsesToExplore("capture-tracking-max-uses-to-explore", cl::Hidden,
                                                 cl::desc("Maximal number of uses to explore."),
                                                 cl::init(20));

unsigned llvm::getDefaultMaxUsesToExploreForCaptureTracking() { return DefaultMaxUsesToExplore; }

CaptureTracker::~CaptureTracker() {}

bool CaptureTracker::shouldExplore(const Use *U) { return true; }

bool CaptureTracker::isDereferenceableOrNull(Value *O, const DataLayout &DL) {
  // An inbounds GEP can either be a valid pointer (pointing into
  // or to the end of an allocation), or be null in the default
  // address space. So for an inbounds GEP there is no way to let
  // the pointer escape using clever GEP hacking because doing so
  // would make the pointer point outside of the allocated object
  // and thus make the GEP result a poison value. Similarly, other
  // dereferenceable pointers cannot be manipulated without producing
  // poison.
  if (auto *GEP = dyn_cast<GetElementPtrInst>(O))
    if (GEP->isInBounds())
      return true;
  bool CanBeNull;
  return O->getPointerDereferenceableBytes(DL, CanBeNull);
}

namespace {
struct SimpleCaptureTracker : public CaptureTracker {
  explicit SimpleCaptureTracker(bool ReturnCaptures)
      : ReturnCaptures(ReturnCaptures), Captured(false) {}

  void tooManyUses() override { Captured = true; }

  bool captured(const Use *U) override {
    if (isa<ReturnInst>(U->getUser()) && !ReturnCaptures)
      return false;

    Captured = true;
    return true;
  }

  bool ReturnCaptures;

  bool Captured;
};
} // namespace

/// PointerMayBeCaptured - Return true if this pointer value may be captured
/// by the enclosing function (which is required to exist).  This routine can
/// be expensive, so consider caching the results.  The boolean ReturnCaptures
/// specifies whether returning the value (or part of it) from the function
/// counts as capturing it or not.  The boolean StoreCaptures specified whether
/// storing the value (or part of it) into memory anywhere automatically
/// counts as capturing it or not.
bool llvm::PointerMayBeCaptured(const Value *V, bool ReturnCaptures, bool StoreCaptures,
                                unsigned MaxUsesToExplore) {
  assert(!isa<GlobalValue>(V) && "It doesn't make sense to ask whether a global is captured.");

  // TODO: If StoreCaptures is not true, we could do Fancy analysis
  // to determine whether this store is not actually an escape point.
  // In that case, BasicAliasAnalysis should be updated as well to
  // take advantage of this.
  (void)StoreCaptures;

  SimpleCaptureTracker SCT(ReturnCaptures);
  PointerMayBeCaptured(V, &SCT, MaxUsesToExplore);
  return SCT.Captured;
}

void llvm::PointerMayBeCaptured(const Value *V, CaptureTracker *Tracker,
                                unsigned MaxUsesToExplore) {
  assert(V->getType()->isPointerTy() && "Capture is for pointers only!");
  if (MaxUsesToExplore == 0)
    MaxUsesToExplore = DefaultMaxUsesToExplore;

  SmallVector<const Use *, 20> Worklist;
  Worklist.reserve(getDefaultMaxUsesToExploreForCaptureTracking());
  SmallSet<const Use *, 20> Visited;

  auto AddUses = [&](const Value *V) {
    unsigned Count = 0;
    for (const Use &U : V->uses()) {
      // If there are lots of uses, conservatively say that the value
      // is captured to avoid taking too much compile time.
      if (Count++ >= MaxUsesToExplore)
        return Tracker->tooManyUses();
      if (!Visited.insert(&U).second)
        continue;
      if (!Tracker->shouldExplore(&U))
        continue;
      Worklist.push_back(&U);
    }
  };
  AddUses(V);

  while (!Worklist.empty()) {
    const Use *U = Worklist.pop_back_val();
    Instruction *I = cast<Instruction>(U->getUser());
    V = U->get();

    switch (I->getOpcode()) {
    case Instruction::Call:
    case Instruction::Invoke: {
      auto *Call = cast<CallBase>(I);
      // Not captured if the callee is readonly, doesn't return a copy through
      // its return value and doesn't unwind (a readonly function can leak bits
      // by throwing an exception or not depending on the input value).
      if (Call->onlyReadsMemory() && Call->doesNotThrow() && Call->getType()->isVoidTy())
        break;

      // The pointer is not captured if returned pointer is not captured.
      // NOTE: CaptureTracking users should not assume that only functions
      // marked with nocapture do not capture. This means that places like
      // GetUnderlyingObject in ValueTracking or DecomposeGEPExpression
      // in BasicAA also need to know about this property.
      if (isIntrinsicReturningPointerAliasingArgumentWithoutCapturing(Call, true)) {
        AddUses(Call);
        break;
      }

      // Volatile operations effectively capture the memory location that they
      // load and store to.
      if (auto *MI = dyn_cast<MemIntrinsic>(Call))
        if (MI->isVolatile())
          if (Tracker->captured(U))
            return;

      // Not captured if only passed via 'nocapture' arguments.  Note that
      // calling a function pointer does not in itself cause the pointer to
      // be captured.  This is a subtle point considering that (for example)
      // the callee might return its own address.  It is analogous to saying
      // that loading a value from a pointer does not cause the pointer to be
      // captured, even though the loaded value might be the pointer itself
      // (think of self-referential objects).
      for (auto IdxOpPair : enumerate(Call->data_ops())) {
        int Idx = IdxOpPair.index();
        Value *A = IdxOpPair.value();
        if (A == V && !Call->doesNotCapture(Idx))
          // The parameter is not marked 'nocapture' - captured.
          if (Tracker->captured(U))
            return;
      }
      break;
    }
    case Instruction::Load:
      // Volatile loads make the address observable.
      if (cast<LoadInst>(I)->isVolatile())
        if (Tracker->captured(U))
          return;
      break;
    case Instruction::VAArg:
      // "va-arg" from a pointer does not cause it to be captured.
      break;
    case Instruction::Store:
      // Stored the pointer - conservatively assume it may be captured.
      // Volatile stores make the address observable.
      if (V == I->getOperand(0) || cast<StoreInst>(I)->isVolatile())
        if (Tracker->captured(U))
          return;
      break;
    case Instruction::AtomicRMW: {
      // atomicrmw conceptually includes both a load and store from
      // the same location.
      // As with a store, the location being accessed is not captured,
      // but the value being stored is.
      // Volatile stores make the address observable.
      auto *ARMWI = cast<AtomicRMWInst>(I);
      if (ARMWI->getValOperand() == V || ARMWI->isVolatile())
        if (Tracker->captured(U))
          return;
      break;
    }
    case Instruction::AtomicCmpXchg: {
      // cmpxchg conceptually includes both a load and store from
      // the same location.
      // As with a store, the location being accessed is not captured,
      // but the value being stored is.
      // Volatile stores make the address observable.
      auto *ACXI = cast<AtomicCmpXchgInst>(I);
      if (ACXI->getCompareOperand() == V || ACXI->getNewValOperand() == V || ACXI->isVolatile())
        if (Tracker->captured(U))
          return;
      break;
    }
    case Instruction::BitCast:
    case Instruction::GetElementPtr:
    case Instruction::PHI:
    case Instruction::Select:
    case Instruction::AddrSpaceCast:
      // The original value is not captured via this if the new value isn't.
      AddUses(I);
      break;
    case Instruction::ICmp: {
      unsigned Idx = (I->getOperand(0) == V) ? 0 : 1;
      unsigned OtherIdx = 1 - Idx;
      if (auto *CPN = dyn_cast<ConstantPointerNull>(I->getOperand(OtherIdx))) {
        // Don't count comparisons of a no-alias return value against null as
        // captures. This allows us to ignore comparisons of malloc results
        // with null, for example.
        if (CPN->getType()->getAddressSpace() == 0)
          if (isNoAliasCall(V->stripPointerCasts()))
            break;
        if (!I->getFunction()->nullPointerIsDefined()) {
          auto *O = I->getOperand(Idx)->stripPointerCastsSameRepresentation();
          // Comparing a dereferenceable_or_null pointer against null cannot
          // lead to pointer escapes, because if it is not null it must be a
          // valid (in-bounds) pointer.
          if (Tracker->isDereferenceableOrNull(O, I->getModule()->getDataLayout()))
            break;
        }
      }
      // Comparison against value stored in global variable. Given the pointer
      // does not escape, its value cannot be guessed and stored separately in a
      // global variable.
      auto *LI = dyn_cast<LoadInst>(I->getOperand(OtherIdx));
      if (LI && isa<GlobalVariable>(LI->getPointerOperand()))
        break;
      // Otherwise, be conservative. There are crazy ways to capture pointers
      // using comparisons.
      if (Tracker->captured(U))
        return;
      break;
    }
    default:
      // Something else - be conservative and say it is captured.
      if (Tracker->captured(U))
        return;
      break;
    }
  }

  // All uses examined.
}