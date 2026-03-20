#include "import/output/_.out"

#if defined(OUTPUT_NULL)
#include "import/output/null.null"
#define OUT null
#else
#include "import/output/stdout.std"
#define OUT std
#endif
