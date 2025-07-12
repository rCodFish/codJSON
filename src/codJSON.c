#include "codJSON_internal.h"
#include "../include/codJSON.h"

// ===| Main Logic |=================================

// ===
// Parses the query to extract query path tokens and counts
// ===
QueryContext parseQuery(char* query) {
    QueryContext qctx = {0};

    if (!query || strlen(query) == 0) return qctx;

    char* queryCopy = strdup(query);
    if (!queryCopy) return qctx;

    int count     = 1;
    int wildcards = 0;

    for (const char* p = query; *p; p++) {
        if (*p == '/') count++;
        if (*p == '*') wildcards++;
    }

    char** tokens = malloc(sizeof(char*) * (count + 1));
    if (!tokens) {
        free(queryCopy);
        return qctx;
    }

    int i = 0;
    char* token = strtok(queryCopy, "/");
    while (token != NULL) {
        tokens[i++] = strdup(token);
        token       = strtok(NULL, "/");
    }
    tokens[i] = NULL;

    qctx.tokenCount    = i;
    qctx.wildcardCount = wildcards;
    qctx.tokens        = tokens;

    free(queryCopy);

    return qctx;
}

// ===
// Cleanup query context
// ===
void freeQueryContext(QueryContext* qctx) {
    for(int i = 0; i < qctx->tokenCount; i++){
        free(qctx->tokens[i]);
    }
    free(qctx->tokens);
}

// ===
// Uses a query context and returns the first position it finds that match the query
// ===
long jsonSearcher(QueryContext qctx, FILE* fd) {
    ParseState pstat = INITIAL;
    int curTokenIdx = 0;

    char c;
    while ((c = fgetc(fd)) != EOF) {
        char* curToken = qctx.tokens[curTokenIdx];

        switch (pstat) {
        case INITIAL:
            if (c == '{') pstat = VOID;
            break;
            
        case VOID:
            if (c == '"') pstat = KEY;
            if (c == '{') pstat = VOID;
            break;

        case KEY:
            long keyInitPos = ftell(fd) - CUR_STRING_CHAR_OFFSET; 
            int  count      = CUR_STRING_CHAR_OFFSET; 
            char ck;

            while ((ck = fgetc(fd)) != '"' && ck != EOF) count++;

            char* strBuf = malloc(sizeof(char) * (count + 1));
            if (!strBuf) break;

            fseek(fd, keyInitPos, SEEK_SET);

            for(int i = 0; i<count; i++) {
                ck = fgetc(fd);
                strBuf[i] = ck;
            }

            strBuf[count] = '\0';

            if (strcmp(strBuf, curToken) == 0) curTokenIdx++;

            free(strBuf);

            fseek(fd, ftell(fd) + 1, SEEK_SET);

            pstat = KEY_VOID;

            break;

        case KEY_VOID:
            if (c == '"')             pstat = STRING;
            if (c == '[')             pstat = LIST;
            if (c >= '0' && c <= '9') pstat = NUMBER;
            if (c == '{')             pstat = VOID;
            if (c == 't' || c == 'f') pstat = BOOL;

            break;

        case STRING:
            if (curTokenIdx == qctx.tokenCount) {
                long strInitPos = ftell(fd) - CUR_STRING_CHAR_OFFSET;
                return strInitPos;
            }

            if (c == '"') pstat = VOID;

            break;
        
        case NUMBER:
            if (curTokenIdx == qctx.tokenCount) {
                long numInitPos = ftell(fd) - CUR_NUMBER_CHAR_OFFSET;
                return numInitPos;
            } 

            if (c == ',' || c == ' ') pstat = VOID;

            break;

        case BOOL:
            if (curTokenIdx == qctx.tokenCount) {
                long boolInitPos = ftell(fd) - CUR_NUMBER_CHAR_OFFSET;
                return boolInitPos;
            }

            if (c == ',' || c == ' ') pstat = VOID;

            break;
        
        case LIST: 
            if(curTokenIdx == qctx.tokenCount) {
                long listInitPos = ftell(fd) - CUR_STRING_CHAR_OFFSET;
                return listInitPos;
            }

            if (c == ']') pstat = VOID;
            
            break;

        default:
            break;
        }
    }

    return -1;
}

// ===| Data Type Modules |=================================

// ===
// Returns the string value of the specified query
// ===
char* getString(char* query, char* fileName) {
    FILE* fd = fopen(fileName, "r");
    if (fd == NULL) return NULL;

    QueryContext qctx = parseQuery(query);

    long offset = jsonSearcher(qctx, fd);

    int  count = 0;
    char c;

    fseek(fd, offset, SEEK_SET);

    while ((c = fgetc(fd)) != '"' && c != EOF) count++;

    char* strBuf = malloc(sizeof(char) * (count + 1));
    if (!strBuf) return NULL;

    fseek(fd, offset, SEEK_SET);

    for(int i = 0; i<count; i++) {
        c = fgetc(fd);
        strBuf[i] = c;
    }

    strBuf[count] = '\0';

    freeQueryContext(&qctx);

    fclose(fd);

    return strBuf;
}

