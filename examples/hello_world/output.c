typedef char *Greeting;
typedef void (*Output)(Greeting greeting);

#pragma  parcel _
#pragma      typedef: Greeting
#pragma      function: Output
#include "export/output/_"
