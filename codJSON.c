#include "codJSON.h"

// ===| Main Logic |=================================

int main(void) {
    char* query = "glossary/title";
    char* file = "./testA.json";
 
    char* data = codJSON_getString(query, file);

    if (data == NULL) {
        printf("Something went wrong\n");
    } else {
        printf("Data: %s\n", data);
    }
        
    return 0;
}

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
            long keyInitPos = ftell(fd) - CUR_CHAR_OFFSET; 
            int  count      = CUR_CHAR_OFFSET; 
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

            break;

        case STRING:
            long strInitPos = ftell(fd) - CUR_CHAR_OFFSET;
            if (curTokenIdx == qctx.tokenCount) return strInitPos;

            if (c == '"') pstat = VOID;

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
char* codJSON_getString(char* query, char* fileName) {
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

