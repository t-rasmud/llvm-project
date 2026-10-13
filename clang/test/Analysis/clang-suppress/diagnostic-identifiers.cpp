// RUN: %clang_analyze_cc1 -analyzer-checker=core,debug.ExprInspection -verify %s

void clang_analyzer_warnIfReached();

// Tests for [[clang::suppress]] with diagnostic identifier arguments.

// ============================================================================
// Group A: Bare [[clang::suppress]] vs. with identifier
// ============================================================================

void bare_suppress() {
  [[clang::suppress]] {
    clang_analyzer_warnIfReached(); // no-warning: bare suppress works
  }
}

void suppress_with_identifier() {
  [[clang::suppress("debug.ExprInspection")]] {
    clang_analyzer_warnIfReached(); // no-warning: checker name matches
  }
}

void suppress_with_wrong_identifier() {
  // A non-matching checker name does not suppress the report.
  [[clang::suppress("alpha.SomeOtherChecker")]] {
    clang_analyzer_warnIfReached(); // expected-warning{{REACHABLE}}
  }
}

// ============================================================================
// Group B: Identifier on declarations
// ============================================================================

[[clang::suppress]] void decl_bare_suppress() {
  clang_analyzer_warnIfReached(); // no-warning
}

[[clang::suppress("debug.ExprInspection")]] void decl_with_identifier() {
  clang_analyzer_warnIfReached(); // no-warning
}

// ============================================================================
// Group C: Identifier on class
// ============================================================================

struct [[clang::suppress]] C_BareSuppressedClass {
  void method() {
    clang_analyzer_warnIfReached(); // no-warning
  }
};

// "core" does not name debug.ExprInspection's package, so no suppression.
struct [[clang::suppress("core")]] C_IdentifierSuppressedClass {
  void method() {
    clang_analyzer_warnIfReached(); // expected-warning{{REACHABLE}}
  }
};

// ============================================================================
// Group D: Multiple identifiers
// ============================================================================

void multiple_identifiers() {
  // None of the identifiers match debug.ExprInspection.
  [[clang::suppress("core.NullDereference", "core.DivideZero")]] {
    clang_analyzer_warnIfReached(); // expected-warning{{REACHABLE}}
  }
  // Any one matching identifier is enough.
  [[clang::suppress("core.DivideZero", "debug.ExprInspection")]] {
    clang_analyzer_warnIfReached(); // no-warning
  }
}

// ============================================================================
// Group E: Empty string identifier
// ============================================================================

void empty_string_identifier() {
  // An empty string is still a non-empty identifier list and matches nothing.
  [[clang::suppress("")]] {
    clang_analyzer_warnIfReached(); // expected-warning{{REACHABLE}}
  }
}

// ============================================================================
// Group F: Mixed — bare suppress and identifier suppress in same function
// ============================================================================

void mixed_suppressions() {
  [[clang::suppress]] {
    clang_analyzer_warnIfReached(); // no-warning: bare suppress works
  }

  [[clang::suppress("debug.ExprInspection")]] {
    clang_analyzer_warnIfReached(); // no-warning
  }
}

// ============================================================================
// Group G: Identifier on namespace
// ============================================================================

namespace [[clang::suppress]] G_BareNS {
  void func() {
    clang_analyzer_warnIfReached(); // no-warning
  }
} // namespace G_BareNS

namespace [[clang::suppress("debug.ExprInspection")]] G_IdentifierNS {
  void func() {
    clang_analyzer_warnIfReached(); // no-warning
  }
} // namespace G_IdentifierNS

// ============================================================================
// Group H: Package-prefix matching
// ============================================================================

void package_prefix() {
  // A package name suppresses every checker in that package.
  [[clang::suppress("debug")]] {
    clang_analyzer_warnIfReached(); // no-warning
  }
}

void partial_component_does_not_match() {
  // Matching is on '.'-separated components, not raw string prefixes.
  [[clang::suppress("debug.Expr")]] {
    clang_analyzer_warnIfReached(); // expected-warning{{REACHABLE}}
  }
  [[clang::suppress("deb")]] {
    clang_analyzer_warnIfReached(); // expected-warning{{REACHABLE}}
  }
}

void identifier_longer_than_checker_name() {
  [[clang::suppress("debug.ExprInspection.Extra")]] {
    clang_analyzer_warnIfReached(); // expected-warning{{REACHABLE}}
  }
}

// ============================================================================
// Group I: Real checkers
// ============================================================================

int null_deref_matching() {
  int *p = 0;
  [[clang::suppress("core.NullDereference")]]
  return *p; // no-warning
}

int null_deref_package() {
  int *p = 0;
  [[clang::suppress("core")]]
  return *p; // no-warning
}

int null_deref_other_checker() {
  int *p = 0;
  [[clang::suppress("core.DivideZero")]]
  return *p; // expected-warning{{Dereference of null pointer}}
}

int only_matching_checker_suppressed(int x) {
  int *p = 0;
  [[clang::suppress("core.NullDereference")]] {
    if (x == 0)
      return 1 / x; // expected-warning{{Division by zero}}
  }
  return *p; // expected-warning{{Dereference of null pointer}}
}
