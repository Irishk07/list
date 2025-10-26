#include "list.h"

int main() {

    List list = {};

    ListCtor(&list);

    // ListDump(&list);

    InsertElement(&list, 5, 0);
    ListDump(&list);

    // InsertElement(&list, 7, 1);
    // ListDump(&list);

    // InsertElement(&list, 6, 1);
    // ListDump(&list);

    // InsertElement(&list, 9, 2);
    // ListDump(&list);

    // InsertElement(&list, 3, 0);
    // ListDump(&list);

    // DeleteElement(&list, 4);
    // ListDump(&list);

    ListDtor(&list);

    return 0;
}