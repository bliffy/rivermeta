#ifndef _WSMAN_WORD_WRAP_H
#define _WSMAN_WORD_WRAP_H


#define WRAP_WIDTH 80


//structs
typedef struct word_t {
     const char *s;
     int len;
} *word;


// prototypes
void greedy_wrap(word words, int count, int cols, int *breaks);
word make_word_list(const char * s, int *n);
void print_wrap(FILE *fp, const char * src, int width, int tab);
void show_wrap(FILE *fp, word list, int count, int *breaks, int tab);


word make_word_list(const char *s, int *n) {
     int max_n = 0;
     word words = 0;
     *n = 0;
     while (1) {
          while (*s && isspace(*s)) {
               // if we see a newline, create a fake word of length zero
               if (*s == '\n') {
                    if (*n >= max_n) {
                         if (!(max_n *= 2)) max_n = 2;
                         words = (word) realloc(words, max_n * sizeof(*words));
                         if (!words) {
                              error_print("failed make_word_list realloc of words");
                              return NULL;
                         }
                    }
                    words[*n].s = "";
                    words[*n].len = 0;
                    s++;
                    (*n)++;
                    continue;
               }
               // if we see a tab, create a fake word of four spaces
               if (*s == '\t') {
                    if (*n >= max_n) {
                         if (!(max_n *=2)) max_n = 2;
                         words = (word) realloc(words, max_n * sizeof(*words));
                         if (!words) {
                              error_print("failed make_word_list realloc of words");
                              return NULL;
                         }
                    }
                    words[*n].s = "    ";
                    words[*n].len = 4;
                    s++;
                    (*n)++;
                    continue;
               }
               s++;
          };

          if (!*s) break;

          if (*n >= max_n) {
               if (!(max_n *= 2)) max_n = 2;
               words = (word) realloc(words, max_n * sizeof(*words));
               if (!words) {
                    error_print("failed make_word_list realloc of words");
                    return NULL;
               }
          }
          words[*n].s = s;

          while (*s && !isspace(*s)) s++;
          
          words[*n].len = s - words[*n].s;
          (*n) ++;
     }
     return words;
}

void greedy_wrap(word words, int count, int cols, int *breaks) {
     int line, i, j;
     i = j = line = 0;
     while (1) {
          if (i == count) {
               breaks[j++] = i;
               break;
          }
          if (words[i].len == 0) {
               breaks[j++] = i;
               i++;
               line = 0;
               continue;
          }
          if (!line) {
               line = words[i++].len;
               continue;
          }
          if (line + words[i].len < cols) {
               line += words[i++].len + 1;
               continue;
          }
          breaks[j++] = i;
//          if (i < count) {
//               d = cols - line;
//          }
          line = 0;
     }
     breaks[j++] = 0;
}

void show_wrap(FILE *fp, word list, int count, int *breaks, int tab) {
     int i, j, ctab;
     for (i = j = 0; i < count && breaks[i]; i++) {
          ctab = tab;
          while (ctab) {
               fputc(' ', fp);
               ctab--;
          }
          while (j < breaks[i]) {
               // if this is a "real" word, (not a newline) print it
               if (list[j].len != 0) {
                    fprintf(fp, "%.*s", list[j].len, list[j].s);
                    if (j < breaks[i] - 1)
                         fputc(' ', fp);
               }
               j++;
          }
          if (breaks[i]) fputc('\n', fp);
     }
}

void print_wrap(FILE *fp, const char * src, int width, int tab) {

     if (!src) return; 

     int len;
     word list = make_word_list(src, &len);
     int *breaks = (int*) malloc(sizeof(int) * (len + 1));
     if (!breaks) {
          error_print("failed print_wrap malloc of breaks");
          return;
     }
     greedy_wrap(list, len, width-tab, breaks);
     show_wrap(fp, list, len, breaks, tab);
     free(breaks);
}

#endif // _WSMAN_WORD_WRAP_H
