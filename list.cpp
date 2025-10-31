#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "list.h"

#include "canary.h"
#include "string_functions.h"


list_status ListCtor(List* list, const char* dump_filename, const char* directory) {
    assert(list);
    assert(dump_filename);
    assert(directory);

    list->capacity = START_CAPACITY;
    list->size     = 0;
    list->free     = 1;
    list->num_dump = 0;
    list->directory = directory;

    // TODO prev and next are indexes, not type_t
    list->data = (type_t*)calloc(RealSizeList(list->capacity + 1, CNT_CANARIES), sizeof(type_t)); // +1 because list->data[0] is not elem
    list->next = (int*)calloc(list->capacity + 1, sizeof(int)); // +1 because list->next[0] == head, not elem
    list->prev = (int*)calloc(list->capacity + 1, sizeof(int)); // +1 because list->prev[0] == tail, not elem

    if (list->data == NULL || list->next == NULL || list->prev == NULL)
        LIST_CHECK_AND_RETURN_ERRORS(NOT_ENOUGH_MEMORY,     free(list->data);
                                                            free(list->next);
                                                            free(list->prev););

    InitFreeSpace(list);
    list->data[0] = DEFAULT_POISON;

    ON_CANARY(
        SettingCanariesToBegin(list->data);
        SettingCanariesToEnd(list->data, list->capacity);
    )


    FILE* dump_file = fopen(dump_filename, "w");
    if (dump_file == NULL)
        LIST_CHECK_AND_RETURN_ERRORS(OPEN_ERROR,     free(list->data);
                                                     free(list->next);
                                                     free(list->prev););
    list->dump_file = dump_file;

    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list),   free(list->data);
                                                     free(list->next);
                                                     free(list->prev);
                                                     fclose(dump_file););

    return SUCCESS;
}

void InitFreeSpace(List* list) {
    for (size_t i = list->size + 1; i <= list->capacity; ++i) { // +1 because list->data[0] is not elem
        list->data[i] = DEFAULT_POISON;
    }

    for (size_t i = list->free; i <= list->capacity; ++i) {
        list->next[i] = (int)i + 1;
        list->prev[i] = -1;
    }

    list->next[list->capacity] = 0;
}

