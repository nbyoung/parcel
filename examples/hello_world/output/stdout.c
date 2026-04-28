#include <stdio.h>

#include "import/output/_.out"

void output(out_Greeting greeting) {
    printf("%s\n", greeting);
}

#pragma  parcel stdout
#pragma      function: output
#include "export/output/stdout"
