#ifndef JNPR
#define JNPR
#include <stdio.h>//printf, perror, fileno
#include <stdlib.h>
#include <string.h>
#define JNPR_XPATH_SEPARATOR '/'
#define JNPR_IS_DIGIT(x) ((x) >= 48 && (x) < 58)
#define JNPR_IS_EMPTY(x) ((x) > 0 && (x) <= 32)
#define JNPR_CHARS_SPECIAL "{}[]:,\"\\"
#define JNPR_PROPERTY_MAX_LENGTH 512
#define JNPR_FLAG_STRICT_START 1
#define JNPR_JSON_TYPE_STRING "string"
#define JNPR_JSON_TYPE_NUMBER "number"
#define JNPR_JSON_TYPE_OBJECT "object"
#define JNPR_JSON_TYPE_ARRAY "array"
#define JNPR_JSON_TYPE_BOOLEAN "boolean"
#define JNPR_JSON_TYPE_NULL "null"
#define JNPR_JSON_BOOLEAN_TRUE "true"
#define JNPR_JSON_BOOLEAN_FALSE "false"
//#define JSON_STAT_RESULT_PATTERN "{\"offset\": 0, \"length\": 0, \"type\": null}"
//#define JSON_STAT_PATTERN "{\"success\": %s, \"result\": %s, \"error\": %s, \"length\": %zu, \"chars_parsed\": %zu, \"chars_parsed_percent\": %s, \"xpath\":\"%s\"}"
typedef struct {
    char success;
    const char* result_type;
    size_t result_offset, result_length, chars_parsed;
    const char* xpath;
} json_parser_stat_t;
const char* dig_to_value_of(const char* property, const char* data, int flags, const char** parsed){
    const char* d = data;
    long square_level = 0, qurve_level = 0, qurve_delta = 0,
    square_delta = 0,
    value_sector = 0, string_sector = 0,
    property_len = strlen(property),
    target_sector = 0;
    if (flags & JNPR_FLAG_STRICT_START){
        if (d[0] != JNPR_CHARS_SPECIAL[0]){
            *parsed = d;
            return 0;
        }
    }
    const char* p0 = d;
    while(*d && d[property_len]){
        p0 = d;
        square_delta = 0;
        qurve_delta = 0;
        if(!string_sector){
            if (*d == JNPR_CHARS_SPECIAL[0]){
                qurve_level++;
                qurve_delta = 1;
            }else if(*d == JNPR_CHARS_SPECIAL[1]){
                qurve_level--;
                qurve_delta = -1;
            }else if(*d == JNPR_CHARS_SPECIAL[2]){
                square_level++;
                square_delta = 1;
            }else if(*d == JNPR_CHARS_SPECIAL[3]){
                square_level--;
                square_delta = -1;
            }
        }
        if(target_sector && value_sector && !JNPR_IS_EMPTY(*d)) {
            *parsed = d;
            return d;
        }
        if(square_level == 0 && qurve_level == 1){
            if (*d == JNPR_CHARS_SPECIAL[4]){
                value_sector = 1;
            }else if(*d == JNPR_CHARS_SPECIAL[5]) {
                value_sector = 0;
            }else if(*d == JNPR_CHARS_SPECIAL[6] && d[-1] != JNPR_CHARS_SPECIAL[7]) {
                string_sector = !string_sector;
            }else if (string_sector
                && !value_sector
                && d[-1] == JNPR_CHARS_SPECIAL[6]
                && d[-2] != JNPR_CHARS_SPECIAL[7]
                && d[property_len] == JNPR_CHARS_SPECIAL[6]
                && d == strstr(d, property) ) {
                
                d += property_len;
                target_sector = 1;
                string_sector = !string_sector;
            }
        }
        if (square_level < 0 || qurve_level < 0){
            break;
        }
        d++;
    }
    *parsed = d;
    return 0;
}
const char* dig_to_array_item(long index, const char* data, int flags, const char** parsed){
    const char* d = data;
    long square_level = 0, qurve_level = 0, qurve_delta = 0, square_delta = 0, i = index, string_sector = 0, empty = 1;
    if (flags & JNPR_FLAG_STRICT_START){
        if (d[0] != JNPR_CHARS_SPECIAL[2]){
            *parsed = d;
            return 0;
        }
        d++;
    }
    
    while(*d){
        square_delta = 0;
        qurve_delta = 0;
        empty = 1;
        if (!string_sector){
            if (*d == JNPR_CHARS_SPECIAL[0]){
                qurve_level++;
                qurve_delta = 1;
                empty = 0;
            }else if(*d == JNPR_CHARS_SPECIAL[1]){
                qurve_level--;
                qurve_delta = -1;
                empty = 0;
            }else if(*d == JNPR_CHARS_SPECIAL[2]){
                square_level++;
                square_delta = 1;
                empty = 0;
            }else if(*d == JNPR_CHARS_SPECIAL[3]){
                square_level--;
                square_delta = -1;
                empty = 0;
            }
        }
        //CHECK "
        if(*d == JNPR_CHARS_SPECIAL[6] && d[-1] != JNPR_CHARS_SPECIAL[7]){
            string_sector = !string_sector;
        }
        if(*d == JNPR_CHARS_SPECIAL[5] && !square_level && !qurve_level && !string_sector){
            i--;
        }else if(i == 0 && !JNPR_IS_EMPTY(*d)){
            *parsed = d;
            return d;
        }else if (i < 0 || square_level < 0){
            break;
        }
        d++;
    }
    *parsed = d;
    return 0;
}
long read_digit(const char** data){
    long base = 1, result = 0;
    const char* d = *data;
    const char* initial = d;
    if(!JNPR_IS_DIGIT(d[0]))return result;
    do{d++;}while(JNPR_IS_DIGIT(d[0]));
    *data = d;
    do{
        d--;
        result += (d[0] - 48) * base;
        base *= 10;
    }while(initial != d);
    
    return result;
}
int read_string(const char** data, const char terminator, char* out){
    int len = 0;
    const char* d = *data;
    //printf("\n");
    while(*d != terminator && *d != 0){
        //printf("read_string %02x '%c'\n",*d, *d);
        *out = *d;
        out++;
        d++;
    }
    *data = d;
    return len;
}
const char* dig_to_the_end_of(const char* data, const char** parsed){
    const char* d = data;
    long square_level = 0, qurve_level = 0, string_sector = 0;
    char terminator_is_empty_char = 0;
    char terminator;
    if (*d == JNPR_CHARS_SPECIAL[2]){
        square_level++;
        terminator = JNPR_CHARS_SPECIAL[3];
    }else if(*d == JNPR_CHARS_SPECIAL[6]){
        string_sector++;
        terminator = JNPR_CHARS_SPECIAL[6];
    }else if(*d == JNPR_CHARS_SPECIAL[0]){
        qurve_level++;
        terminator = JNPR_CHARS_SPECIAL[1];
    }else {
        terminator_is_empty_char = 1;
    }
    if (!terminator_is_empty_char){
        d++;
    }
    while(*d){
        if(!string_sector){
            if (*d == JNPR_CHARS_SPECIAL[0]){
                qurve_level++;
            }else if(*d == JNPR_CHARS_SPECIAL[1]){
                qurve_level--;
            }else if(*d == JNPR_CHARS_SPECIAL[2]){
                square_level++;
            }else if(*d == JNPR_CHARS_SPECIAL[3]){
                square_level--;
            }
        }
        if(*d == JNPR_CHARS_SPECIAL[6] && d[-1] != JNPR_CHARS_SPECIAL[7]) {
            string_sector = !string_sector;
        }
        if(terminator_is_empty_char){
            if(!string_sector && !square_level && !qurve_level && (*d == JNPR_CHARS_SPECIAL[5] || JNPR_IS_EMPTY(*d))){
                *parsed = d;
                return d - 1;
            }
        }else{
            if(*d == terminator && !string_sector && !square_level && !qurve_level){
                *parsed = d;
                return d;
            }
        }
        
        d++;
    }
    *parsed = d;
    return 0;
}

