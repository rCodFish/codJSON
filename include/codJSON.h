#ifndef COD_JSON
#define COD_JSON

#include <stdbool.h>

// ===| Functions > API |==================

char*   codJSON_getString (char* query, char* fileName);
double* codJSON_getNumber (char* query, char* fileName);
bool*   codJSON_getBool   (char* query, char* fileName);

char** codJSON_getStringList (char* query, char* fileName);

#endif // COD_JSON