#include <stdbool.h>
#include <stdio.h>

void expectEq(const char *name, int result, int expected) {
    if (result == expected) {
        printf("\x1b[0;30;42mPASS\x1b[0m\t");
    } else {
        printf("\x1b[0;39;41mFAIL\x1b[0m\t");
    }
    printf("%s:\texpected %d, got %d\n", name, expected, result);
}

int main() {
    extern int example(int, int);
	extern int testVars(int);

	expectEq("example", example(2, 3), 13);
	expectEq("testVars", testVars(4), -20);

    return 0;
}
