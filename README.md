# Parcel

The _Parcel_ encapsulation language brings simple modules, called _parcels_, to C.

**`foo.c`**
```C
#include <stdio.h>

#pragma parcel first { Message output }
#include "export/hello"

typedef char * Message;

void output(Message message) {
    printf("%s\n", message);
}
```

**`bar.c`**
```C
#include "import/first.this"

#pragma parcel second { message }
#include "export/second"

const this_Message message = "Hello, world!";
```

**`main.c`**
```C
#include "import/first.this"
#include "import/second.that"

int main() {
    this_output(that_message);
    return 0;
}
```

## Syntax

| Parcel | |
|--|--|
| `#pragma parcel <parcel> { [<identifier>] ... }` | Declare a parcel with a name and identifiers |
| `#include "<export>/<path>/<parcel>"` | Export to a path within the program's parcel namespace |
| `#include "<import>/<path>/<parcel>.<stem>"` | Import from the namespace using `<stem>` as a local prefix |

| _where_ | | |
|--|--|--|
| `<parcel>` | Name | Any C identifier, including `_` (underscore) |
| `[<identifier>] ...` | Interface | Identifiers of typedefs, variables and functions in the exporting file |
| `<export>` | Export base | Typically `export` |
| `<import>` | Import base | Typically `import` |
| `<path>` | Path | Path within the parcel namepace `<segment1>/<segment2>/...` |
| `<stem>` | Prefix | C identifier fragment applied to local identifiers |


