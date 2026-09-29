//===- BugSuppression.h - Suppression interface -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines BugSuppression, a simple interface class encapsulating
//  all user provided in-code suppressions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_STATICANALYZER_CORE_BUGREPORTER_SUPPRESSION_H
#define LLVM_CLANG_STATICANALYZER_CORE_BUGREPORTER_SUPPRESSION_H

#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

namespace clang {
class ASTContext;
class Decl;

namespace ento {
class BugReport;
class PathDiagnosticLocation;

class BugSuppression {
public:
  explicit BugSuppression(const ASTContext &ACtx) : ACtx(ACtx) {}

  /// Return true if the given bug report was explicitly suppressed by the user.
  bool isSuppressed(const BugReport &);

  /// Return true if the bug reported at the given location, by the checker
  /// with the given name, was explicitly suppressed by the user.
  bool isSuppressed(const PathDiagnosticLocation &Location,
                    const Decl *DeclWithIssue, llvm::StringRef CheckerName);

  // A single `[[clang::suppress]]` annotation found in the source, together
  // with the (possibly empty) list of checker names it was restricted to.
  // An empty `CheckerNames` list means "suppress everything in this range",
  // matching the attribute's traditional argument-less behavior.
  //
  // Exposed as an implementation detail so that BugSuppression.cpp's AST
  // visitor can populate the cache below; not meant to be used elsewhere.
  struct Suppression {
    SourceRange Range;
    llvm::SmallVector<llvm::StringRef, 2> CheckerNames;
  };

private:
  // Overly pessimistic number, to be honest.
  static constexpr unsigned EXPECTED_NUMBER_OF_SUPPRESSIONS = 8;
  using CachedSuppressions =
      llvm::SmallVector<Suppression, EXPECTED_NUMBER_OF_SUPPRESSIONS>;

  llvm::DenseMap<const Decl *, CachedSuppressions> CachedSuppressionLocations;

  const ASTContext &ACtx;
};

} // end namespace ento
} // end namespace clang

#endif // LLVM_CLANG_STATICANALYZER_CORE_BUGREPORTER_SUPPRESSION_H
