#ifndef COD_JSON
#define COD_JSON

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// ===| Defines |==================

#define CUR_CHAR_OFFSET 1

// ===| Structs/Enums |==================

typedef struct QueryContext {
    int    tokenCount;
    int    wildcardCount;
    char** tokens;
} QueryContext;

typedef enum ParseState {
   KEY,
   VOID,
   INITIAL,
   KEY_VOID,
   STRING,
   LIST,
   NUMBER
} ParseState;

// ===| Functions |==================

QueryContext parseQuery            (char* query);
void         freeQueryContext      (QueryContext* qctx);
long         jsonSearcher          (QueryContext qctx, FILE* fd);
char*        codJSON_getString     (char* query, char* fileName);

#endif // COD_JSON