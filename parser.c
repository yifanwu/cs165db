#include "parser.h"
#include "dbg.h"

/* The following is the size of a buffer to contain any error messages
   encountered when the regular expression is compiled. */

#define MAX_ERROR_MSG 0x1000

// Compile "regex_text" into "r".

int compile_regex(regex_t *r, const char *regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);
    if (status != 0) {
        char error_message[MAX_ERROR_MSG];
        regerror (status, r, error_message, MAX_ERROR_MSG);
        if(V) {printf ("Regex error compiling '%s': %s\n",
            regex_text, error_message);}
            return 1;
    }
    return 0;
}

/*
  Match the string in "to_match" against the compiled regular
  expression in "r".
  input_args is a array of 4 strings with maximum size
 */

  int match_regex (regex_t *r, const char * to_match, char **input_args)
  {
    /* "P" is a pointer into the string which points to the end of the
       previous match. */
    const char * p = to_match;
    // "N_matches" is the maximum number of matches allowed.
    // must be 5 here the first one takes stuff in
    const int n_matches = MAX_ARG_NUM;
    /* "M" contains the matches found. */
    regmatch_t m[n_matches];

    int i = 0;
    int nomatch = regexec (r, p, n_matches, m, 0);
    if (nomatch) {
        log_warn("No matches for string %s.", to_match);
        return nomatch;
    }
    for (i = 0; i < n_matches; i++) {
        int start;
        int finish;
        if (m[i].rm_so == -1) {
          // nothing left
          for (int j=i; j < n_matches; j++) {
            // just to be sure, for comparison
            memset(input_args[j-1], 0, (int)sizeof(input_args[j-1]));
          }
          break;
        }
        start = m[i].rm_so + (p - to_match);
        finish = m[i].rm_eo + (p - to_match);
        if (i > 0) {
          if (finish > start) {
            strncpy(input_args[i-1],&to_match[start], finish - start);
            // end the string
            input_args[i-1][finish - start]=0;
          } else {
            memset(input_args[i-1], 0, (int)sizeof(input_args[i-1]));
          }
      }
      //if (VV) {printf ("No %d: '%.*s' (bytes %d:%d)\n", i, (finish - start),
       //    to_match + start, start, finish);}
    }
    p += m[0].rm_eo;
    return 0;
}
