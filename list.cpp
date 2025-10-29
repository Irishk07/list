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
    if (list == NULL)                   return NULL_POINTER_ON_STRUCT;

    if (list->data == NULL)             return NULL_POINTER_ON_DATA;

    if (list->next == NULL)             return NULL_POINTER_ON_NEXT;

    if (list->prev == NULL)             return NULL_POINTER_ON_PREV;

    if (list->capacity > MAX_CAPACITY)  return CAPACITY_IS_TOO_BIG;

    if (list->size > list->capacity)    return SIZE_BIGGER_THAN_CAPACITY;

    if (list->free > list->capacity)    return INVALIDE_FREE;

    if (list->dump_file == NULL)        return NULL_POITER_ON_DUMP_FILE;

    ON_CANARY(
        if (list->data[0] != CANARY ||
            list->data[OffsetToNewElement(list->capacity, CNT_CANARIES)] != CANARY) return CORRUPTED_CANARY;
    )

    size_t cnt = 0;
    for (int i = ListHead(list); list->next[i] != 0; i = list->next[i], ++cnt) {
        if (cnt > list->size)           return LIST_HAS_CYCLE;

        if (list->data[i] == DEFAULT_POISON ||
            list->prev[i] == -1)        return LIST_DATA_POISON;
    }

    cnt = 0;
    for (size_t i = list->free; list->next[i] != 0; i = (size_t)list->next[i], ++cnt) {
        if (cnt > list->capacity - list->size) return LIST_HAS_CYCLE;

        if (list->data[i] != DEFAULT_POISON ||
            list->prev[i] != -1)               return LIST_DATA_POISON;
    }

    if (ListHead(list) < 0 ||
        ListHead(list) >= (type_t)list->capacity)  return INVALIDE_HEAD;

    if (ListTail(list) < 0 ||
        ListTail(list) >= (type_t)list->capacity)  return INVALIDE_TAIL;

    return SUCCESS;
}

// need insert element after physical_index
list_status InsertElementAfter(List* list, type_t elem, size_t physical_index) {
    About_elem about_elem = {.value = elem, .physical_index = physical_index};

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, about_elem, "Before", DUMP_INFO, INSERT_AFTER));

    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfterNoDump(list, elem, physical_index));

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, about_elem, "After", DUMP_INFO, INSERT_AFTER));

    return SUCCESS;
}

// need insert element befort physical_index
list_status InsertElementBefore(List* list, type_t elem, size_t physical_index) {
    About_elem about_elem = {.value = elem, .physical_index = physical_index};

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, about_elem, "Before", DUMP_INFO, INSERT_BEFORE));

    LIST_CHECK_AND_RETURN_ERRORS(InsertElementAfterNoDump(list, elem, (size_t)list->prev[physical_index]));

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, about_elem, "After", DUMP_INFO, INSERT_BEFORE));

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

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, about_elem, "Before", DUMP_INFO, DELETE));

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

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, about_elem, "After", DUMP_INFO, DELETE));

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

list_status ListHTMLDump(List* list, About_elem about_elem, const char* type_dump, int line, const char* file, function_name func_name) {
    fprintf(list->dump_file, "<pre>\n <font size = \"6\">\n");

    if (func_name == INSERT_AFTER)
        fprintf(list->dump_file, "<h3> DUMP <font color=green> %s Insert <%d> after physical_index [%zu] </font> </h3>\n",
                type_dump, about_elem.value, about_elem.physical_index);
    else if (func_name == INSERT_BEFORE)
        fprintf(list->dump_file, "<h3> DUMP <font color=green> %s Insert <%d> before physical_index [%zu] </font> </h3>\n",
                type_dump, about_elem.value, about_elem.physical_index);
    else if (func_name == DELETE)
        fprintf(list->dump_file, "<h3> DUMP <font color=red> %s Delete <%d> from physical_index [%zu] </font> </h3>\n",
                type_dump, about_elem.value, about_elem.physical_index);

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

    fprintf(list->dump_file, "<img src = %s/images/image%d.png width = 1500px>", list->directory, list->num_dump);

    fprintf(list->dump_file, "\n\n");

    fprintf(list->dump_file, "Meow <3\n\n");

    fprintf(list->dump_file, "<img src = cat.png width = 150px> </font>");

    fprintf(list->dump_file, "\n\n");

    list->num_dump++;

    return SUCCESS;
}

