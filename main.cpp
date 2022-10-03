#include <stdio.h>
#include "stack.h"


int main () { 

    stack stk = StackCtor(-1000);

    for (int i = 0; i < 10; i++) {
        Push(stk, i);
    }

    for (int i = 0; i < 10; i++) {
        printf("%d ", Pop(stk));
    }

    StackDtor(stk);

    return 0;
}

