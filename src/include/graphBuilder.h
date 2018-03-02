#ifndef _GRAPHBUILDER_H
#define _GRAPHBUILDER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct parse_graph_t;
struct mimo_t;

/**
 * Add a file to the list to be parsed
 * */
void pg_add_file(const char* fname);

/**
 * Set the string (from the commandline) to be parsed
 */
void pg_set_cmdstring(const char* str);


/**
 * Returns 1 on success, 0 on error
 */
int pg_parse(void);


/**
 * Compiles the parsed AST
 */
parse_graph_t* pg_buildGraph(mimo_t* mimo);

/**
 * Call when done to cleanup memory
 */
void pg_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // _GRAPHBUILDER_H
