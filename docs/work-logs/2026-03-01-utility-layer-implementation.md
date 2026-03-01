# Utility Layer Implementation: TrussUtils Library and Cross-Layer Refactoring

**Project:** 2D Truss Analysis C++ v3.0.0  
**Date:** March 1, 2026  
**Status:** ✅ Complete  
**Scope:** Introduction of a dependency-free utility library, elimination of duplicated math and string logic across the infrastructure and core layers, and centralisation of support type serialization

---

## Context

Mathematical comparison predicates and angle conversion functions were defined in `core/model/types.hpp` under the internal `Utils` namespace, coupling lower-level utility logic to the core domain header. Across the infrastructure layer, six exporters each maintained independent, non-shared implementations of numeric formatting and format-specific string escaping. Four I/O files duplicated equivalent `parseSupportType` and `supportTypeToString` implementations. This work introduces a dedicated `TrussUtils` static library, migrates all duplicated logic into it, and removes the misplaced `core::Utils` namespace, establishing a clean dependency boundary that any layer can consume without introducing coupling.

---

## Scope of Work

**Utility Library**

- Introduced `src/utilities/math_utils.hpp` as a header-only, dependency-free module exposing floating-point comparison predicates and angle conversion functions in the `truss::utils::math` namespace
- Introduced `src/utilities/string_utils.hpp` and `string_utils.cpp` exposing numeric formatting, timestamp generation, case conversion, and format-specific escaping (`escapeJson`, `escapeXml`, `escapeHtml`, `escapeLatex`) in the `truss::utils::string` namespace

**Build**

- Added `TrussUtils` as a CMake `STATIC` library target, linked publicly to `TrussCore`
- Registered `support_type_serializer.cpp` as a `TrussCore` source
- Registered unit test sources for the utility and serializer modules in the `unit_tests` CMake target

**Refactoring**

- Removed the `core::Utils` namespace from `types.hpp`; updated `member.cpp`, `load.cpp`, and `node.cpp` to reference `truss::utils::math`
- Eliminated `formatNumber`, `formatTimestamp`, and format-specific escape implementations from all six exporter translation units; replaced call sites with the corresponding `truss::utils::string` functions
- Centralised `parseSupportType` and `supportTypeToString` into `src/infrastructure/io/support_type_serializer.hpp/.cpp`; removed the six duplicated implementations from the JSON and XML reader and writer translation units

**Testing**

- Added unit tests for `truss::utils::math` (16 tests), `truss::utils::string` (38 tests), and the support type serializer (18 tests)

**Documentation and Style**

- Standardised file-level Doxygen metadata across all new and modified files
- Applied clang-format to all modified source and header files

---

## Technical Changes

`math_utils.hpp` is intentionally header-only and includes only standard library headers, ensuring it imposes no transitive dependencies on any consuming layer. All functions are marked `[[nodiscard]]` and `noexcept` where applicable. The `DEFAULT_TOLERANCE` constant (`1e-12`) is exposed as `inline constexpr` to allow call sites to reference it by name.

`string_utils` separates formatting concerns (`formatReal`, `formatTimestamp`) from escaping concerns (`escapeJson`, `escapeXml`, `escapeHtml`, `escapeLatex`). The `formatReal` function subsumes the per-exporter `formatNumber` logic, accepting precision and a scientific-notation flag to replicate the previous `ExportOptions`-coupled behaviour without depending on any project type.

`support_type_serializer` is placed in the `truss::infrastructure::io` namespace rather than the utility layer because its error path depends on `ParseException`, which is defined in `io_types.hpp`. This preserves the constraint that utility modules carry no project-level dependencies while correctly scoping the serialization contract to the I/O layer. The returned string from `supportTypeToString` is guaranteed to round-trip through `parseSupportType`.

In `exporter_factory.cpp`, the previous `std::transform` / `std::tolower` call was replaced with `truss::utils::string::toLower`, removing the direct `<algorithm>` and `<cctype>` dependencies from that translation unit.

---

## Testing and Validation

Unit tests cover all public functions of `truss::utils::math` and `truss::utils::string`, including boundary conditions for the zero-tolerance predicate, precision and notation variants for `formatReal`, and all recognised escape sequences for each output format. Support type serializer tests cover all valid token variants, case variants accepted for backwards compatibility, round-trip correctness, and `ParseException` propagation for invalid input. All 72 new tests pass. The full test suite of 884 tests passes with no regressions. Build is stable with no new compiler errors or warnings.

---

## Outcome

The `TrussUtils` library provides a single, tested location for cross-cutting mathematical and string utilities that any architectural layer can consume without coupling constraints. The `core::Utils` namespace has been fully removed from the core domain header. Approximately 120 lines of duplicated exporter logic and 40 lines of duplicated I/O serialization logic have been eliminated. No changes were made to any public interface or observable behaviour.
