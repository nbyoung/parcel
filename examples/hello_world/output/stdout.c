#include <stdio.h>

#include "import/output/_.out"

#pragma parcel stdout { output }

void output(out_Greeting greeting) {
    printf("%s\n", greeting);
}

#include "export/output/stdout"
