#ifndef PARSER_H_
#define PARSER_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>
#include "config.h"

int compile_regex (regex_t *r, const char *regex_text);
int match_regex (regex_t * r, const char * to_match, char **input_args);
#endif  // PARSER_H_
