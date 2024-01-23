/*
    -- * Initialize contest folder with .cpp files by some template.
       * Usage:
       *    init <name> [A..Z] <flags>

    -- * flags:
                --custom-dir=<dir> - specify template file.
                    by default using ./template/init.cpp
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/stat.h>

#define exit_help                                                           \
    do {                                                                    \
        fprintf(stderr, "Usage: init name [A..Z] [flags]\n");               \
        fprintf(stderr, "\t--custom-dir=<path> - set custom from file.\n"); \
        exit(-1);                                                           \
    } while (0)

#define _error(msg)           \
    do {                      \
        fprintf(stderr, msg); \
        exit(-1);             \
    } while(0)

typedef char* str;

struct config {
    char * from_path,
         * to_path;
    str  * names;
    char * content;
};

int names_length(struct config* cptr)
{
    str* names = cptr->names;
    int len = 0;
    while (*(names++)) ++len;
    return len;
}

void init(struct config* cptr)
{
    // create contest directory if not exist.
    struct stat s = { 0 };
    if (stat(cptr->to_path, &s) == -1)
        mkdir(cptr->to_path, 0700);

    int count = names_length(cptr);
    // memory leak, but it is faster this way.
    for (int i = 0; i < count; ++i) {
        char * name = (char*)
                      malloc (
                        strlen(cptr->to_path) + 1 \
                      + strlen(cptr->names[i]) + 5
                      );
        sprintf(name, "%s/%s.cpp", cptr->to_path, cptr->names[i]);

        FILE * temp = fopen(name, "a");
        if (temp == NULL) {
            fprintf(stderr, "error, file \"%s\" not created.\n", cptr->names[i]);
            continue;
        }

        fputs(cptr->content, temp);
        fclose(temp);
    }
}

char* read_file(struct config* cptr)
{
    FILE * fptr   = fopen(cptr->from_path, "r");
    char * buffer = NULL;

    if (fptr == NULL)
        _error("error, --custom-dir= file doesn't exist.\n");

    size_t  len;
    ssize_t bytes = getdelim(&buffer, &len, EOF, fptr);

    fclose (fptr);

    if (bytes == -1)
        _error("error, failed to read the template file.\n");

    char* heap_buffer = (char*) malloc (len);
    memcpy(heap_buffer, buffer, len);

    return heap_buffer;
}

char check_content(char* arg)
{
    const char* f_compare = "--custom-dir=";
    for (int i = 0; arg[i] && *f_compare; ++i, ++f_compare) {
        if (*f_compare != arg[i])
            return 0;
    }
    return 1;
}

char* parse_dir(char* arg)
{
    return arg + strlen("--custom-dir=");
}

int main(int argc, char** argv)
{
    if (argc <= 2)
        exit_help;

    // config.names is NULL terminated for future reallocations.
    struct config config = { NULL, NULL, (str*) malloc (1), NULL };
    config.names[0] = NULL;

    // set default to_path with starting ./ ;
    config.to_path = (char*)
                     malloc (
                        strlen(argv[1]) + 3
                     );

    strcat(config.to_path, "./");
    strcpy(config.to_path + 2, argv[1]);

    for (int i = 2; i < argc; ++i) {
        if (config.from_path == NULL && check_content(argv[i])) {
            char* dir = parse_dir(argv[i]);

            config.from_path = (char*)
                               malloc (
                                  strlen(dir) + 1
                               );
            strcpy(config.from_path, dir);
        }
        else {
            if (argv[i][0] == '-')
                exit_help;

            int nlen = names_length(&config);
            config.names = (str*)
                           realloc (
                               config.names,
                               (nlen+2) * sizeof(char*)
                           );

            config.names[nlen]   = argv[i];
            config.names[nlen+1] = NULL;
        }
    }

    if (config.from_path == NULL) {
        config.from_path = (char*)
                           malloc (
                                strlen("./template/init.cpp") + 1
                           );
        strcpy(config.from_path, "./template/init.cpp");
    }

    config.content = read_file(&config);

    init(&config);

    return 0; 
}

