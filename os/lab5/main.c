#include "./list.h"
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
