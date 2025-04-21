#include <stdio.h>
#include <stdlib.h>

typedef struct listnode {
    int data;
    struct listnode *prev;
    struct listnode *next;
} listNode;

listNode *makeNode (int data);
void insertAfter (listNode *oldNode, listNode *newNode);
void deleteNode (listNode *node);
void deleteList (listNode *head);
void printList (listNode *head);
void serializeList (listNode *head);