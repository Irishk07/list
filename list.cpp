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

    list->data = (type_t*)calloc(RealSizeList(list->capacity, CNT_CANARIES), sizeof(type_t));
    list->next = (type_t*)calloc((size_t)list->capacity + 1, sizeof(type_t)); // +1 because list->next[0] == head, not elem
    list->prev = (type_t*)calloc((size_t)list->capacity + 1, sizeof(type_t)); // +1 because list->prev[0] == tail, not elem

    if (list->data == NULL || list->next == NULL || list->prev == NULL)
        LIST_CHECK_AND_RETURN_ERRORS(NOT_ENOUGH_MEMORY,     free(list->data);
                                                            free(list->next);
                                                            free(list->prev););

    InitWithPoisons(list->data + OffsetDueCanaries(CNT_CANARIES), list->capacity);
    SettingCanariesToBegin(list->data);
    SettingCanariesToEnd(list->data, list->capacity);

    InitNextPrev(list);

    FILE* file = fopen(dump_filename, "w");
    if (file == NULL)
        LIST_CHECK_AND_RETURN_ERRORS(OPEN_ERROR,     free(list->data);
                                                     free(list->next);
                                                     free(list->prev););
    list->file = file;

    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list),   free(list->data);
                                                     free(list->next);
                                                     free(list->prev);
                                                     fclose(file););

    return SUCCESS;
}

void InitNextPrev(List* list) {
    for (type_t i = (type_t)list->free; i <= (type_t)list->capacity; ++i) {
        list->next[i] = i + 1;
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

    if (list->file == NULL)             return NULL_POITER_ON_DUMP_FILE;

    if (list->data[0] != CANARY ||
        list->data[OffsetToNewElement(list->capacity, CNT_CANARIES)] != CANARY) return CORRUPTED_CANARY;

    for (size_t i = list->free; list->next[i] != 0; i = (size_t)list->next[i]) {
        if (list->data[i] != DEFAULT_POISON ||
            list->prev[i] != -1)        return LIST_DATA_POISON;
    }

    if (ListHead(list) < 0 ||
        ListHead(list) >= (type_t)list->capacity)  return INVALIDE_HEAD;

    if (ListTail(list) < 0 ||
        ListTail(list) >= (type_t)list->capacity)  return INVALIDE_TAIL;

    return SUCCESS;
}

// need insert element after position
list_status InsertElement(List* list, type_t elem, size_t position) { // TODO rename position maybe
    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    list->about_elem.position = position;
    list->about_elem.value    = elem;

    if (list->size >= list->capacity) {
        size_t old_capacity = list->capacity;
        list->capacity *= REALLOC_COEFF;

        LIST_CHECK_AND_RETURN_ERRORS(ListResize(list, old_capacity));
    }

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, "Before", DUMP_INFO))

    if (position > list->capacity || position > list->size)
        LIST_CHECK_AND_RETURN_ERRORS(INVALID_POSITION);

    list->data[list->free] = elem;

    type_t current_free = (type_t)list->free;
    list->free = (size_t)list->next[list->free];

    list->prev[list->next[position]] = current_free;
    list->prev[current_free] = (type_t)position;

    list->next[current_free] = list->next[position];
    list->next[position] = current_free;

    list->size++;

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, "After", DUMP_INFO));

    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    return SUCCESS;
}

list_status ListResize(List* list, size_t old_capacity) {
    assert(list);

    list->free = OffsetToNewElement(list->size, CNT_CANARIES);

    type_t* temp_data = (type_t*)my_recalloc(list->data, RealSizeList(list->capacity, CNT_CANARIES) * sizeof(type_t),
                                                         RealSizeList(old_capacity, CNT_CANARIES) * sizeof(type_t));
    type_t* temp_next = (type_t*)my_recalloc(list->next, (list->capacity + 1) * sizeof(type_t), old_capacity * sizeof(type_t)); // +1 because list->next[0] == head, not elem
    type_t* temp_prev = (type_t*)my_recalloc(list->prev, (list->capacity + 1) * sizeof(type_t), old_capacity * sizeof(type_t)); // +1 because list->prev[0] == tail, not elem

    if (temp_data == NULL || temp_next == NULL || temp_prev == NULL) {
        LIST_CHECK_AND_RETURN_ERRORS(NOT_ENOUGH_MEMORY, free(temp_data);
                                                        free(temp_next);
                                                        free(temp_prev));
    }

    list->data = temp_data;
    list->next = temp_next;
    list->prev = temp_prev;

    InitWithPoisons(list->data + OffsetToNewElement(list->size, CNT_CANARIES), list->capacity - list->size);
    SettingCanariesToEnd(list->data, list->capacity);
    InitNextPrev(list);

    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    return SUCCESS;
}

