# json_parser_c

One .h file for json parsing for C
The main goal is to extract target data from json in C 
There is no validating, serializing and deserializing

## It's very easy to use it
```c
const char* json_string = "{\"Error\":{\"Code\":\"AccessDenied\",\"Message\": \"Access Denied\"}}";
const char* result = json_parse("/Error/Code", json_string, 0);
printf("%s\n", result);
```

Output:

```c
AccessDenied
```
## Function and parameters:
#### Signature
```c
const char* json_parse(const char* xpath, const char* data, json_parser_stat_t* stat)
```
#### Params

1.  `xpath` - Xpath to the target value (for example **/path/to/data/3/message**) every part of that path is the property name or item index in array.

2.  `data` - Input json string

3.  `stat` - A structure that provides information about the result type, offset, length, and other parameters. Can be null.

#### Result 
**0** - if it can't find the desired value using such xpath
**pointer to the string** - In success. 
> **Note** This is a newly allocated string (you have to free it yourself), quotes are excluded if it is a string type.

**stat**  - parameter will be updated by fresh data. Using **start.result_offset** and **start.result_length**, you can "see" where the desired value is in the input string, quotes are included if it is a string type.
## License

MIT