// ===
// Returns the double value of the specified query
// ===
double* getNumber(char* query, char* fileName) {
    FILE* fd = fopen(fileName, "r");
    if (fd == NULL) return NULL;

    QueryContext qctx = parseQuery(query);

    long offset = jsonSearcher(qctx, fd);
    int  count = 0;
    char c;

    fseek(fd, offset, SEEK_SET);

    while ((c = fgetc(fd)) != EOF) {
        bool isDigit = (c >= '0' && c <= '9');
        bool extraCharsCondition = (c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E');
        
        if (!(isDigit || extraCharsCondition)) break;

        count++;
    }

    // Empty value
    if (count == 0) {
        freeQueryContext(&qctx);
        fclose(fd);
        return NULL;
    }

    char* strBuf = malloc(count + 1);
    if (!strBuf) {
        freeQueryContext(&qctx);
        fclose(fd);
        return NULL;
    }

    fseek(fd, offset, SEEK_SET);
    fread(strBuf, 1, count, fd);
    strBuf[count] = '\0';  

    double* res = malloc(sizeof(double));
    if (!res) {
        free(strBuf);
        freeQueryContext(&qctx);
        fclose(fd);
        return NULL;
    }

    *res = strtod(strBuf, NULL);  

    free(strBuf);
    freeQueryContext(&qctx);
    fclose(fd);

    return res;
}

// ===
// Returns the bool value of the specified query
// ===
bool* getBool(char* query, char* fileName) {
    FILE* fd = fopen(fileName, "r");
    if (fd == NULL) return NULL;

    QueryContext qctx = parseQuery(query);

    long offset = jsonSearcher(qctx, fd);

    int  count = 0;
    char c;

    fseek(fd, offset, SEEK_SET);

    while ((c = fgetc(fd)) != EOF) {
        bool extraCharsCondition = (c == ' ' || c == ',');
        bool endOfFileCondition  = (c == EOF);

        if(extraCharsCondition || endOfFileCondition) break;

        count++;
    } 

    char* strBuf = malloc(sizeof(char) * (count + 1));
    if (!strBuf) return NULL;

    fseek(fd, offset, SEEK_SET);

    for(int i = 0; i<count; i++) {
        c = fgetc(fd);
        strBuf[i] = c;
    }

    strBuf[count] = '\0';

    char* trueStr  = "true";
    char* falseStr = "false";
    bool* res      = malloc(sizeof(bool));

    if (!strcmp(strBuf, trueStr)){
        *res = true;
    } else if (!strcmp(strBuf, falseStr)) {
        *res = false;
    } else {
        printf("string: %s\n", strBuf);
        freeQueryContext(&qctx);
        fclose(fd);
        return NULL;
    }
    
    freeQueryContext(&qctx);

    fclose(fd);

    return res;
}

// ===
// Returns the list with string values of the specified query
// ===
char** getStringList(char* query, char* fileName) {
    FILE* fd = fopen(fileName, "r");
    if (fd == NULL) return NULL;

    QueryContext qctx = parseQuery(query);

    long baseOffset = jsonSearcher(qctx, fd);

    int    strCount   = 0;
    int    commaCount = 0;
    char** list;
    char   c;

    fseek(fd, baseOffset, SEEK_SET);

    // get comma count
    while ((c = fgetc(fd)) != ']' && c != EOF) {
        if(c == ',') commaCount++;
    }

    strCount = commaCount + 1; // the number of items
    list = malloc(sizeof(char*) * strCount + 1); // last pointer will be NULL to serve as \0 for the array
    list[strCount] = NULL;

    fseek(fd, baseOffset, SEEK_SET);

    // for each item in the list count chars and assign mem
    for(int i = 0; i < strCount; i++) {
        int curStrCharCount = 0;   

        while ((c = fgetc(fd)) != '"' && c != EOF) {}                 // consume chars until the first "       ___"

        long strStartPos = ftell(fd);                                 // store string starting pos             ___"x

        while ((c = fgetc(fd)) != '"' && c != EOF) curStrCharCount++; // count string chars until the first "  ___"xxx"

        list[i] = malloc(sizeof(char) * (curStrCharCount + 1));       // +1 for \0     
        if (!list[i]) {                                              
            for (int j = 0; j < i; j++) free(list[j]);
            free(list);
            freeQueryContext(&qctx);
            fclose(fd);
            return NULL;
        }
        list[i][curStrCharCount] = '\0';                              // assign \0
 
        fseek(fd, strStartPos, SEEK_SET);                             // go back to string start               ___"           

        for(int ii = 0; ii < curStrCharCount; ii++) {                 // store the string                      ___"xxx"
            list[i][ii] = fgetc(fd);
        }

        while ((c != ',' || c != ']') && c != EOF) c = fgetc(fd);       // consume chars until the first , or ]  ___"xxx"___, ___"xxx"___]
    }

    freeQueryContext(&qctx);

    fclose(fd);

    return list;
}

// ===| API Wrappers |=================================

char*   codJSON_getString (char* query, char* fileName) {return getString (query, fileName);}
double* codJSON_getNumber (char* query, char* fileName) {return getNumber (query, fileName);}
bool*   codJSON_getBool   (char* query, char* fileName) {return getBool   (query, fileName);}

char** codJSON_getStringList (char* query, char* fileName) {return getStringList (query, fileName);}
