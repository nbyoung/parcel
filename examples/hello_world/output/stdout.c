#include <stdio.h>

#include "import/output/_.out"

#pragma parcel stdout { output }
#include "export/output/stdout"

static void print(out_Greeting greeting) {
    printf("%s\n", greeting);
}

out_Output output = print;