list_status ListVerify(List* list) {
    if (list == NULL)                   
        DUMP_AND_RETURN_ERRORS(NULL_POINTER_ON_STRUCT);

    if (list->data == NULL)             
        DUMP_AND_RETURN_ERRORS(NULL_POINTER_ON_DATA);

    if (list->next == NULL)    
        DUMP_AND_RETURN_ERRORS(NULL_POINTER_ON_NEXT);         

    if (list->prev == NULL)
        DUMP_AND_RETURN_ERRORS(NULL_POINTER_ON_PREV);

    if (list->capacity > MAX_CAPACITY)
        DUMP_AND_RETURN_ERRORS(CAPACITY_IS_TOO_BIG);

    if (list->size > list->capacity)    
        DUMP_AND_RETURN_ERRORS(SIZE_BIGGER_THAN_CAPACITY);

    if (list->free > list->capacity)    
        DUMP_AND_RETURN_ERRORS(INVALIDE_FREE);

    if (list->dump_file == NULL)        
        DUMP_AND_RETURN_ERRORS(NULL_POITER_ON_DUMP_FILE);

    ON_CANARY(
        if (list->data[0] != CANARY ||
            list->data[OffsetToNewElement(list->capacity, CNT_CANARIES)] != CANARY) 
                DUMP_AND_RETURN_ERRORS(CORRUPTED_CANARY);    
    )

    size_t cnt_free = 1;
    for (size_t i = list->free; list->next[i] != 0; i = (size_t)list->next[i], ++cnt_free) {
        if (cnt_free > list->capacity - list->size) 
            DUMP_AND_RETURN_ERRORS(FREE_HAS_CYCLE);

        if (list->data[i] != DEFAULT_POISON ||
            list->prev[i] != -1)               
            DUMP_AND_RETURN_ERRORS(LIST_DATA_POISON);
    }
    if (cnt_free < list->capacity - list->size) 
        DUMP_AND_RETURN_ERRORS(NOT_ENOUGH_ELEMENTS_IN_FREE);

    size_t cnt_next = 1;
    for (int i = ListHead(list); list->next[i] != 0; i = list->next[i], ++cnt_next) {
        if (cnt_next > list->size)                                       
            DUMP_AND_RETURN_ERRORS(NEXT_HAS_CYCLE);    

        if (list->data[i] == DEFAULT_POISON ||
            list->prev[i] == -1 || i != list->prev[list->next[i]])  
            DUMP_AND_RETURN_ERRORS(LIST_DATA_POISON);
    }
    if (cnt_next < list->size)
        DUMP_AND_RETURN_ERRORS(NOT_ENOUGH_ELEMENTS_IN_NEXT);

    size_t cnt_prev = 1;
    for (int i = ListTail(list); list->prev[i] != 0; i = list->prev[i], ++cnt_prev) {
        if (cnt_prev > list->size)                
            DUMP_AND_RETURN_ERRORS(PREV_HAS_CYCLE);

        if (i != list->next[list->prev[i]])  
            DUMP_AND_RETURN_ERRORS(LIST_DATA_POISON);
    }
    if (cnt_prev < list->size)
        DUMP_AND_RETURN_ERRORS(NOT_ENOUGH_ELEMENTS_IN_PREV);

    if (cnt_next != cnt_prev)
        DUMP_AND_RETURN_ERRORS(UNEQUAL_CNT_ELEMENTS_NP);
        
    if (list->size == 0) {
        cnt_next--;
        cnt_prev--;
    }

    if(list->capacity - cnt_free != cnt_next)
        DUMP_AND_RETURN_ERRORS(UNEQUAL_CNT_ELEMENTS_ND);

    if(list->capacity - cnt_free != cnt_prev)
        DUMP_AND_RETURN_ERRORS(UNEQUAL_CNT_ELEMENTS_PD);

    if (ListHead(list) < 0 ||
        ListHead(list) >= (type_t)list->capacity)  
        DUMP_AND_RETURN_ERRORS(INVALIDE_HEAD);

    if (ListTail(list) < 0 ||
        ListTail(list) >= (type_t)list->capacity)  
        DUMP_AND_RETURN_ERRORS(INVALIDE_TAIL);

    return SUCCESS;
}

// need insert element after physical_index
list_status InsertElementAfter(List* list, type_t elem, size_t physical_index) {
    About_elem about_elem = {.value = elem, .physical_index = physical_index};

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, about_elem, "Before", DUMP_INFO, INSERT_AFTER, SUCCESS));

    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfterNoDump(list, elem, physical_index));

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, about_elem, "After", DUMP_INFO, INSERT_AFTER, SUCCESS));

    return SUCCESS;
}

// need insert element befort physical_index
list_status InsertElementBefore(List* list, type_t elem, size_t physical_index) {
    About_elem about_elem = {.value = elem, .physical_index = physical_index};

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, about_elem, "Before", DUMP_INFO, INSERT_BEFORE, SUCCESS));

    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfterNoDump(list, elem, (size_t)list->prev[physical_index]));

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, about_elem, "After", DUMP_INFO, INSERT_BEFORE, SUCCESS));

    return SUCCESS;
}

// need insert element after physical_index
list_status InsertElementAfterNoDump(List* list, type_t elem, size_t physical_index) {
    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    list->size++;

    if (list->size >= list->capacity) {
        size_t old_capacity = list->capacity;
        list->capacity *= REALLOC_COEFF;

        list->free = list->size;

        LIST_CHECK_AND_RETURN_ERRORS(ListResize(list, old_capacity));
    }

    if (physical_index > list->capacity || list->prev[physical_index] == -1)
        LIST_CHECK_AND_RETURN_ERRORS(INVALID_POSITION);

    list->data[list->free] = elem;

    type_t current_free = (type_t)list->free;
    list->free = (size_t)list->next[list->free];

    list->prev[list->next[physical_index]] = current_free;
    list->prev[current_free] = (type_t)physical_index;

    list->next[current_free] = list->next[physical_index];
    list->next[physical_index] = current_free;

    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    return SUCCESS;
}

