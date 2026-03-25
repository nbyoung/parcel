# Parcel

The _Parcel_ encapsulation language brings simple modules, called _parcels_, to C.

## Motivation

C has headers and translation units, but no module system. A header declares an interface, but nothing in the language separates the interface from the implementation, prevents a caller from depending on internal identifiers, or allows two files to provide alternative implementations of the same abstraction. The result is that large C programs tend toward tight coupling: callers depend on concrete implementations, and substituting behaviour requires changing call sites.

Parcel adds a lightweight module layer on top of standard C. A _parcel_ is a named set of identifiers — typedefs, variables, or functions — declared in one file and made available in others. A file that declares an interface parcel containing only types can be imported by any number of implementation files, each of which exports a conforming parcel under its own name. A consumer imports whichever implementation it needs. The types are consistent across all implementations, and call sites do not change when implementations are substituted.

Parcel operates at the source level, before the preprocessor. It requires no changes to the compiler toolchain.

See the [_Parcelator_](https://github.com/nbyoung/parcelator) project for a translator implementation.

## Concepts

### Declaring a parcel

A parcel is declared with a `#pragma` statement that names the parcel and lists the local identifiers it exports:

```c
#pragma parcel <name> { [<identifier> ...] }
```

The interface lists the typedefs, variables, and functions defined in the same file that the parcel makes available to importers.

By convention, the name `_` denotes the **default parcel** for a file — typically an interface or utility parcel that other files import to obtain shared types. Named parcels (any name other than `_`) are typically implementations or components.

### Exporting a parcel

A parcel is exported from the same file in which it is declared, by including a generated export file:

```c
#include "export/<path>/<name>"
```

The export path places the parcel in a programmer-defined namespace. `export` is almost always the literal base segment; `<path>` reflects the filesystem location; `<name>` matches the parcel name from the `#pragma`.

The export statement must appear in the same file as the corresponding `#pragma parcel` declaration.

### Importing a parcel

A parcel is imported into a different file by including a generated import file, supplying a _stem_:

```c
#include "import/<path>/<name>.<stem>"
```

The path and name identify which exported parcel to import. The stem is a short mnemonic identifier that scopes the imported identifiers in the local file, preventing name collisions when multiple parcels are in use.

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

**Functions** use a direct struct-pointer call:

```c
stem->identifier(args)
```

For example, a function `output` imported with stem `std` is called as `std->output(greeting)`.

This distinction means that types read naturally in declarations, while variables and functions carry an explicit access path that makes the origin of each identifier visible at the call site.

## Syntax reference

| Statement | |
|--|--|
| Declare | `#pragma parcel <name> { [<identifier> ...] }` |
| Export | `#include "export/<path>/<name>"` |
| Import | `#include "import/<path>/<name>.<stem>"` |

| Term | Meaning |
|--|--|
| `<name>` | Any valid C identifier; `_` by convention for a default (interface) parcel |
| `{ [<identifier> ...] }` | The parcel interface: typedefs, variables, and functions defined in the local file |
| `<path>` | Namespace path, typically reflecting the filesystem location of the parcel |
| `<stem>` | A short C identifier fragment scoping imported identifiers in the local file |

The relationship between export path and import path is not specified by the Parcel language; it is defined by the translator implementation. In practice, paths typically reflect the filesystem structure of the program. See the [examples](examples/) for illustration.

## Example

The [`examples/hello_world`](examples/hello_world/) example demonstrates modular abstraction using an interface parcel and two interchangeable implementations.

![UML class diagram of the hello_world example](examples/hello_world/hello_world.svg)

`output.c` declares a **default parcel** (`_`) containing only types — a `Greeting` typedef and an `Output` function-pointer typedef. This is the interface:

```c
#pragma parcel _ { Greeting Output }

typedef char *Greeting;
typedef void (*Output)(Greeting greeting);

#include "export/output/_"
```

`output/stdout.c` imports the interface, defines a conforming implementation, and exports it as the named parcel `stdout`:

```c
#include "import/output/_.out"

#pragma parcel stdout { output }

void output(out_Greeting greeting) { ... }

#include "export/output/stdout"
```

`output/null.c` exports an identically shaped parcel with a body that does nothing — a valid substitute that requires no changes at call sites.

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

Passing `-DOUTPUT_NULL` selects the null implementation; omitting it selects stdout. `main.c` is unchanged in either case. `OUT->output` uses the variable/function stem pointer form; the `out_Greeting` type, being a typedef, uses the prefix form throughout.
