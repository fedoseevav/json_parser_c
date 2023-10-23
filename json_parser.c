#include <stdio.h>//printf, perror, fileno
#include <stdlib.h>
#include <errno.h> //errno
#include <unistd.h>//isatty
#include "json_parser.h"
#define WHITE "\033[97m"
#define MAGENTA "\033[95m"
#define HEADER "\033[95m"
#define OKBLUE "\033[94m"
#define OKCYAN "\033[96m"
#define OKGREEN "\033[92m"
#define WARNING "\033[93m"
#define FAIL "\033[91m"
#define ENDC "\033[0m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"
typedef struct{
    char verbose;
    const char* xpath, *data, *file;
}args_t;
args_t parse_arguments(int argc, const char** argv){
    args_t args = {};
    int index_arg[3] = {-1, -1, -1};
    for(int i = 1; i < argc; i++){
        const char* arg = argv[i];
        if (!strcmp("-f", arg)){
            index_arg[2] = i + 1;
        }else if (!strcmp("-v", arg)){
            args.verbose = 1;
        }else{
            if (index_arg[0] == -1){
                char found = 0;
                for(int n = 0; n < sizeof(index_arg) / sizeof(int); n++){
                    if (i == index_arg[n]){
                        found = 1;
                    }
                }
                if (!found){
                    index_arg[0] = i;
                }
            }else if (index_arg[1] == -1){
                char found = 0;
                for(int n = 0; n < sizeof(index_arg) / sizeof(int); n++){
                    if (i == index_arg[n]){
                        found = 1;
                    }
                }
                if (!found){
                    index_arg[1] = i;
                }
            }
        }
    }
    for(int i = 0; i < argc; i++){
        if(i == index_arg[0]){
            args.xpath = argv[i];
        }else if(i == index_arg[1]){
            args.data = argv[i];
        }else if(i == index_arg[2]){
            args.file = argv[i];
        }
    }
    
    if (!isatty(0)){
        //We are from the right side of the '|' pipe and must read from stdin
        char buf[1];
        size_t cap_step = 5000000;
        size_t capacity = cap_step;
        char* buffer = (char*)malloc(capacity);
        memset(buffer, 0, capacity);
        size_t i = 0;
        while(read(0, buf, sizeof(buf)) > 0 ) {           
            buffer[i] = buf[0];
            i++;
            if(i > capacity){
                capacity += cap_step;
                buffer = (char*)realloc(buffer, capacity);
                if(!buffer){
                    exit(0);
                }
                memset((void*)((size_t)buffer + capacity - cap_step), 0, cap_step);
            }
        }
        args.data = (const char*)buffer;
    }
    
    if(!args.xpath || (!args.file && !args.data)){
        fprintf(stderr, "%sjson_parser is the utility to extract values from json formatted data.\n", FAIL);
        fprintf(stderr, "There is two nessessary parameters: first one is xpath, second is input string.\n");
        fprintf(stderr, "Note: If input string is preceded by %s-f%s flag, it used as path to the json file%s\n", OKCYAN,FAIL, ENDC);
        fprintf(stderr, "%s-v%s verbose output%s\n", OKCYAN,FAIL, ENDC);
        fprintf(stderr, "Example: %sjson_parser %s/items/0/keys_url%s %s-f%s %sapi_output.txt%s\n", OKCYAN, WHITE, ENDC, OKCYAN, ENDC, WHITE, ENDC);
        fprintf(stderr, "Example: %sjson_parser -v %s/data/3/pizza%s %s{\"name\": \"Alex\", \"type\": \"customer\", \"score\": 6.2, \"data\": [null,1,\"some\",{\"pizza\": \"good\"}]}%s\n", OKCYAN, WHITE, ENDC, WHITE, ENDC);
        exit(0);
    }else if (args.verbose){
        printf("These parameters will be used:\n");
        printf("%sverbose: %s%s%s\n",OKCYAN, WHITE, args.verbose ? "Yes" : "No", ENDC);
        printf("%sxpath: %s%s%s\n",OKCYAN, WHITE, args.xpath, ENDC);
        printf("%sdata: %s%s%s\n",OKCYAN, WHITE, args.data, ENDC);
        printf("%sfile: %s%s%s\n\n",OKCYAN, WHITE, args.file, ENDC);
    }
    return args;
}
long get_file_size(const char* path){
    errno = 0;
    FILE* f = fopen(path, "r");
    if (f == 0) {
        fprintf(stderr, "%sfopen failed (\"%s\")", FAIL, path);
        perror(" ");
        fprintf(stderr, ENDC);
        errno = 0;
        return 0;
    }
    fseek(f, 0, SEEK_END); // seek to end of file
    long size = ftell(f); // get current file pointer
    fclose(f);
    return size;
}
int main(int argc, const char** argv){
    
    args_t args = parse_arguments(argc, argv);
    if (args.file){
        json_parser_stat_t stat = {};
        long file_size = get_file_size(args.file);
        char* data = (char*)calloc((size_t)(file_size + 1), 1);
        FILE* file = fopen(args.file, "r");
        fread(data, (size_t)file_size, 1, file);
        fclose(file);
        char* result = json_parse(args.xpath, data, &stat);
        if (result){
            printf("%s\n", result);
        }
        if(args.verbose){
            printf("\nadditional info:\n");
            printf("%s{\n", OKCYAN);
            printf("    \"success\": %s%s%s,\n", WHITE, stat.success ? "true" : "false" ,OKCYAN);
            printf("    \"result\":");
            if(stat.success){
                printf("\n    {,\n");
                printf("        \"type\": %s\"%s\"%s,\n", WHITE, stat.result_type, OKCYAN);
                printf("        \"offset\": %s%zu%s,\n", WHITE, stat.result_offset, OKCYAN);
                printf("        \"length\": %s%zu%s\n", WHITE, stat.result_length, OKCYAN);
                printf("    },\n");
            }else{
                printf(" %snull%s,\n", WHITE, OKCYAN);
            }
            printf("    \"chars_parsed\": %s%zu%s,\n", WHITE, stat.chars_parsed, OKCYAN);
            printf("    \"chars_parsed_percent\": %s%.2f%%%s,\n", WHITE, 100.0 * (double)stat.chars_parsed / (double)file_size , OKCYAN);
            printf("    \"json_size: %s%zu%s\n", WHITE, file_size,  OKCYAN);
            printf("}%s\n", ENDC);
        }
        free(result);
        free(data);
    }else{
        json_parser_stat_t stat = {};
        char* result = json_parse(args.xpath, args.data, &stat);
        long data_length = strlen(args.data);
        if (result){
            printf("%s\n", result);
        }
        if(args.verbose){
            printf("\nadditional info:\n");
            printf("%s{\n", OKCYAN);
            printf("    \"success\": %s%s%s,\n", WHITE, stat.success ? "true" : "false" ,OKCYAN);
            printf("    \"result\": %s,\n", stat.success ? "{" : "null");
            if(stat.success){
                printf("        \"type\": %s\"%s\"%s,\n", WHITE, stat.result_type, OKCYAN);
                printf("        \"offset\": %s%zu%s,\n", WHITE, stat.result_offset, OKCYAN);
                printf("        \"length\": %s%zu%s\n", WHITE, stat.result_length, OKCYAN);
                printf("    },\n");
            }
            printf("    \"chars_parsed\": %s%zu%s,\n", WHITE, stat.chars_parsed, OKCYAN);
            printf("    \"chars_parsed_percent\": %s%.2f%%%s,\n", WHITE, 100.0 * (double)stat.chars_parsed / (double)data_length , OKCYAN);
            printf("    \"json_size: %s%ld%s\n", WHITE, data_length, OKCYAN);
            printf("}%s\n", ENDC);
        }
        free(result);
    }
    return 0;
}
