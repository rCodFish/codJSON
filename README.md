# ===**WIP**=== codJSON ===**WIP**=== 

<div align="center">
    <img src="resources/codJSON.png" width="300" height="300">
</div>

---

A lightweight JSON query parser for C.

**codJSON** is a simple, lightweight JSON parser written in C that allows the user to fetch values directly from a JSON file using **query-style paths**.

Its intended to resolve simple queries and not try to model json object types directly. Such intention can be observed in multi type arrays fetching which is not supported, and only single type fetching is supported, provided the type is not another object.

---

## Usage

The functions expect a query string to parse the json. The query expects a **/** for each indentation level the user needs to enter. 


Example:

```C
int main(void) {
    char* query = "example/title";
    char* file = "./example.json";
 
    char* data = codJSON_getString(query, file);

    if (data == NULL) {
        printf("Something went wrong\n");
    } else {
        printf("Data: %s\n", data);
    }
        
    return 0;
}
```

```JSON
{
    "example": {
        "title": "Codfish"
    }  
}
```

```Bash
_> Data: Codfish
```