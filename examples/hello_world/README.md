# Hello, World!

This example shows how the Parcel language supports **modular abstraction** and **software re-use** through a small program that prints `Hello, world!` — or deliberately does not. After applying a _Parcel_ translator (not shown here, see the [_Parce_](https://github.com/nbyoung/parce) project), selecting between the default `stdout` implementation and the `null` implementation of the same `output` module requires no changes to the source code, — only applying a command line option, `-DOUT_NULL`.

```c
#include "output.h"

out_Greeting greeting = "Hello, world!";

int main() {
    OUT->output(greeting);
    return 0;
}
```

```sh
$ gcc output.c output/stdout.c main.c -o hello && ./hello
Hello, world!
```

```sh
$ gcc output.c output/null.c main.c -DOUT_NULL -o hello && ./hello
$ # No output!
```

## Motivation

A recurring problem in C is that a module's interface and its implementation are inseparable: a header exposes types and declarations, but there is no language mechanism to say _this set of identifiers is the interface_ and _these files are alternative implementations of it_. The result is that callers depend on concrete implementations rather than abstractions, making it hard to substitute behaviour (e.g. for testing, platform variation, or feature selection) without changing call sites.

Parcel addresses this directly. An interface is expressed as a parcel whose identifiers name the types and behaviours that implementations must provide. Any number of implementation files can import that interface and export a conforming parcel. Call sites import whichever implementation they need, and the types remain consistent across all of them.

## Files

```
output.c            ← interface: defines Greeting and Output types
output/
  stdout.c          ← implementation: prints to standard output
  null.c            ← implementation: discards output silently
output.h            ← selector: conditionally imports one implementation
main.c              ← consumer: calls the selected implementation
```

## Module structure

![UML class diagram of the hello_world parcel structure](hello_world.svg)

The interface parcel (`output/_`) defines only types. Both implementation parcels (`output/stdout` and `output/null`) conform to it independently. The selector header (`output.h`) conditionally imports exactly one implementation at build time, controlled by the `-DOUT_NULL` compiler flag. The consumer (`main`) includes the selector and calls through the `OUT` macro — `OUT->output(greeting)` — without naming the implementation directly.

## The interface parcel (`output.c`)

`output.c` defines the shared vocabulary for all output implementations: a `Greeting` type and an `Output` function-pointer type. It declares and exports a **default parcel** (`_`), placing it in the `output` namespace.

```c
#pragma parcel _ { Greeting Output }
#include "export/output"

typedef char *Greeting;
typedef void (*Output)(Greeting greeting);
```

The interface contains no behaviour — only types. Both `Greeting` and `Output` are typedefs, so implementations import them using the **typedef stem prefix** form (`stem_Identifier`).

## Implementation parcels

Each implementation imports the interface with stem `out` and exports its own **named parcel** containing a single `output` variable — a function pointer conforming to `out_Output`.

### `output/stdout.c`

```c
#include <stdio.h>

#include "import/output/_.out"

#pragma parcel stdout { output }
#include "export/output/stdout"

static void print(out_Greeting greeting) {
    printf("%s\n", greeting);
}

out_Output output = print;
```

The `print` function is `static` — it is an internal implementation detail, invisible outside this file. Only the `output` function pointer is in the parcel interface. Callers see the abstraction (`output`), not the mechanism (`print`).

### `output/null.c`

```c
#include "import/output/_.out"

#pragma parcel null { output }
#include "export/output/null"

static void null(out_Greeting greeting) {
}

out_Output output = null;
```

The null implementation fulfils the same interface with a body that does nothing. It is a valid substitute wherever `stdout` is used — call sites do not change.

## The selector (`output.h`)

`output.h` is a plain C header — not a parcel — that centralises the implementation selection. It imports the interface parcel for the `out_Greeting` type, then conditionally imports exactly one implementation based on the `OUT_NULL` preprocessor flag, and defines the `OUT` macro to match the chosen stem:

```c
#include "import/output/_.out"

#if defined(OUT_NULL)
#include "import/output/null.null"
#define OUT null
#else
#include "import/output/stdout.std"
#define OUT std
#endif
```

Only the selected import is compiled in. Passing `-DOUT_NULL` at the compiler command line selects the null implementation; omitting it selects stdout. The `OUT` macro always resolves to the stem of the active implementation.

## The consumer (`main.c`)

`main.c` includes the selector header and calls `OUT->output(greeting)`. It names neither the implementation nor its stem directly:

```c
#include "output.h"

out_Greeting greeting = "Hello, world!";

int main() {
    OUT->output(greeting);
    return 0;
}
```

`OUT->output` uses the **variable/function stem pointer** form — `stem->identifier` — with the stem supplied indirectly through the `OUT` macro. The `out_Greeting` type, being a typedef, uses the **typedef stem prefix** form directly.

Switching implementations requires no changes to `main.c` — only the compiler flag changes.

## Concept map

| Concept | Where |
|--|--|
| Default parcel (`_`) | `output.c` — `#pragma parcel _ { Greeting Output }` |
| Named parcel | `output/stdout.c` — `#pragma parcel stdout { output }` |
| Named parcel | `output/null.c` — `#pragma parcel null { output }` |
| Typedef in interface | `output.c` — `Greeting`, `Output` |
| Variable in interface | `output/stdout.c`, `output/null.c` — `output` |
| Export with namespace path | `output.c` — `export/output` |
| Export with namespace path | `output/stdout.c` — `export/output/stdout` |
| Export with namespace path | `output/null.c` — `export/output/null` |
| Import with stem | `output/stdout.c`, `output/null.c` — `import/output/_.out` |
| Import with stem | `output/stdout.c`, `output/null.c` — `import/output/_.out` |
| Import with stem | `output.h` — `import/output/stdout.std`, `import/output/null.null` (conditional) |
| Typedef stem prefix | `output/stdout.c`, `output/null.c` — `out_Greeting`, `out_Output` |
| Typedef stem prefix | `main.c` — `out_Greeting` |
| Variable stem pointer | `main.c` — `OUT->output` (expands to `std->output` or `null->output`) |
| Selector header | `output.h` — conditionally imports one implementation; defines `OUT` |
| Build-time selection | `output.h` — `-DOUT_NULL` selects null; default selects stdout |
| Modular abstraction | `output.c` separates interface from implementation |
| Implementation substitution | `main.c` unchanged; only compiler flag changes |
