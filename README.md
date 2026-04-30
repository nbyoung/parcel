# Parcel

The _Parcel_ encapsulation language brings simple modules, called _parcels_, to C.

* [Motivation](#motivation)
* [Concepts](#concepts)
* [Syntax](#syntax-reference)
* [Example](#example)

Follow these links for more details.

* [Language semantics](SEMANTICS.md)
* ["Hello, world!" example](examples/hello_world/README.md)
* [Parcel translation](TRANSLATION.md)
* [_Parcelator_ translator](https://github.com/nbyoung/parcelator)

## Motivation

C has headers and translation units, but no modular encapsulation. Header files can be used as a substitute for module interfaces, but the language generally fails to:
 
* Enforce separation of the interface from its implementations
* Prevent a module caller from depending on internal details
* Allow multiple alternative implementations of the same interface

Even with disciplined use of header files, large C programs tend toward tight coupling. Programmers come to depend on implementation details. Substituting behaviour requires changing the call sites.

Parcel adds a lightweight module layer on top of standard C. A _parcel_ is a named set of identifiers — typedefs, constants, variables, and functions — declared in one file and made available in others. A file that declares an interface parcel containing only types can be imported by any number of implementation files, each of which exports a conforming parcel under its own name. A module consumer imports whichever implementation it needs. The types are consistent across all implementations, and call sites do not change when substituting alternative implementations.

The Parcel language applies at the source level, before the preprocessor. It requires no changes to the compiler toolchain.

See the [_Parcelator_](https://github.com/nbyoung/parcelator) project for a translator implementation.

## Concepts

### Declaring and exporting a parcel

A parcel is declared as a block that opens with `#pragma parcel <name>`, affirms each exported identifier's kind on a labeled line, and closes with the export `#include`:

```c
#pragma  parcel <name>
#pragma      typedef: t1, t2, ...
#pragma      constant: c1, c2, ...
#pragma      variable: v1, v2, ...
#pragma      function: f1, f2, ...
#include "export/<path>/<name>"
```

The kind labels — `typedef:`, `constant:`, `variable:`, `function:` — affirm the kind of each exported identifier as defined in the same file. Only the kinds present in the parcel are required; unused labels may be omitted.

The export `#include` closes the block and triggers generation. It must appear in the same file after all of the exported identifier definitions.

The export path places the parcel in a programmer-defined namespace. The string `export` defines the literal base segment of the `#include` file path; `<path>` reflects the specific filesystem location; `<name>` matches the parcel name from the opening `#pragma parcel` line.

By convention, the name `_` denotes the **default parcel** for a file — typically an interface or utility parcel that other files import to obtain shared types. Named parcels (any name other than `_`) are typically implementations or components.

### Importing a parcel

A parcel is imported into a different file by including a generated import file while supplying a _stem_ string:

```c
#include "import/<path>/<name>.<stem>"
```

The path and name identify which exported parcel to import. The stem is a short mnemonic identifier that scopes the imported identifiers in the importing file, thereby preventing name collisions when multiple parcels are in use.

### Stem application

The stem is applied differently depending on the kind of identifier being imported:

**Typedefs** use a simple underscore-separated prefix:

```c
stem_Identifier
```

For example, a typedef `Greeting` imported with stem `out` becomes `out_Greeting`.

**Variables** use a dereferenced struct-pointer:

```c
*stem->identifier         /* read */
*stem->identifier = value /* write */
```

**Functions** use a direct struct-pointer expression:

```c
stem->identifier(args)
```

For example, a function `output` imported with stem `std` is called as `std->output(greeting)`.

## Syntax reference

| Statement | |
|--|--|
| Open declaration | `#pragma parcel <name>` |
| Kind label | `#pragma typedef:` / `constant:` / `variable:` / `function:` `<id>, ...` |
| Export (closes declaration) | `#include "export/<path>/<name>"` |
| Import | `#include "import/<path>/<name>.<stem>"` |

| Term | Meaning |
|--|--|
| `<name>` | Any valid C identifier; By convention, `_` denotes the exporting file's _default_ parcel |
| `typedef:` / `constant:` / `variable:` / `function:` | Kind label affirming the declared kind of each exported identifier |
| `<id>, ...` | Comma-separated identifiers of the declared kind |
| `<path>` | Namespace path, typically reflecting the filesystem location of the parcel |
| `<stem>` | A short C identifier fragment that scopes the imported identifiers within the importing file |

The relationship between export path and import path is not specified by the Parcel language. Strictly spesking, it is defined by the translator implementation. In practice, paths typically reflect the filesystem structure of the program. See the [examples](examples/) for illustration.

## Example

The [`examples/hello_world`](examples/hello_world/) example demonstrates modular abstraction using an interface parcel and two interchangeable implementations, `stdout` and `null`.

`output.c` declares a **default parcel** (`_`) containing only types — a `Greeting` typedef and an `Output` function-pointer typedef. This is the _output_ module interface:

```c
typedef char *Greeting;
typedef void (*Output)(Greeting greeting);

#pragma  parcel _
#pragma      typedef: Greeting
#pragma      function: Output
#include "export/output/_"
```

`output/stdout.c` imports the interface, defines a conforming implementation, and exports it as the named parcel `stdout`:

```c
#include "import/output/_.out"

void output(out_Greeting greeting) { ... }

#pragma  parcel stdout
#pragma      function: output
#include "export/output/stdout"
```

`output/null.c` exports an identically shaped parcel with a body that does nothing — a valid substitute that requires no changes at the module call sites.

`output.h` is a selector header that conditionally imports one implementation based on a compiler flag, and defines `OUT` to match its stem:

```c
#include "import/output/_.out"

#if defined(OUTPUT_NULL)
#include "import/output/null.null"
#define OUT null
#else
#include "import/output/stdout.std"
#define OUT std
#endif
```

`main.c` includes the selector and calls through `OUT`, naming neither the implementation nor its stem directly:

```c
#include "output.h"

OUT->output(greeting);   // prints, or is silent — determined at compile time
```

Passing `-DOUTPUT_NULL` selects the `null` implementation; omitting it selects `stdout`. `main.c` is unchanged in either case. `OUT->output` uses the variable/function stem pointer form; the `out_Greeting` type, being a typedef, uses the prefix form throughout.

```sh
$ cd examples/hello_world
$ make
stdout implementation:
Hello, world!
null implementation (output is suppressed):
(No output — Correct!)
```