char* json_parse(const char* xpath, const char* data, json_parser_stat_t* stat){
    const char* parsed = data;
    long xpath_len = strlen(xpath);
    json_parser_stat_t st = {};
    if (!stat){
        stat = &st;
    }
    const char* xp = xpath;
    const char* d = data;
    char key[JNPR_PROPERTY_MAX_LENGTH] = {};
    while(*xp){
        if(*xp != JNPR_XPATH_SEPARATOR){
            if(JNPR_IS_DIGIT(xp[0])){
                //INDEX
                long i = read_digit(&xp);
                //FIND ITEM AT INDEX
                d = dig_to_array_item(i, d, JNPR_FLAG_STRICT_START, &parsed);
                if(!d || !d[0]){
                    stat->success = 0;
                    stat->chars_parsed = (size_t)(parsed - data);
                    return 0;
                }
            }else{
                //PROPERTY NAME
                long len = read_string(&xp, JNPR_XPATH_SEPARATOR, (char*)key);
                //FIND PROPERTY NAME
                d = dig_to_value_of(key, d, JNPR_FLAG_STRICT_START, &parsed);
                if(!d || !d[0]){
                    stat->success = 0;
                    stat->chars_parsed = (size_t)(parsed - data);
                    return 0;
                }
                //FREE KEY NAME
                for(int i = 0; i < 512; i++){
                    if (key[i] == 0)
                        break;
                    key[i] = 0;
                }
            }
            if (!xp[0]){
                break;
            }
        }
        xp++;
    }
    stat->chars_parsed = (size_t)(parsed - data);
    if (!d || !d[0]){
        stat->success = 0;
        return 0 ;
    }
    
    if (d[0] == JNPR_CHARS_SPECIAL[6]){
        const char* src = d + 1;
        stat->result_offset = (size_t)(d - data);
        stat->result_type = JNPR_JSON_TYPE_STRING;
        d = dig_to_the_end_of(d, &parsed);
        stat->chars_parsed = (size_t)(parsed - data);
        if(!d){
            stat->success = 0;
            return 0;
        }
        
        size_t len = (size_t)(d - src);
        char* dst = calloc(len + 1, 1);
        memcpy(dst, src, len);
        stat->success = 1;
        stat->result_length = len + 2;//include 2 quots
        
        return dst;
    }else{
        const char* src = d;
        stat->result_offset = (size_t)(d - data);
        if (*d == JNPR_CHARS_SPECIAL[0]){
            stat->result_type = JNPR_JSON_TYPE_OBJECT;
        }else if (*d == JNPR_CHARS_SPECIAL[2]){
            stat->result_type = JNPR_JSON_TYPE_ARRAY;
        }else if (JNPR_IS_DIGIT(*d)){
            stat->result_type = JNPR_JSON_TYPE_NUMBER;
        }else if (*d == 't' || *d == 'f') {
            stat->result_type = JNPR_JSON_TYPE_BOOLEAN;
        }else {
            stat->result_type = JNPR_JSON_TYPE_NULL;
        }
        
        d = dig_to_the_end_of(d, &parsed);
        stat->chars_parsed = (size_t)(parsed - data);
        if(!d){
            stat->success = 0;
            return 0;
        }
        size_t len = (size_t)(d - src + 1);
        char* dst = calloc(len + 1, 1);
        memcpy(dst, src, len);
        stat->success = 1;
        stat->result_length = len;
        return dst;
    }
}
#endif
