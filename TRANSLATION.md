# Parcel Translation

Parcel translation generates the `export/` and `import/` header files that express the modular semantics in standard C (see [SEMANTICS.md](SEMANTICS.md)). The translator runs as a pre-preprocessor step that scans the C source files for `#pragma parcel` declarations and parcel `#include` directives, and generates the corresponding files. The standard C preprocessor processes those generated files in the usual build step, leaving the compiler toolchain otherwise unchanged.

## Diagnostics

The translator validates parcel declarations across all scanned source files and emits diagnostics at two severity levels.

### Errors

Errors indicate conditions from which the translator cannot generate correct output. Affected `export/` and `import/` files must not be emitted.

**Undefined exported identifier.** An identifier listed in a `#pragma parcel` declaration has no corresponding definition in the same file. The export file cannot be generated because the canonical typedef would reference an undeclared name.

**Orphan export include.** A file contains `#include "export/<path>/<name>"` but no `#pragma parcel <name>` declaration. The translator has no interface from which to generate the export file.

**Import missing export.** A file contains `#include "import/<path>/<name>.<stem>"` but with no corresponding `#include "export/..."` statement for that name. The translator has no interface from which to derive the import file contents.

**Canonical name collision.** Two distinct `<path>/<name>` pairs produce the same canonical prefix after replacing `/` with `_`. For example, path `foo_bar` with name `p` and path `foo` with name `bar_p` both yield the prefix `foo_bar_p_`. Any identifiers exported by either parcel would receive identical canonical names, making the generated files mutually incompatible. This collision cannot be resolved at the identifier level; it is an error regardless of whether the affected parcels share any identifier names. It must be resolved by renaming one of the paths or parcel names.

**Stem collision.** Two `#include "import/..."` directives in the same file specify the same stem. Identifiers from both parcels would be emitted under the same prefix, causing typedef redefinition or shadowing.

### Warnings

Warnings indicate suspicious constructs that do not prevent generation but are likely to cause compile-time failures or incorrect behaviour.

**Parcel declaration without export.** A `#pragma parcel` declaration has no `#include "export/..."` following it in the file. The parcel is declared but never exported; this is unusual outside of interface-only or draft files.

**Exported static identifier.** An identifier listed in a `#pragma parcel` declaration is declared `static` in the same file. Static identifiers have internal linkage and are not accessible to importers; the generated export file will compile but the exported name will be unusable in other translation units.

**Canonical name conflict.** A user-defined identifier in the file has the same spelling as a canonical name the translator is about to generate (for example, a local definition of `foo_p_T` in `foo.c`). The generated typedef will produce a redefinition error at compile time.

**Duplicate interface identifier.** The same identifier is listed more than once in the same parcel.

**Unused import.** A parcel import include is present but no stemmed identifier derived from it appears in the file.

## Notes

**Cumulative parcel identifiers.** Multiple parcel declarations with the same parcel name are equivalent to a single parcel declaration (at the final parcel position) with all identifiers listed.