list_status GenerateGraph(List* list) {
    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    char filename_graph[MAX_LEN_NAME] = {};
    snprintf(filename_graph, MAX_LEN_NAME, "%s/graphes/graph%d.txt", list->directory, list->num_dump);

    FILE* graph = fopen(filename_graph, "w");
    if (graph == NULL)
        LIST_CHECK_AND_RETURN_ERRORS(OPEN_ERROR);

    fprintf(graph, "digraph {\n");
    fprintf(graph, "    splines=ortho;\n");

    fprintf(graph, "    node0 [shape = Mrecord; style = filled; fillcolor = \"#00FFFF\"; label = \"{idx = 0 | canary = %d | head = %d | tail = %d}\"];\n",
                    list->data[0], list->next[0], list->prev[0]);

    for (size_t i = 1; i <= list->capacity; ++i) {
        if ((list->next[i] == 0 && list->prev[i] != -1))
            fprintf(graph, "    node%zu [shape = Mrecord; style = filled; fillcolor = \"#99FF99\"; label = <{idx = %zu | data = %d | <FONT COLOR=\"Blue\">next = %d </FONT> | prev = %d}>];\n",
                    i, i, list->data[i], list->next[i], list->prev[i]);
        else if (list->prev[i] == 0)
            fprintf(graph, "    node%zu [shape = Mrecord; style = filled; fillcolor = \"#99FF99\"; label = <{idx = %zu | data = %d |next = %d |  <FONT COLOR=\"Red\">prev = %d</FONT>}>];\n",
                    i, i, list->data[i], list->next[i], list->prev[i]);
        else if (list->prev[i] != -1)
            fprintf(graph, "    node%zu [shape = Mrecord; style = filled; fillcolor = \"#99FF99\"; label = \"{idx = %zu | data = %d | next = %d | prev = %d}\"];\n",
                    i, i, list->data[i], list->next[i], list->prev[i]);
        else
            fprintf(graph, "    node%zu [shape = Mrecord; style = filled; fillcolor = \"#9999FF\"; label = \"{idx = %zu | data = %d | next = %d | prev = %d}\"];\n",
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


    for (type_t i = 0; list->next[i] != 0; i = list->next[i]) {
        fprintf(graph, "    node%d -> node%d [color = blue, arrowhead = vee];\n", i, list->next[i]);
    }
    fprintf(graph, "    node%d -> node0 [color = blue, arrowhead = vee];\n", list->prev[0]);

    for (type_t i = 0; list->prev[i] != 0; i = list->prev[i]) {
        fprintf(graph, "    node%d -> node%d [color = red, arrowhead = vee];\n", i, list->prev[i]);
    }
    fprintf(graph, "    node%d -> node0 [color = red, arrowhead = vee];\n", list->next[0]);

    for (type_t i = (type_t)list->free; list->next[i] != 0; i = list->next[i]) {
        fprintf(graph, "    node%d -> node%d [color = purple, arrowhead = vee];\n", i, list->next[i]);
    }

    fprintf(graph, "    node_head [shape = component, style = filled; fillcolor = \"#FFFF99\"; label = \"Head\"];\n");
    fprintf(graph, "    node_tail [shape = component, style = filled; fillcolor = \"#FFFF99\"; label = \"Tail\"];\n");
    fprintf(graph, "    node_free [shape = component, style = filled; fillcolor = \"#FFFF99\"; label = \"Free\"];\n");

    fprintf(graph, "    node_head -> node%d [color = orange, arrowhead = vee];\n", list->next[0]);
    fprintf(graph, "    node_tail -> node%d [color = orange, arrowhead = vee];\n", list->prev[0]);
    fprintf(graph, "    node_free -> node%zu [color = orange, arrowhead = vee];\n", list->free);

    fprintf(graph, "}");

    if (fclose(graph) == EOF) {
        LIST_CHECK_AND_RETURN_ERRORS(CLOSE_ERROR,    perror("Error is:"));
    }

    return SUCCESS;
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