list_status ListResize(List* list, size_t old_capacity) {
    assert(list);

    type_t* temp_data = (type_t*)my_recalloc(list->data, RealSizeList(list->capacity + 1, CNT_CANARIES) * sizeof(type_t),
                                                         RealSizeList(old_capacity + 1, CNT_CANARIES) * sizeof(type_t)); // +1 because list->data[0] is not elem

    int* temp_next = (int*)my_recalloc(list->next, (list->capacity + 1) * sizeof(int),
                                                   (old_capacity + 1) * sizeof(int)); // +1 because list->next[0] == head, not elem

    int* temp_prev = (int*)my_recalloc(list->prev, (list->capacity + 1) * sizeof(int),
                                                   (old_capacity + 1) * sizeof(int)); // +1 because list->prev[0] == tail, not elem

    if (temp_data == NULL || temp_next == NULL || temp_prev == NULL) {
        LIST_CHECK_AND_RETURN_ERRORS(NOT_ENOUGH_MEMORY, free(temp_data);
                                                        free(temp_next);
                                                        free(temp_prev));
    }

    list->data = temp_data;
    list->next = temp_next;
    list->prev = temp_prev;

    InitFreeSpace(list);

    ON_CANARY(SettingCanariesToEnd(list->data, list->capacity));

    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    return SUCCESS;
}

// need delete element from physical_index
list_status DeleteElement(List* list, size_t physical_index) {
    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    About_elem about_elem = {.physical_index = physical_index};
    LIST_CHECK_AND_RETURN_ERRORS(GetElement(list, physical_index, &about_elem.value));

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, about_elem, "Before", DUMP_INFO, DELETE, SUCCESS));

    if (physical_index > list->capacity || physical_index == 0)
        LIST_CHECK_AND_RETURN_ERRORS(INVALID_POSITION);

    if (list->size == 0 || list->prev[physical_index] == -1)
        LIST_CHECK_AND_RETURN_ERRORS(NOT_EXISTING_ELEMENT);

    list->data[physical_index] = DEFAULT_POISON;

    list->next[list->prev[physical_index]] = list->next[physical_index];
    list->prev[list->next[physical_index]] = list->prev[physical_index];

    list->prev[physical_index] = -1;
    list->next[physical_index] = (type_t)list->free;

    list->free = physical_index;

    list->size--;

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, about_elem, "After", DUMP_INFO, DELETE, SUCCESS));

    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    return SUCCESS;
}

list_status GetElement(List* list, size_t physical_index, type_t* elem) {
    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list))

    if (physical_index >= list->capacity)
        LIST_CHECK_AND_RETURN_ERRORS(INVALID_POSITION);

    if (list->prev[physical_index] == -1)
        LIST_CHECK_AND_RETURN_ERRORS(NOT_EXISTING_ELEMENT);

    *elem = list->data[physical_index];

    return SUCCESS;
}

type_t ListHead(List* list) {
    return list->next[0];
}

type_t ListTail(List* list) {
    return list->prev[0];
}

