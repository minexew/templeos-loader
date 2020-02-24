#include "symtable.h"

#include <stdlib.h>
#include <string.h>

static struct sym* syms = NULL;

struct sym* lookupsym(void* address) {
    for (struct sym* sym = syms; sym; sym = sym->next) {
        if (address >= sym->address)
            return sym;
    }

    return NULL;
}

void addsym(const char* module, const char* name, void* address) {
    // the list is sorted by address (syms points to the highest-address symbol)

    struct sym* sym = (struct sym*) malloc(sizeof(struct sym));
    sym->module = strdup(module);           // Hugely inefficient and we don't care
    sym->name = strdup(name);
    sym->address = address;

    struct sym* prev = NULL;
    struct sym* curr = NULL;

    // find the first symbol after this one
    for (curr = syms; curr; curr = curr->next) {
        if (address == curr->address)
            // fuck
            return;

        if (address >= curr->address)
            break;

        prev = curr;
    }

    if (prev) {
        sym->next = prev->next;
        prev->next = sym;
    }
    else {
        sym->next = syms;
        syms = sym;
    }
}

struct sym* findsym(const char* name) {
    for (struct sym* sym = syms; sym; sym = sym->next) {
        if (!strcmp(sym->name, name))
            return sym;
    }

    return NULL;
}
