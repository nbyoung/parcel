#pragma parcel _ { Greeting Output }

typedef char *Greeting;
typedef void (*Output)(Greeting greeting);

#include "export/output/_"