list_status ListHTMLDump(List* list, About_elem about_elem, const char* before_or_after, int line, const char* file, type_of_dump type_dump, list_status status) {
    if (status == NULL_POITER_ON_DUMP_FILE) {
        return status;
    }

    fprintf(list->dump_file, "<pre>\n <font size = \"6\">\n");

    if (type_dump == ERROR_DUMP) {
        fprintf(list->dump_file, "<h2> ERROR ERROR ERROR </h2>\n");

        fprintf(list->dump_file, "<h3><font color=red> ");
        PrintErrors(status, list->dump_file);
        fprintf(list->dump_file, "</font></h3>\n");
    }

    if (status == NULL_POINTER_ON_STRUCT ||
        status == NULL_POINTER_ON_DATA   ||
        status == NULL_POINTER_ON_NEXT   ||
        status == NULL_POINTER_ON_PREV) {
            return status;
        }

    if (type_dump == INSERT_AFTER)
        fprintf(list->dump_file, "<h3> DUMP <font color=green> %s Insert <%d> after physical_index [%zu] </font> </h3>\n",
                before_or_after, about_elem.value, about_elem.physical_index);
    else if (type_dump == INSERT_BEFORE)
        fprintf(list->dump_file, "<h3> DUMP <font color=green> %s Insert <%d> before physical_index [%zu] </font> </h3>\n",
                before_or_after, about_elem.value, about_elem.physical_index);
    else if (type_dump == DELETE)
        fprintf(list->dump_file, "<h3> DUMP <font color=red> %s Delete <%d> from physical_index [%zu] </font> </h3>\n",
                before_or_after, about_elem.value, about_elem.physical_index);

    fprintf(list->dump_file, "List {%s: %d}\n", file, line);

    fprintf(list->dump_file, "Capacity: %zu\n", list->capacity);
    fprintf(list->dump_file, "Size: %zu\n", list->size);
    fprintf(list->dump_file, "Free: %zu\n", list->free);

    fprintf(list->dump_file, "Indexes |");

    fprintf(list->dump_file, "    Service    | ");
    for (size_t i = 1; i <= list->capacity; ++i) {
        fprintf(list->dump_file, "%4zu | ", i);
    }
    ON_CANARY(fprintf(list->dump_file, "   Service"));
    fprintf(list->dump_file, "\n");

    fprintf(list->dump_file, "Data    |");
    fprintf(list->dump_file, " %4d (Canary) | ", list->data[0]);
    for (size_t i = 1; i <= list->capacity; ++i) {
        fprintf(list->dump_file, "%4d | ", list->data[i]);
    }
    ON_CANARY(fprintf(list->dump_file, "%4d (Canary)", list->data[list->capacity + 1]));
    fprintf(list->dump_file, "\n");

    fprintf(list->dump_file, "Next    |");
    fprintf(list->dump_file, " %4d (Head)   | ", list->next[0]);
    for (size_t i = 1; i <= list->capacity; ++i) {
        fprintf(list->dump_file, "%4d | ", list->next[i]);
    }
    ON_CANARY(fprintf(list->dump_file, "     -     "));
    fprintf(list->dump_file, "\n");

    fprintf(list->dump_file, "Prev    |");
    fprintf(list->dump_file, " %4d (Tail)   | ", list->prev[0]);
    for (size_t i = 1; i <= list->capacity; ++i) {
        fprintf(list->dump_file, "%4d | ", list->prev[i]);
    }
    ON_CANARY(fprintf(list->dump_file, "     -     "));
    fprintf(list->dump_file, "\n");

    fprintf(list->dump_file, "\n");

    LIST_CHECK_AND_RETURN_ERRORS(GenerateGraph(list));

    char command[MAX_LEN_NAME] = {};
    snprintf(command, MAX_LEN_NAME, "dot %s/graphes/graph%d.txt -T png -o %s/images/image%d.png", list->directory, list->num_dump, list->directory, list->num_dump);
    if (system((const char*)command) != 0)
        LIST_CHECK_AND_RETURN_ERRORS(EXECUTION_FAILED,      fprintf(list->dump_file, "Error with create image:(\n"));

    fprintf(list->dump_file, "<img src = %s/images/image%d.png width = 1700px>", list->directory, list->num_dump);

    fprintf(list->dump_file, "\n\n");

    fprintf(list->dump_file, "Meow <3\n\n");

    fprintf(list->dump_file, "<img src = cat.png width = 150px> </font>");

    fprintf(list->dump_file, "\n\n");

    list->num_dump++;

    return status;
}

