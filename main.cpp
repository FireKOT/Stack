#include <stdio.h>
#include "stack.h"


int main () { 

    stack_t stk = StackCtor(-1);

    for (int i = 0; i < 10; i++) {
        StackPush(stk, i);
    }

    for (int i = 0; i < 10; i++) {
        printf("%d ", StackPop(stk));
    }

    StackDtor(stk);

    return 0;
}