// need delete element from position
list_status DeleteElement(List* list, size_t position) {
    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    list->about_elem.position = position;
    LIST_CHECK_AND_RETURN_ERRORS(GetElement(list, position, &list->about_elem.value));

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, "Before", DUMP_INFO));

    if (position > list->capacity)
        LIST_CHECK_AND_RETURN_ERRORS(INVALID_POSITION);

    if (list->size == 0 || list->prev[position] == -1)
        LIST_CHECK_AND_RETURN_ERRORS(NOT_EXISTENS_ELEMENT);

    list->data[position] = DEFAULT_POISON;

    list->next[list->prev[position]] = list->next[position];
    list->prev[list->next[position]] = list->prev[position];

    list->prev[position] = -1;
    list->next[position] = (type_t)list->free;

    list->free = position;

    LIST_CHECK_AND_RETURN_ERRORS(ListHTMLDump(list, "After", DUMP_INFO));

    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    return SUCCESS;
}

list_status GetElement(List* list, size_t position, type_t* elem) {
    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list))

    if (position >= list->capacity)
        LIST_CHECK_AND_RETURN_ERRORS(INVALID_POSITION);

    if (list->prev[position] == -1)
        LIST_CHECK_AND_RETURN_ERRORS(NOT_EXISTENS_ELEMENT);

    *elem = list->data[position];

    return SUCCESS;
}

type_t ListHead(List* list) {
    return list->next[0];
}

type_t ListTail(List* list) {
    return list->prev[0];
}

list_status ListHTMLDump(List* list, const char* type_dump, int line, const char* func, const char* file) {
    fprintf(list->file, "<pre>\n");

    if (strcmp(func, "InsertElement") == 0)
        fprintf(list->file, "<h3> DUMP <font color=green> %s %s <%d> after position [%zu] </font> </h3>\n",
                type_dump, func, list->about_elem.value, list->about_elem.position);
    else if (strcmp(func, "DeleteElement") == 0)
        fprintf(list->file, "<h3> DUMP <font color=red> %s %s <%d> from position [%zu] </font> </h3>\n",
                type_dump, func, list->about_elem.value, list->about_elem.position);

    fprintf(list->file, "List {%s: %d}\n", file, line);

    fprintf(list->file, "Capacity: %zu\n", list->capacity);
    fprintf(list->file, "Size: %zu\n", list->size);
    fprintf(list->file, "Free: %zu\n", list->free);

    fprintf(list->file, "Indexes |");

    fprintf(list->file, "    Service    | ");
    for (size_t i = 1; i <= list->capacity; ++i) {
        fprintf(list->file, "%4zu | ", i);
    }
    fprintf(list->file, "   Service\n");

    fprintf(list->file, "Data    |");
    fprintf(list->file, " %4d (Canary) | ", list->data[0]);
    for (size_t i = 1; i <= list->capacity; ++i) {
        fprintf(list->file, "%4d | ", list->data[i]);
    }
    fprintf(list->file, "%4d (Canary)\n", list->data[list->capacity + 1]);

    fprintf(list->file, "Next    |");
    fprintf(list->file, " %4d (Head)   | ", list->next[0]);
    for (size_t i = 1; i <= list->capacity; ++i) {
        fprintf(list->file, "%4d | ", list->next[i]);
    }
    fprintf(list->file, "     -     \n");

    fprintf(list->file, "Prev    |");
    fprintf(list->file, " %4d (Tail)   | ", list->prev[0]);
    for (size_t i = 1; i <= list->capacity; ++i) {
        fprintf(list->file, "%4d | ", list->prev[i]);
    }
    fprintf(list->file, "     -     \n");

    fprintf(list->file, "\n");

    LIST_CHECK_AND_RETURN_ERRORS(GenerateGraph(list));

    char command[MAX_LEN_NAME] = {};
    sprintf(command, "dot %s/graph%d.txt -T png -o %s/image%d.png", list->directory, list->num_dump, list->directory, list->num_dump);
    if (system((const char*)command) != 0)
        LIST_CHECK_AND_RETURN_ERRORS(EXECUTION_FAILED,      fprintf(list->file, "Error with create image:(\n"));

    fprintf(list->file, "<img src = %s/image%d.png width = 500px>", list->directory, list->num_dump);

    fprintf(list->file, "\n");

    list->num_dump++;

    return SUCCESS;
}