list_status GenerateGraph(List* list) {
    char filename_graph[MAX_LEN_NAME] = {};
    snprintf(filename_graph, MAX_LEN_NAME, "%s/graphes/graph%d.txt", list->directory, list->num_dump);

    FILE* graph = fopen(filename_graph, "w");
    if (graph == NULL)
        LIST_CHECK_AND_RETURN_ERRORS(OPEN_ERROR);

    fprintf(graph, "digraph {\n");
    fprintf(graph, "    splines = ortho;\n");
    fprintf(graph, "    nodesep = 0.5;\n");
    fprintf(graph, "    ranksep = 1;\n");

    fprintf(graph, "    node [shape = octagon, fontcolor = white, fillcolor = \"#CC0000\", style = filled];\n");
    fprintf(graph, "    edge [penwidth = 3, color = red];\n");

    fprintf(graph, "    node0 [shape = \"plaintext\";  color = black; fontcolor = black; style = filled; fillcolor = \"#00FFFF\"; label = <<table cellspacing = \"0\">\n"
                        "<tr><td colspan = \"2\">idx = 0</td></tr>"
                        "<tr><td colspan = \"2\">canary = %d</td></tr>"
                        "<tr><td>head = %d</td><td>tail = %d</td></tr></table>>];\n",
                    list->data[0], list->next[0], list->prev[0]);

    for (size_t i = 1; i <= list->capacity; ++i) {
        if ((list->next[i] == 0 && list->prev[i] != -1))
            fprintf(graph, "    node%zu [shape = \"plaintext\"; color = black; fontcolor = black; style = filled; fillcolor = \"#99FF99\"; label = <<table cellspacing = \"0\">\n"
                        "<tr><td colspan = \"2\">idx = %zu</td></tr>"
                        "<tr><td colspan = \"2\">data = %d</td></tr>"
                        "<tr><td> <font color = \"#330099\">next = %d</font></td><td>prev = %d</td></tr></table>>];\n",
                    i, i, list->data[i], list->next[i], list->prev[i]);
        else if (list->prev[i] == 0)
            fprintf(graph, "    node%zu [shape = \"plaintext\"; color = black; fontcolor = black; style = filled; fillcolor = \"#99FF99\"; label = <<table cellspacing = \"0\">\n"
                        "<tr><td colspan = \"2\">idx = %zu</td></tr>"
                        "<tr><td colspan = \"2\">data = %d</td></tr>"
                        "<tr><td>next = %d</td><td><font color = \"#990099\">prev = %d</font></td></tr></table>>];\n",
                    i, i, list->data[i], list->next[i], list->prev[i]);
        else if (list->prev[i] != -1)
            fprintf(graph, "    node%zu [shape = \"plaintext\"; color = black; fontcolor = black; style = filled; fillcolor = \"#99FF99\"; label = <<table cellspacing = \"0\">\n"
                    "<tr><td colspan = \"2\">idx = %zu</td></tr>"
                    "<tr><td colspan = \"2\">data = %d</td></tr>"
                    "<tr><td>next = %d</td><td>prev = %d</td></tr></table>>];\n",
                i, i, list->data[i], list->next[i], list->prev[i]);
        else
            fprintf(graph, "    node%zu [shape = \"plaintext\"; color = black; fontcolor = black; style = filled; fillcolor = \"#9999FF\"; label = <<table cellspacing = \"0\">\n"
                "<tr><td colspan = \"2\">idx = %zu</td></tr>"
                "<tr><td colspan = \"2\">data = %d</td></tr>"
                "<tr><td>next = %d</td><td>prev = %d</td></tr></table>>];\n",
            i, i, list->data[i], list->next[i], list->prev[i]);
    }

    fprintf(graph, "    {rank = same; ");
    for (size_t i = 0; i <= list->capacity - 1; ++i) {
        fprintf(graph, "node%zu ", i);
    }
    fprintf(graph, "node%zu};\n", list->capacity);

    fprintf(graph, "    ");
    for (size_t i = 0; i <= list->capacity - 1; ++i) {
        fprintf(graph, "node%zu -> ", i);
    }
    fprintf(graph, "node%zu [style = invis];\n", list->capacity);

    for (size_t i = 0; i <= list->capacity; ++i) {
        if (list->prev[i] == -1 && list->next[i] != 0)
            fprintf(graph, "    node%zu -> node%d [penwidth = 1, color = \"#660066\", arrowhead = vee, weight = 0];\n", i, list->next[i]);

        else if (i == (size_t)list->prev[list->next[i]])
            fprintf(graph, "    node%zu -> node%d [penwidth = 1, color = blue, weight = 0, dir = \"both\", arrowhead = vee, arrowtail = vee]\n", i, list->next[i]);
        
        else if (i != (size_t)list->prev[list->next[i]] && list->next[i] != 0) {
            fprintf(graph, "    node%zu -> node%d [color = orange, weight = 0, arrowhead = vee]\n", i, list->next[i]);
            fprintf(graph, "    node%d -> node%d [color = red, weight = 0, arrowhead = vee]\n", list->next[i], list->prev[list->next[i]]);
        }
    }

    fprintf(graph, "    node_head [shape = component;  color = black; fontcolor = black; style = filled; fillcolor = \"#FFFF99\"; label = \"Head\"];\n");
    fprintf(graph, "    node_tail [shape = component;  color = black; fontcolor = black; style = filled; fillcolor = \"#FFFF99\"; label = \"Tail\"];\n");
    fprintf(graph, "    node_free [shape = component;  color = black; fontcolor = black; style = filled; fillcolor = \"#FFFF99\"; label = \"Free\"];\n");

    fprintf(graph, "    node_head -> node%d [penwidth = 2, color = \"#FF0099\", arrowhead = vee];\n", list->next[0]);
    fprintf(graph, "    node_tail -> node%d [penwidth = 2, color = \"#FF0099\", arrowhead = vee];\n", list->prev[0]);
    fprintf(graph, "    node_free -> node%zu [penwidth = 2, color = \"#FF0099\", arrowhead = vee];\n", list->free);

    fprintf(graph, "}");

    if (fclose(graph) == EOF) {
        LIST_CHECK_AND_RETURN_ERRORS(CLOSE_ERROR,    perror("Error is:"));
    }

    return SUCCESS;
}

