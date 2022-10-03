#ifndef __STACK
#define __STACK


typedef int elem_t;
typedef int stack;


enum STACK_ERR {

    ERR_INVAL_STACK_PTR = 2,
    ERR_SIZE_BIGGER_CAPACITY = 3,
    ERR_STACK_CANARY_DEAD = 4,
    ERR_DATA_CANARY_DEAD = 5,
};


int StackCtor  (size_t base_size = 1024);
int Push       (stack vstk, elem_t value);
elem_t Pop     (stack vstk);
void StackDtor (stack vstk);


#endif