list_status GenerateGraph(List* list) {
    LIST_CHECK_AND_RETURN_ERRORS(ListVerify(list));

    char filename_graph[MAX_LEN_NAME] = {};
    sprintf(filename_graph, "%s/graph%d.txt", list->directory, list->num_dump);

    FILE* graph = fopen(filename_graph, "w");
    if (graph == NULL)
        LIST_CHECK_AND_RETURN_ERRORS(OPEN_ERROR);

    fprintf(graph, "digraph {\n");
    fprintf(graph, "    rankdir = LR;\n");
    fprintf(graph, "    splines=ortho;\n");
    fprintf(graph, "    nodesep=0.5;\n");

    fprintf(graph, "    node0 [shape = Mrecord; style = filled; fillcolor = \"#00FFFF\"; label = \"canary = %d | head = %d | tail = %d\"];\n",
                    list->data[0], list->next[0], list->prev[0]);

    for (size_t i = 1; i <= (size_t)list->capacity; ++i) {
        if (list->prev[i] != -1)
            fprintf(graph, "    node%zu [shape = Mrecord; style = filled; fillcolor = \"#99FF99\"; label = \"data = %d | next = %d | prev = %d\"];\n",
                    i, list->data[i], list->next[i], list->prev[i]);
        else
            fprintf(graph, "    node%zu [shape = Mrecord; style = filled; fillcolor = \"#9999FF\"; label = \"data = %d | next = %d | prev = %d\"];\n",
                    i, list->data[i], list->next[i], list->prev[i]);
    }

    fprintf(graph, "    node%zu [shape = Mrecord; style = filled; fillcolor = \"#00FFFF\"; label = \"canary = %d\"];\n",
                    list->capacity + 1, list->data[list->capacity + 1]);

    for (size_t i = 0; i < RealSizeList(list->capacity, CNT_CANARIES) - 1; ++i) {
        fprintf(graph, "    node%zu -> node%zu [style = invis];\n", i, i + 1);
    }

    for (type_t i = list->next[0]; list->next[i] != 0; i = list->next[i]) {
        fprintf(graph, "    node%d -> node%d [constraint=false, color = blue];\n", i, list->next[i]);
    }

    for (type_t i = list->prev[0]; list->prev[i] != 0; i = list->prev[i]) {
        fprintf(graph, "    node%d -> node%d [constraint=false, color = red];\n", i, list->prev[i]);
    }

    for (type_t i = (type_t)list->free; list->next[i] != 0; i = list->next[i]) {
        fprintf(graph, "    node%d -> node%d [constraint=false, color = purple];\n", i, list->next[i]);
    }

    fprintf(graph, "    node_head [shape = component, style = filled; fillcolor = \"#FFFF99\"; label = \"Head\"];\n");
    fprintf(graph, "    {rank = same; node%d; node_head;}\n", list->next[0]);
    fprintf(graph, "    node_head -> node%d [constraint=false, color = orange];\n", list->next[0]);

    fprintf(graph, "    node_tail [shape = component, style = filled; fillcolor = \"#FFFF99\"; label = \"Tail\"];\n");
    fprintf(graph, "    {rank = same; node%d; node_tail;}\n", list->prev[0]);
    fprintf(graph, "    node_tail -> node%d [constraint=false, color = orange];\n", list->prev[0]);

    fprintf(graph, "    node_free [shape = component, style = filled; fillcolor = \"#FFFF99\"; label = \"Free\"];\n");
    fprintf(graph, "    {rank = same; node%zu; node_free;}\n", list->free);
    fprintf(graph, "    node_free -> node%zu [constraint=false, color = orange];\n", list->free);

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

    if (fclose(list->file) == EOF) {
        LIST_CHECK_AND_RETURN_ERRORS(CLOSE_ERROR,    perror("Error is:"));
    }

    *list = {};

    return code_error;
}
