#ifndef COD_JSON_INTERNAL
#define COD_JSON_INTERNAL

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// ===| Defines |==================

#define CUR_STRING_CHAR_OFFSET 1
#define CUR_NUMBER_CHAR_OFFSET 2

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
   NUMBER,
   BOOL
} ParseState;

// ===| Functions |==================

QueryContext parseQuery       (char* query);
void         freeQueryContext (QueryContext* qctx);
long         jsonSearcher     (QueryContext qctx, FILE* fd);

char*   getString (char* query, char* fileName);
double* getNumber (char* query, char* fileName);
bool*   getBool   (char* query, char* fileName);

char** getStringList (char* query, char* fileName);


#endif // COD_JSON_INTERNAL