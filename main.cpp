#include "list.h"

int main(int, char** argv) {
    List list = {};

    LIST_CHECK_AND_RETURN_ERRORS(ListCtor(&list, argv[1]));

    LIST_CHECK_AND_RETURN_ERRORS(InsertElement(&list, 5, 0), ListDtor(&list));

    LIST_CHECK_AND_RETURN_ERRORS(InsertElement(&list, 7, 1), ListDtor(&list));

    LIST_CHECK_AND_RETURN_ERRORS(InsertElement(&list, 6, 1), ListDtor(&list));

    LIST_CHECK_AND_RETURN_ERRORS(InsertElement(&list, 9, 2), ListDtor(&list));

    LIST_CHECK_AND_RETURN_ERRORS(InsertElement(&list, 3, 0), ListDtor(&list));
    
    LIST_CHECK_AND_RETURN_ERRORS(InsertElement(&list, 10, 4), ListDtor(&list));

    LIST_CHECK_AND_RETURN_ERRORS(DeleteElement(&list, 1), ListDtor(&list));

    LIST_CHECK_AND_RETURN_ERRORS(DeleteElement(&list, 3), ListDtor(&list));

    LIST_CHECK_AND_RETURN_ERRORS(DeleteElement(&list, 4), ListDtor(&list));

    LIST_CHECK_AND_RETURN_ERRORS(DeleteElement(&list, 5), ListDtor(&list));

    LIST_CHECK_AND_RETURN_ERRORS(InsertElement(&list, 9, 2), ListDtor(&list));

    LIST_CHECK_AND_RETURN_ERRORS(ListDtor(&list));

    return 0;
}