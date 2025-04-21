#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "list.h"

int main() {
    void *handle = dlopen("./liblist.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        exit(1);
    }

    dlerror();

    listNode* (*makeNode)(int) = dlsym(handle, "makeNode");
    void (*insertAfter)(listNode*, listNode*) = dlsym(handle, "insertAfter");
    void (*deleteNode)(listNode*) = dlsym(handle, "deleteNode");
    void (*deleteList)(listNode*) = dlsym(handle, "deleteList");
    void (*printList)(listNode*) = dlsym(handle, "printList");
    void (*serializeList)(listNode*) = dlsym(handle, "serializeList");

    char *error;
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Error: %s\n", error);
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    listNode* head = makeNode(100);
    for (int i = 1; i < 15; i++) {
        insertAfter(head, makeNode(i * (i - 1)));
    }

    printList(head);
    serializeList(head);
    deleteList(head);

    dlclose(handle);

}