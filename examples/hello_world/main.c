#include "import/output/_.out"
#include "import/output/stdout.std"
#include "import/output/null.null"

out_Greeting greeting = "Hello, world!";

int main() {
    std->output(greeting);
    null->output(greeting);
    return 0;
}
