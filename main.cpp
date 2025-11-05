#include "list.h"
#include "test.h"

int main(int, char** argv) {
    List list = {};

    LIST_CHECK_AND_RETURN_ERRORS(ListCtor(&list, argv[1], argv[2]));

    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, 2, 0), ListDtor(&list));
    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, 4, 1), ListDtor(&list));
    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, 5, 2), ListDtor(&list));
    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, 3, 1), ListDtor(&list));
    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, 6, 3), ListDtor(&list));
    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfter(&list, 7, 4), ListDtor(&list));
    LIST_CHECK_AND_RETURN_ERRORS(DeleteElement(&list, 3), ListDtor(&list));
    // list.next[8] = (int)list.free;
    // ListHTMLDump(&list, {.value = 335, .physical_index = 6}, "nothing", DUMP_INFO, ERROR_DUMP, LIST_DATA_POISON);
    // list.prev[6] = 335;
    // ListHTMLDump(&list, {.value = 335, .physical_index = 6}, "change prev on elem 6 on value 335", DUMP_INFO, ERROR_DUMP, LIST_DATA_POISON);

    LIST_CHECK_AND_RETURN_ERRORS(DeleteElement(&list, 2), ListDtor(&list));

    ListHTMLDump(&list, {.value = DEFAULT_POISON, .physical_index = 0}, "Before linearization", DUMP_INFO, JUST_DUMP, SUCCESS);
    LIST_CHECK_AND_RETURN_ERRORS(Linearization(&list), ListDtor(&list));
    ListHTMLDump(&list, {.value = DEFAULT_POISON, .physical_index = 0}, "After linearization", DUMP_INFO, JUST_DUMP, SUCCESS);

    // list.capacity = 8;
    // ListResize(&list, 16);
    // ListHTMLDump(&list, {.value = DEFAULT_POISON, .physical_index = 0}, "After resize down", DUMP_INFO, JUST_DUMP, SUCCESS);
    
    LIST_CHECK_AND_RETURN_ERRORS(ListDtor(&list));

    // Tests();

    return 0;
}
