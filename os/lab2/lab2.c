#include <stdio.h>
#include <stdlib.h>

typedef struct listnode {
    int data;
    struct listnode *prev;
    struct listnode *next;
} listNode;

listNode *makeNode(int data) {
    listNode *node = malloc(sizeof(listNode));
    node->data = data;
    node->prev = node;
    node->next = node;
    return node;    
}

void insertAfter(listNode *oldNode, listNode *newNode) {
    if (newNode->prev != newNode || newNode->next != newNode) {
        printf("Error: the new node can't have any links to other nodes!\n");
        return;
    }
    newNode->next = oldNode->next;
    newNode->prev = oldNode;
    oldNode->next->prev = newNode;
    oldNode->next = newNode;
}

void deleteNode(listNode *node) {
    if (node == NULL) return;
    if (node->next == node && node->prev == node) {
        free(node);
        return;
    }
    node->prev->next = node->next;
    node->next->prev = node->prev;
    free(node);
}

void deleteList(listNode* head) {
    if (head == NULL) return;
    listNode *current = head;
    listNode *nextNode;
    do {
        nextNode = current->next;
        free(current);
        current = nextNode;
    } while (current != head);
}

void printList(listNode *head) {
    if (head == NULL) return;
    listNode *current = head;
    do {
        printf("%d ", current->data);
        current = current->next;
    } while (current != head);
    printf("\n");
}

void serializeList(listNode *head) {
    if (head == NULL) return;
    FILE *file = fopen("file.txt", "w");
    if (file == NULL) {
        printf("Failed to open file");
        return;
    }
    listNode *current = head;
    do {
        fprintf(file, "%d\n", current->data);
        current = current->next;
    } while (current != head);
    fclose(file);
    return;
}

int main() {
    listNode* head = makeNode(100);
    for (int i = 1; i < 10; i++) {
        insertAfter(head, makeNode(i * (i - 1)));
    }
    printList(head);
	serializeList(head);
    deleteList(head);
    return 0;
}
