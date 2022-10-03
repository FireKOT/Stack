#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <time.h>
#include <windows.h>
#include "stack.h"
#include "config.h"


const unsigned long CANARY = 0xDEADBABE;
const elem_t        POISON = 0xADADADAD;


struct stack_prot {

    unsigned long StartBird;

    elem_t *values;

    size_t size;
    size_t capacity;
    
    double factor;

    unsigned long long StackHash;

    unsigned long EndBird;
};


static int  GetCode            ();
static void SetBirds           (void *mas, size_t size);
static void ResizeUp           (stack_prot *stk);
static void ResizeDown         (stack_prot *stk);
static void FillbyPoison       (elem_t *start, size_t size);
static char StackOK            (stack_prot *stk);
static void SetErr             (char *err, size_t num);
static void StackDump          (stack_prot *stk, char err);
static unsigned long long pow  (unsigned long long n, int k);
static unsigned long long hash (stack_prot *test);


static const int CODE = GetCode();


static int GetCode () {

    time_t now = time(NULL);
    void *tmp = calloc(1, 1);
    assert(!IsBadReadPtr(tmp, _msize(tmp)));

    return now^((int) tmp);
}

int StackCtor (size_t base_size) {

    elem_t *values = (elem_t*) calloc(base_size*sizeof(elem_t) + 2*sizeof(CANARY), 1);
    assert(!IsBadReadPtr(values, _msize(values)));

    SetBirds(values, base_size);
    FillbyPoison((elem_t*) ((char*)values + sizeof(CANARY)), base_size);

    stack_prot *stk = (stack_prot*) calloc(1, sizeof(stack_prot));
    assert(!IsBadReadPtr(stk, _msize(stk)));

    stk->StartBird = CANARY;

    stk->values = (elem_t*) ((char*) values + sizeof(CANARY));

    stk->size = 0;
    stk->capacity = base_size;

    stk->factor = 2.0;

    stk->StackHash = 0;

    stk->EndBird = CANARY;

    stk->StackHash = hash(stk);

    FILE *dump = fopen("dump.txt", "w");
    assert(dump);
    fclose(dump);

    int code = ((int) stk)^CODE;

    char err = StackOK(stk);
    if(err) StackDump(stk, err);

    return code;
}

static void SetBirds (void *mas, size_t size) {

    assert(mas);

    *((unsigned long*) mas) = CANARY;
    *((unsigned long*) ((char*)mas + size*sizeof(elem_t) + sizeof(CANARY))) = CANARY;
}

int Push (stack vstk, elem_t value) {

    stack_prot *stk = (stack_prot*) (vstk^CODE);
    char err = StackOK(stk);
    if(err) StackDump(stk, err);

    stk->values[stk->size] = value;
    stk->size++;

    stk->StackHash = hash(stk);

    if (stk->size >= stk->capacity) {
        ResizeUp(stk);
    }

    err = StackOK(stk);
    if(err) StackDump(stk, err);

    return 0;
}

elem_t Pop (stack vstk) {

    stack_prot *stk = (stack_prot*) (vstk^CODE);
    char err = StackOK(stk);
    if(err) StackDump(stk, err);

    stk->size--;
    elem_t value = stk->values[stk->size];
    stk->values[stk->size] = POISON;

    stk->StackHash = hash(stk);

    if (stk->size <= stk->capacity / (stk->factor*stk->factor)) {
        ResizeDown(stk);
    }
    
    err = StackOK(stk);
    if(err) StackDump(stk, err);

    return value;
}

static void ResizeUp (stack_prot *stk) {

    char err = StackOK(stk);
    if(err) StackDump(stk, err);

    elem_t *tmp = (elem_t*) realloc((char*)stk->values - sizeof(CANARY),\
                                    (size_t) (stk->capacity*stk->factor*sizeof(elem_t)) + 2*sizeof(CANARY));

    SetBirds(tmp, (size_t) (stk->capacity * stk->factor));
    FillbyPoison((elem_t*) ((char*)tmp + sizeof(CANARY) + stk->capacity*sizeof(elem_t)), stk->capacity);

    stk->values = (elem_t*) ((char*)tmp + sizeof(CANARY));
    stk->capacity = (size_t) (stk->capacity * stk->factor);

    stk->StackHash = hash(stk);

    err = StackOK(stk);
    if(err) StackDump(stk, err);
}

