# Parcel Translation

Parcel translation generates the `export/` and `import/` _include_ files that express the modular semantics in standard C.

Translation applies three phases. The _extraction_ phase scans the original Parcel source files and extracts the parcel interface _declarations_, as well as each declaration's associated _export_ statement, and any parcel _import_ statements. The _analysis_ phase analyses the parcel declarations within the context of their enclosing C source code, thereby determining the parcel interface identifier definitions, as well as the parcel export-import relationships. If the analysis determines that the declared parcels and their relationships are correct and complete, then the _generation_ phase generates their modular semantic representations in ordinary C files that the original source files `#include` through their respective export and import statements.

 The original Parcel source files, augmented with the translator-generated `#include` files, comprise a complete C program that can be compiled in the usual way using a conventional compiler toolchain.

<figure>
  <img src="PARCEL_TRANSLATION.svg" alt="Parcel Translation">
</figure>

## Diagnostics

The translator validates parcel declarations across all scanned source files and emits diagnostics at two severity levels.

### Errors

Error messages indicate input conditions from which the translator cannot generate correct output. Consequently, the affected `export/` and `import/` files are not generated.

**Undefined identifier.** An identifier listed under a kind label in a `#pragma parcel` block has no corresponding C definition in the same translation unit, i.e., file.

**Duplicate interface identifier.** The same identifier is listed more than once for the same parcel.

**Label mismatch.** An identifier listed under a `#pragma <kind>:` label has a C declaration whose kind conflicts with the stated kind. For example, listing a typedef identifier under `function:`, or a function identifier under `constant:`.

**Unexported parcel.** A `#pragma parcel <name>` block is opened but no corresponding `#include "export/.../<name>"` appears in the file. The parcel is declared but never exported.

**Redeclared parcel.** A second or subsequent `#pragma parcel <name>` declaration appears for the same parcel `<name>` within the same source file.  

**Orphan export.** A file contains `#include "export/<path>/<name>"` but no open `#pragma parcel <name>` block. The translator has no interface from which to generate the export file.

**Undefined export.** A file contains `#include "import/<path>/<name>.<stem>"` but with no corresponding `#include "export/..."` statement for that parcel name. The translator has no interface from which to derive the _import_ file.

**Path-name collision.** Two distinct `<path>/<name>` pairs produce the same canonical prefix after replacing `/` with `_`. For example, path `foo_bar` with name `p` and path `foo` with name `bar_p` both yield the prefix `foo_bar_p_`. Any identifiers exported by either parcel would receive identical canonical names, making the generated files mutually incompatible. This collision cannot be resolved at the identifier level; it is an error regardless of whether the affected parcels share any identifier names. It must be resolved by renaming one of the paths or parcel names.

**Stem collision.** Two `#include "import/..."` directives in the same file apply the same stem. Identifiers from both parcels would be emitted under the same prefix, causing unintended typedef redefinition, i.e., _shadowing_.

### Warnings

Warning messages indicate parcel declarations that do not prevent generation but are likely to cause compile-time failures or unintended behaviour.

**Exported static identifier.** An identifier listed in a `#pragma parcel` declaration is declared `static` in the same file. Static identifiers have internal linkage and are not accessible to importers; the generated export file will compile but the exported name will be unusable in other translation units.

**Canonical name conflict.** A user-defined identifier in the file has the same spelling as a canonical name the translator is about to generate (for example, a local definition of `foo_p_T` in `foo.c`). The generated typedef will produce a redefinition error at compile time.

**Unused import.** A parcel import applies `<stem>` but no corresponding `<stem>`-qualified identifiers appearing in the C code.