void PrintErrors(int error, FILE* stream) {
    if (error == SUCCESS                    ) fprintf(stream, "ALL_RIGHT\n");
    if (error == NULL_POINTER_ON_STRUCT     ) fprintf(stream, "Null pointer on cycle\n");
    if (error == NULL_POINTER_ON_DATA       ) fprintf(stream, "Null pointer on data\n");
    if (error == NULL_POINTER_ON_NEXT       ) fprintf(stream, "Null pointer on next\n");
    if (error == NULL_POINTER_ON_PREV       ) fprintf(stream, "Null pointer on prev\n");
    if (error == SIZE_BIGGER_THAN_CAPACITY  ) fprintf(stream, "Size is bigger than capacity\n");
    if (error == INVALIDE_FREE              ) fprintf(stream, "Invalid free\n");
    if (error == INVALIDE_HEAD              ) fprintf(stream, "Invalid head\n");
    if (error == INVALIDE_TAIL              ) fprintf(stream, "Invalid tail\n");
    if (error == NOT_EXISTING_ELEMENT       ) fprintf(stream, "Not existing element\n");
    if (error == NOT_ENOUGH_MEMORY          ) fprintf(stream, "Not enough memory\n");
    if (error == OPEN_ERROR                 ) fprintf(stream, "Open error\n");
    if (error == INVALID_POSITION           ) fprintf(stream, "Invalid position\n");
    if (error == CLOSE_ERROR                ) fprintf(stream, "Close error\n");
    if (error == EXECUTION_FAILED           ) fprintf(stream, "Execution failed\n");
    if (error == NULL_POITER_ON_DUMP_FILE   ) fprintf(stream, "Null pointer on dump file\n");
    if (error == CAPACITY_IS_TOO_BIG        ) fprintf(stream, "Capacity is too big or negative\n");
    if (error == CORRUPTED_CANARY           ) fprintf(stream, "Corrupted canary in data\n");
    if (error == LIST_DATA_POISON           ) fprintf(stream, "List data is poison\n");
    if (error == FREE_HAS_CYCLE             ) fprintf(stream, "Free has cycle\n");
    if (error == TEST_ERROR                 ) fprintf(stream, "Test error\n");
    if (error == NEXT_HAS_CYCLE             ) fprintf(stream, "Next has cycle\n");
    if (error == PREV_HAS_CYCLE             ) fprintf(stream, "Prev has cycle\n");
    if (error == NOT_ENOUGH_ELEMENTS_IN_NEXT) fprintf(stream, "Not enough elements in next\n");
    if (error == NOT_ENOUGH_ELEMENTS_IN_PREV) fprintf(stream, "Not enough elements in prev\n");
    if (error == NOT_ENOUGH_ELEMENTS_IN_FREE) fprintf(stream, "Not enough elements in free\n");
    if (error == UNEQUAL_CNT_ELEMENTS_NP    ) fprintf(stream, "Unequal count elements if next and prev\n");
    if (error == UNEQUAL_CNT_ELEMENTS_ND    ) fprintf(stream, "Unequal count elements if next and data\n");
    if (error == UNEQUAL_CNT_ELEMENTS_PD    ) fprintf(stream, "Unequal count elements if prev and data\n");
}

list_status ListDtor(List* list) {
    list_status code_error = ListVerify(list);

    free(list->data);
    free(list->next);
    free(list->prev);

    if (fclose(list->dump_file) == EOF) {
        LIST_CHECK_AND_RETURN_ERRORS(CLOSE_ERROR,    perror("Error is:"));
    }

    *list = {};

    return code_error;
}