static void ResizeDown (stack_prot *stk) {

    char err = StackOK(stk);
    if(err) StackDump(stk, err);

    elem_t *tmp = (elem_t*) realloc((char*)stk->values - sizeof(CANARY),\
                                    (size_t) (stk->capacity/stk->factor*sizeof(elem_t)) + 2*sizeof(CANARY));
    SetBirds(tmp, (size_t) (stk->capacity / stk->factor));

    stk->values = (elem_t*) ((char*)tmp + sizeof(CANARY));
    stk->capacity = (size_t) (stk->capacity / stk->factor);

    stk->StackHash = hash(stk);

    err = StackOK(stk);
    if(err) StackDump(stk, err);
}

static void FillbyPoison (elem_t *start, size_t size) {

    assert(!IsBadReadPtr(start, size));

    for (size_t i = 0; i < size; i++) {
        start[i] = POISON;
    }
}

static char StackOK (stack_prot *stk) {

    char err = 0;

    if (IsBadReadPtr(stk, _msize(stk))) {
        SetErr(&err, 1);
        return err;
    }

    if (stk->values == 0) {
        SetErr(&err, 2);
    }
    if (stk->size > stk->capacity) {
        SetErr(&err, 3);
    }
    if(stk->StartBird != CANARY || stk->EndBird != CANARY) {
        SetErr(&err, 4);
    }
    if (*((unsigned long*) ((char*)stk->values - sizeof(CANARY))) != CANARY ||\
        *((unsigned long*) ((char*)stk->values + stk->capacity*sizeof(elem_t))) != CANARY) {
        SetErr(&err, 5);
    }
    if (hash(stk) != stk->StackHash) {
        SetErr(&err, 6);
    }

    return err;
}

static void SetErr (char *err, size_t num) {

    assert(!IsBadReadPtr(err, sizeof(char)));

    *err = *err | (char) (1 << (num-1));
}

static void StackDump (stack_prot *stk, char err) {

    FILE *dump = fopen("dump.txt", "a");

    fprintf(dump, "<<------------------------------->>\n");

    fprintf(dump, "Errors:\n");
    for (size_t i = 0; i < sizeof(err) * 8; i++) {
        if(err % 2) {
            fprintf(dump, "Error № %d\n", i+1);
        }

        err /= 2;
    }
    fprintf(dump, "\n");

    fprintf(dump, "Stack:\n");
    fprintf(dump, "StartBird: %ld\n", stk->StartBird);
    fprintf(dump, "values: %p\n", stk->values);
    fprintf(dump, "size: %d\n", stk->size);
    fprintf(dump, "capacity: %d\n", stk->capacity);
    fprintf(dump, "factor: %lg\n", stk->factor);
    fprintf(dump, "hash: %llu\n", stk->StackHash);
    fprintf(dump, "EndBird: %ld\n", stk->EndBird);

    fprintf(dump, "<<------------------------------->>\n\n");

    fclose(dump);
}

void StackDtor (stack vstk) {

    stack_prot *stk = (stack_prot*) (vstk^CODE);
    char err = StackOK(stk);
    if(err) StackDump(stk, err);

    free((char*) stk->values - sizeof(CANARY));
    free(stk);
}

static unsigned long long pow (unsigned long long n, int k) {

    if(k <= 0) return 1;
    if(k % 2 == 0) return pow(n, k / 2) * pow(n, k / 2);

    return n * pow(n, k - 1);
}

static unsigned long long hash (stack_prot *stk) {

    assert(stk);
    
    unsigned long long tmp = stk->StackHash;
    stk->StackHash = 0;

    unsigned long long n = 0;

    size_t num = 0;
    for (size_t i = 0; i < sizeof(*stk); i++) {
        num++;
        n += (*((char*)stk + i) + 128) * pow(313, num);
    }

    for (size_t i = 0; i < stk->capacity * sizeof(elem_t) + 2*sizeof(CANARY); i++) {
        num++;
        n += *((char*) stk->values - sizeof(CANARY) + i) * pow(313, num);
    }

    stk->StackHash = tmp;

    return n;
}