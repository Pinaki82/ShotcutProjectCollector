#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

void free_strings_array(char **array, size_t count);
int parse_project_file(const char *filename, char ***resources, size_t *count);

#endif // PARSER_H
