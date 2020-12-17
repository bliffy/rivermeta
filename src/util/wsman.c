/* program to print out all the help options for each module*/
/* this program makes use of the aliases file for modules */

#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define _WSUTIL
#include "waterslide.h"
#include "waterslidedata.h"
#include "wstypes.h"
#include "wscalc.h"

#include "dl_assist.h"
#include "dir_assist.h"
#include "win_extras.h"

#include "wsman_color.h"
#include "wsman_map.h"
#include "wsman_util.h"
#include "wsman_word_wrap.h"

#ifdef __cplusplus
CPP_OPEN
#endif

int keyword_search = 0;
int check_kid = 0;
int tag_search = 0;
int input_type_search = 0;
int output_type_search = 0;
int verbose = 1;

FILE * outfp;
int is_pager = 0;
map_t * map = NULL;

char * last_kid = NULL;

char errbuf[2048];

int print_module_help(FILE *, const char *);


char * get_module_path(
        const char * name,
        const char * libpath)
{
    size_t dlen   = strlen(libpath);
    size_t modlen = strlen(name);
    size_t prefix = strlen(WS_PROC_MOD_PREFIX);
    size_t suffix = strlen(WS_PROC_MOD_SUFFIX);
    char * fullname = (char *)calloc(
        1, dlen + prefix + modlen + suffix + 1);
    if (!fullname) {
        error_print("failed get_module_path calloc of fullname");
        return NULL;
    }

    memcpy(fullname, libpath, dlen);
    memcpy(fullname + dlen, WS_PROC_MOD_PREFIX, prefix);
    memcpy(fullname + dlen + prefix, name, modlen);
    memcpy(fullname + dlen + prefix + modlen,
           WS_PROC_MOD_SUFFIX, suffix);

    return fullname;
}

char * get_module_path_parallel(
        const char * name,
        const char * libpath)
{
    size_t dlen   = strlen(libpath);
    size_t modlen = strlen(name);
    size_t prefix = strlen(WS_PROC_MOD_PREFIX);
    size_t suffix = strlen(WS_PROC_MOD_PARALLEL_SUFFIX);
    char * fullname = (char *)calloc(
        1, dlen + prefix + modlen + suffix + 1);
    if (!fullname) {
        error_print("failed get_module_path_parallel calloc of fullname");
        return NULL;
    }

    memcpy(fullname, libpath, dlen);
    memcpy(fullname + dlen, WS_PROC_MOD_PREFIX, prefix);
    memcpy(fullname + dlen + prefix, name, modlen);
    memcpy(fullname + dlen + prefix + modlen,
           WS_PROC_MOD_PARALLEL_SUFFIX, suffix);

    return fullname;
}

int search_for_keyword(
        const char * spath,
        const char * keyword)
{
    DLHANDLE modh;
    if (spath == NULL) {
        return 0;
    }
     
    // try to locate the passed module name
    modh = DLOPEN(spath);
    if ( !modh ) {
        DLERROR(errbuf,2047);
        fprintf(stderr,"%s\n", errbuf);
        fprintf(stderr,"unable to open module %s\n", spath);
        return 0;
    }

    const char ** alias;
    const char *  name;
    const char * purpose;
    const char * description;
    const proc_example_t * examples;
    const proc_option_t * options;
    const char * ns_opts;

    alias = (const char**)DLSYM(modh,"proc_alias");
    name = (const char *)DLSYM(modh,"proc_name");
    purpose = (const char *)DLSYM(modh,"proc_purpose");
    description = (const char *)DLSYM(
        modh,"proc_description");
    examples = (const proc_example_t*)DLSYM(
        modh,"proc_examples"); 
    options = (const proc_option_t*)DLSYM(modh,"proc_opts"); 
    ns_opts = (const char*)DLSYM(modh,"proc_nonswitch_opts"); 

    // Search the name, aliases, purpose,
    // description, example fields for a keyword.
    int retval = 0;

    if (name && _strcasestr(name, keyword)) {
        DLCLOSE(modh);
        return 1;
    }

    if (purpose && _strcasestr(purpose, keyword)) {
        DLCLOSE(modh);
        return 1;
    }

    if (alias) {
        for (int i = 0; alias[i]; i++) {
            if (_strcasestr(alias[i], keyword)) {
                DLCLOSE(modh);
                return 1;
            }
        }
    }

    if (description && _strcasestr(description, keyword)) {
        DLCLOSE(modh);
        return 1;
    }

    if (examples) {
        for (int i = 0; examples[i].text; i++) {
            if (_strcasestr(examples[i].text, keyword))
                retval = 1;
            if (_strcasestr(examples[i].description, keyword))
                retval = 1;
        }
        if ( retval==1 ) {
            DLCLOSE(modh);
            return 1;
        }
    }

    if (!options && !ns_opts) {
        DLCLOSE(modh);
        return 1;
    }

    // TODO: this is ugly, needs cleaning
    // for every option we need to compile the whole string
    // together, then search it
    if (ns_opts && strlen(ns_opts) > 0) {
        size_t len = strlen(ns_opts) + 2;
        char * ns = (char *) malloc(len+1);
        if (!ns) {
            error_print("failed search_for_keyword calloc of ns");
            DLCLOSE(modh);
            return 0;
        }
        snprintf(ns, len+1, "<%s>", ns_opts); 
        if (_strcasestr(ns, keyword)) {
            DLCLOSE(modh);
            return 1;
        }
    }
    if (!options) {
        DLCLOSE(modh);
        return 0;
    }

    int i = 0;
    while (options[i].option != ' ') {
        size_t sz = 1024;
        char * buf = (char *) calloc(1,sz);
        char * p = buf;
        int read = 0;

        if (options[i].option != '\0') {
            read = snprintf(p, 4, "[-%c", options[i].option);
        }
        else {
            while ((read+3+strlen(options[i].long_option)+1) > sz) {
                sz += 1024;
                buf = (char *) realloc(buf, sz);
                if (!buf) {
                    error_print("failed opt.long_option realloc"); 
                }
                p = buf+read;
            }
            read = snprintf(
                p,
                3+strlen(options[i].long_option)+1,
                "[--%s",
                options[i].long_option);
        }
        p = p + read;
        if (options[i].argument && strlen(options[i].argument)) {
            while ((read+3+strlen(options[i].argument)+1) > sz) {
                sz += 1024;
                buf = (char *) realloc(buf, sz);
                if (!buf) {
                    error_print("failed opt.argument realloc"); 
                }
                p = buf+read;
            }
            read = snprintf(p,3+strlen(options[i].argument)+1,
                    " <%s>", options[i].argument);
            p = p + read;
        }
        while ((read+2+strlen(options[i].description)+1) > sz) {
            sz += 1024;
            buf = (char *) realloc(buf, sz);
            if (!buf) {
                error_print("failed opt.description realloc"); 
            }
            p = buf+read;
        }

        snprintf(p,2+strlen(options[i].description)+1,
             "] %s", options[i].description);
        if (_strcasestr(p, keyword)) {
            DLCLOSE(modh);
            return 1;
        }
        i++;
    }

    DLCLOSE(modh);
    return 0;
}

// return 1 if this module contains has the tag
int search_for_tag(const char * spath, const char * tag)  {
    DLHANDLE sh_file_handle;
    if (spath == NULL) {
        return 0;
    }
    // try to locate the passed module name
    if ((sh_file_handle = DLOPEN(spath))) {
        const char ** proc_tags = (const char**)DLSYM(
            sh_file_handle,"proc_tags");
        if (proc_tags) {
            int i;
            for (i = 0; proc_tags[i]; i++) {
                // found the tag
                if (!strcasecmp(proc_tags[i], tag)) {
                    return 1;
                }
            }
        }
        DLCLOSE(sh_file_handle);
        return 0;
    } else {
        DLERROR(errbuf,2047);
        fprintf(stderr,"%s\n", errbuf);
        fprintf(stderr,"unable to open module %s\n", spath);
        return 0;
    }
}

// return 1 if this module contains has the input type
int search_for_input_type(
        const char * spath,
        const char * tag)
{
    DLHANDLE sh_file_handle;
    if (spath == NULL) {
        return 0;
    }
    // try to locate the passed module name
    if ((sh_file_handle = DLOPEN(spath))) {

        const char ** proc_input_types;
        // check for procbuffer kid
        const int * is_procbuffer = (const int *)DLSYM(
            sh_file_handle,"is_procbuffer");
        if (is_procbuffer) {
            const char * proc_itypes[] = \
                {"tuple", "monitor", NULL};
            proc_input_types = (const char**)proc_itypes;
        }
        else {
            proc_input_types = (const char**)DLSYM(
                sh_file_handle,"proc_input_types");
        }
        if (proc_input_types) {
            int i;
            for (i = 0; proc_input_types[i]; i++) {
                // found the tag
                if (!strcasecmp(proc_input_types[i], tag)) {
                    return 1;
                }
            }
        }
        DLCLOSE(sh_file_handle);
        return 0;
    } else {
        DLERROR(errbuf,2047);
        fprintf(stderr,"%s\n", errbuf);
        fprintf(stderr,"unable to open module %s\n", spath);
        return 0;
    }
}

// return 1 if this module contains has the output type
int search_for_output_type(
        const char * spath,
        const char * tag)
{
    DLHANDLE sh_file_handle;
    if (spath == NULL) {
        return 0;
    }
    // try to locate the passed module name
    if ((sh_file_handle = DLOPEN(spath))) {
        const char ** proc_output_types;
        // check for procbuffer kid
        const int * is_procbuffer = (const int *)DLSYM(
            sh_file_handle,"is_procbuffer");
        if (is_procbuffer) {
            const char * proc_otypes[] = {"tuple", NULL};
            proc_output_types = (const char**)proc_otypes;
        }
        else {
            proc_output_types = (const char**)DLSYM(
                sh_file_handle,"proc_output_types");
        }
        if (proc_output_types) {
            int i;
            for (i = 0; proc_output_types[i]; i++) {
                // found the tag
                if (!strcasecmp(proc_output_types[i], tag)) {
                    return 1;
                }
            }
        }
        DLCLOSE(sh_file_handle);
        return 0;
    } else {
        DLERROR(errbuf,2047);
        fprintf(stderr,"%s\n", errbuf);
        fprintf(stderr,"unable to open module %s\n", spath);
        return 0;
    }
}

int check_kid_field_single_dereference(
        FILE * fp,
        const char * field_name,
        const char * field_data,
        uint32_t minimum_length) {

    if (!field_data) {
        fprintf(fp, "FAILURE: %s is null or undefined\n", field_name);
    }
    else if (strlen(field_data) < minimum_length) {
        fprintf(fp, "FAILURE: %s is shorter than %d\n", field_name, minimum_length);
    }
    else if (verbose) {
        fprintf(fp, "SUCCESS: %s is defined\n", field_name);
    }
    return 0;
}

int check_kid_field_double_dereference(
        FILE * fp,
        const char * field_name,
        const char ** field_data,
        uint32_t min_elements) {
    if (!field_data) {
        fprintf(fp, "FAILURE: %s is null or undefined\n", field_name);
    }
    else {
        if (verbose) {
            fprintf(fp, "SUCCESS: %s is defined\n", field_name);
        }
        int i = 0;
        while(field_data[i]) {
            if (strlen(field_data[i]) == 0) {
                fprintf(fp, "FAILURE: %s[%d] has a zero-length\n",field_name,i);
            }
            else if (verbose) {
                fprintf(fp, "SUCCESS: %s[%d]\n",field_name,i);
            }
            i++;
        }
        if (i < min_elements) {
            fprintf(fp, "FAILURE: %s had less than %d elements\n",field_name,min_elements);
        }
    }

    return 0;
}


/**
 * check the documentation variables of a kid
 * Show failures by default.  Use -v flag to show successes.
 */
int check_kid_documentation(
        FILE * fp,
        const char * spath,
        const char * kid)  {
    DLHANDLE sh_file_handle;
    //snprintf(spath,255,"%s/module_%s.so", g_pLibPath, modulename);
    if (spath == NULL) { return 0;}
    // try to locate the passed module name
    if ((sh_file_handle = DLOPEN(spath))) {
        if (verbose) { 
            // test each kid field
            fprintf(fp, "SUCCESS: found %s\n", kid);
        }          
          
        // verify that this variable exists 
        const char ** proc_alias  = (const char**)DLSYM(
            sh_file_handle, "proc_alias");
        check_kid_field_double_dereference(
            fp, "proc_alias", proc_alias, 0);

        // verify that this variable exists and has a
        // positive length 
        const char * proc_name    = (const char *)DLSYM(
            sh_file_handle,"proc_name");
        check_kid_field_single_dereference(
            fp, "proc_name", proc_name, 1);

        // verify that this variable exists and has a
        // positive length 
        const char * proc_purpose = (const char *)DLSYM(
            sh_file_handle, "proc_purpose");
        check_kid_field_single_dereference(
            fp, "proc_purpose", proc_purpose, 1);
          
        // verify that this variable exists and has at
        // least one entry 
        const char ** proc_synopsis  = (const char**)DLSYM(
            sh_file_handle,"proc_synopsis");
        check_kid_field_double_dereference(
            fp, "proc_synopsis", proc_synopsis, 1);

        // verify that this variable exists and has a
        // positive length 
        const char * proc_description = (const char *)DLSYM(
            sh_file_handle,"proc_description");
        check_kid_field_single_dereference(
            fp, "proc_description", proc_description, 1);

        // verify that this variable exists and has at
        // least one entry 
        const char ** proc_tags  = (const char**)DLSYM(
            sh_file_handle,"proc_tags");
        check_kid_field_double_dereference(
            fp, "proc_tags", proc_tags, 1);

        // verify that this variable exists and has at
        // least one entry 
        const proc_example_t * examples;
        examples = (const proc_example_t*)DLSYM(
            sh_file_handle,"proc_examples"); 
        if (examples) {
            if (verbose) {
                fprintf(fp, "SUCCESS: proc_examples is defined\n");
            }
               
            int i = 0;
            while (examples[i].text != NULL) {
                if (strlen(examples[i].text) == 0) {
                    fprintf(fp, "FAILURE: proc_examples[%d].text has a zero-length\n",i);
                }
                else if (verbose) {
                    fprintf(fp, "SUCCESS: proc_examples[%d].text has a length\n",i);
                }
                if (examples[i].description == NULL || strlen(examples[i].description) == 0) {
                    fprintf(fp, "FAILURE: proc_examples[%d].description is null or zero-lengthed\n",i);
                }
                else if (verbose) {
                    fprintf(fp, "SUCCESS: proc_examples[%d].description has a length\n",i);
                }
                i++;
            }
            if (i < 1) {
                fprintf(fp, "FAILURE: proc_examples didn't have any examples\n");
            }
        }
        else {
            fprintf(fp, "FAILURE: proc_examples is null or undefined\n"); 
        }

        // verify that this variable exists 
        const char * proc_requires = (const char *)DLSYM(
            sh_file_handle,"proc_requires");
        check_kid_field_single_dereference(
            fp, "proc_requires", proc_requires, 0);

        // verify that this variable exists and has a
        // positive length 
        const char * proc_version = (char *)DLSYM(
            sh_file_handle,"proc_version");
        check_kid_field_single_dereference(
            fp, "proc_version", proc_version, 1);

        // verify that this variable exists and that each
        // entry has the appropriate lengths 
        const proc_option_t * opt = (proc_option_t*)DLSYM(
            sh_file_handle,"proc_opts"); 
        if (opt) {
            if(verbose) {
                fprintf(fp, "SUCCESS: proc_opts is defined\n");
            }

            // option, long_option, argument, description
            int i = 0;
            while (opt[i].option != ' ') {
                if(verbose) {
                    fprintf(fp, "SUCCESS: proc_opts[%d].option\n",i);
                }

                if(opt[i].long_option == NULL) {
                    fprintf(fp, "FAILURE: proc_opts[%d].long_option shouldn't be null\n",i);
                }
                else if (verbose) {
                    fprintf(fp, "SUCCESS: proc_opts[%d].long_option isn't null\n",i);
                }
                if(opt[i].argument == NULL) {
                    fprintf(fp, "FAILURE: proc_opts[%d].argument shouldn't be null\n",i);
                }
                else if (verbose) {
                    fprintf(fp, "SUCCESS: proc_opts[%d].argument isn't null\n",i);
                }
                if(opt[i].description == NULL || strlen(opt[i].description) == 0) {
                    fprintf(fp, "FAILURE: proc_opts[%d].description is null or zero-lengthed\n",i);
                }
                else if (verbose) {
                    fprintf(fp, "SUCCESS: proc_opts[%d].description has a length\n",i);
                }
                if(opt[i].multiple != 0 && opt[i].multiple != 1) {
                    fprintf(fp, "FAILURE: proc_opts[%d].multiple should be 0 or 1\n",i);
                }
                else if (verbose) {
                    fprintf(fp, "SUCCESS: proc_opts[%d].multiple is 0 or 1\n",i);
                }
                if(opt[i].required != 0 && opt[i].required != 1) {
                    fprintf(fp, "FAILURE: proc_opts[%d].required should be 0 or 1\n",i);
                }
                else if (verbose) {
                    fprintf(fp, "SUCCESS: proc_opts[%d].required is 0 or 1\n",i);
                }
                i++;
            }
        }
        else {
            fprintf(fp, "FAILURE: proc_opts is null or undefined\n"); 
        }
          
        // verify that this variable exists 
        const char * ns_opt = (const char*)DLSYM(
            sh_file_handle,"proc_nonswitch_opts"); 
        check_kid_field_single_dereference(
            fp, "proc_nonswitch_opts", ns_opt, 0);

        // verify that this variable exists 
        // check for procbuffer kid
        const int * is_procbuffer = (const int *)DLSYM(
            sh_file_handle,"is_procbuffer");
        const char ** itypes;
        if (!is_procbuffer) {
            itypes = (const char**)DLSYM(
                sh_file_handle,"proc_input_types"); 
            check_kid_field_double_dereference(
                fp, "proc_input_types", itypes, 0);
        }

        // verify that this variable exists 
        // check for procbuffer kid
        const char ** otypes;
        if (!is_procbuffer) {
            otypes = (const char**)DLSYM(
                sh_file_handle,"proc_output_types");
            check_kid_field_double_dereference(
                fp, "proc_output_types", otypes, 0);
        }

        // verify that this variable exists 
        const proc_port_t * ports;
        ports = (const proc_port_t*)DLSYM(
            sh_file_handle,"proc_input_ports");
        if (ports) {
            if(verbose) {
                fprintf(fp, "SUCCESS: proc_input_ports is defined\n");
            }

            int i = 0;
            while (ports[i].label != NULL) {
                if (strlen(ports[i].label) == 0) {
                    fprintf(fp, "FAILURE: proc_input_ports[%d].label has a zero-length\n",i);
                }
                else if (verbose) {
                    fprintf(fp, "SUCCESS: proc_input_ports[%d].label has a length\n",i);
                }
                if (ports[i].description == NULL
                    || strlen(ports[i].description) == 0) {
                    fprintf(fp, "FAILURE: proc_input_ports[%d].description is null or zero-lengthed\n",i);
                }
                else if (verbose) {
                    fprintf(fp, "SUCCESS: proc_input_ports[%d].description has a length\n",i);
                }
                i++;
            }
        }
        else {
            fprintf(fp, "FAILURE: proc_input_ports  is null or undefined\n"); 
        }

        // verify that this variable exists 
        const char ** tml = (const char **)DLSYM(
            sh_file_handle,"proc_tuple_member_labels");
        check_kid_field_double_dereference(
            fp, "proc_tuple_member_labels", tml, 0);

        // verify that this variable exists 
        const char ** tcl = (const char **)DLSYM(
            sh_file_handle,"proc_tuple_container_labels");
        check_kid_field_double_dereference(
            fp, "proc_tuple_container_labels", tcl, 0);

        // verify that this variable exists 
        const char ** tccl = (const char **)DLSYM(
            sh_file_handle,
            "proc_tuple_conditional_container_labels");
        check_kid_field_double_dereference(
            fp,
            "proc_tuple_conditional_container_labels",
            tccl,
            0);
    }
    else {
        return 0;
    }
    return 1;
}

/**
 * by default this will use the new print out to outfp option
 * unless you want to print to xml
 */
int print_module_help(FILE * fp, const char *spath) {
    DLHANDLE sh_file_handle;
    //snprintf(spath,255,"%s/module_%s.so", g_pLibPath, modulename);
    if (spath == NULL)
        return 0;
    // try to locate the passed module name
    if ((sh_file_handle = DLOPEN(spath))) {
        // find out if the module is deprecated
        const int * is_deprecated = (const int *) DLSYM(
            sh_file_handle,"is_deprecated"); 
        // get the information from the module that we
        // need to print help
        const char ** proc_alias  = (const char**)DLSYM(
            sh_file_handle,"proc_alias");
        const char * proc_name    = (const char *)DLSYM(
            sh_file_handle,"proc_name");
        const char * proc_purpose = (const char *)DLSYM(
            sh_file_handle,"proc_purpose");
        const char ** proc_synopsis  = (const char**)DLSYM(
            sh_file_handle,"proc_synopsis");
        const char * proc_description = (const char *)DLSYM(
            sh_file_handle,"proc_description");
        const char ** proc_tags  = (const char**)DLSYM(
            sh_file_handle,"proc_tags");
        const proc_example_t * examples;
        examples = (const proc_example_t*)DLSYM(
            sh_file_handle,"proc_examples"); 
        const char * proc_requires = (const char *)DLSYM(
            sh_file_handle,"proc_requires");
        const char * proc_version = (const char *)DLSYM(
            sh_file_handle,"proc_version");
        const proc_option_t * opt;
        opt = (const proc_option_t*)DLSYM(
            sh_file_handle,"proc_opts"); 
        const char * ns_opt = (const char*)DLSYM(
            sh_file_handle,"proc_nonswitch_opts"); 
          
        const char ** itypes;
        const char ** otypes;     
        // check for procbuffer kid
        const int * is_procbuffer = (const int *) DLSYM(
            sh_file_handle,"is_procbuffer");
        const char * proc_input_types[] = {"tuple", "monitor", NULL};
        const char * proc_output_types[] = {"tuple", NULL};
        if (is_procbuffer) {
            itypes = (const char**)proc_input_types;
            otypes = (const char**)proc_output_types;
        } 
        else {
            itypes = (const char**)DLSYM(
                sh_file_handle,"proc_input_types");
            otypes = (const char**)DLSYM(
                sh_file_handle,"proc_output_types");
        }

        const proc_port_t * ports = (proc_port_t*)DLSYM(
            sh_file_handle,"proc_input_ports");

        const char ** tml = (const char **)DLSYM(
            sh_file_handle,"proc_tuple_member_labels");
        const char ** tcl = (const char **)DLSYM(
            sh_file_handle,"proc_tuple_container_labels");
        const char ** tccl = (const char **)DLSYM(
            sh_file_handle,
            "proc_tuple_conditional_container_labels");

        title(fp, "PROCESSOR NAME");
        if (proc_purpose) {
            // concat the name and purpose into one string
            // for word wrapping
            size_t len = strlen(proc_name) + strlen(proc_purpose) + 3;
            if (is_deprecated) {
                int dep_len = strlen("DEPRECATED: ");
                len +=dep_len;
            }
            char * name_purpose = (char*) malloc(len+1);
            if (!name_purpose) {
                error_print("failed print_module_help calloc of name_purpose");
                return 0;
            }
            if (is_deprecated) {
                snprintf(name_purpose, len+1, "%s - DEPRECATED: ", proc_name);
            }
            else {
                snprintf(name_purpose, len+1, "%s - ", proc_name);
            }
            strncat(name_purpose, proc_purpose, strlen(proc_purpose)+1);
            print_wrap(fp, name_purpose, WRAP_WIDTH, 4);
            free(name_purpose);
            fprintf(fp, "\n");
        } else {
            if (is_deprecated) {
                fprintf(fp, "\t%s - DEPRECATED", proc_name);
            }
            else { 
                fprintf(fp, "\t%s - ", proc_name);
            }
            fprintf(fp, "\n");
        }
        if (proc_synopsis) {
            title(fp, "SYNOPSIS");
            int i = 0;
            while (proc_synopsis[i] != NULL){
                print_wrap(fp, proc_synopsis[i], WRAP_WIDTH, 4);
                i++;
            }
            fprintf(fp, "\n");
        }
        if (proc_description && verbose) {
            title(fp, "DESCRIPTION");
            print_wrap(fp, proc_description, WRAP_WIDTH, 4);
            fprintf(fp, "\n");
        }
        if (proc_tags && proc_tags[0] && verbose) {
            title(fp, "TAGS");
            print_list(fp, proc_tags);
            fprintf(fp, "\n");
        }
        if (examples && verbose) {
            title(fp, "EXAMPLES");
            int i = 0;
            while (examples[i].text != NULL) {
                if (*examples[i].text != '\0') {
                    print_wrap(fp, examples[i].text, WRAP_WIDTH, 4);
                    print_wrap(fp, examples[i].description, WRAP_WIDTH, 8);
                }
                i++;
            }
            fprintf(fp, "\n");
        }
        if (proc_alias && proc_alias[0] && verbose) {
            title(fp, "ALIAS");
            print_list(fp, proc_alias);
            fprintf(fp, "\n");
        }
        if (verbose) {
            title(fp, "VERSION");
            fprintf(fp, "    %s\n", proc_version);
            fprintf(fp, "\n");
        }
        if (proc_requires && verbose) {
            title(fp, "REQUIRES");
            print_wrap(fp, proc_requires, WRAP_WIDTH, 4);
            fprintf(fp, "\n");
        }
        if (itypes && verbose) {
            title(fp, "INPUT TYPES");
            print_list(fp, itypes);
            fprintf(fp, "\n");
        }
        if (otypes && verbose) {
            title(fp, "OUTPUT TYPES");
            print_list(fp, otypes);
            fprintf(fp, "\n");
        }
        if (ports) {
            title(fp, "INPUT PORTS");
            print_portlist(fp, ports);
            fprintf(fp, "\n");
        }
        if (tcl && verbose) {
            title(fp, "TUPLE CONTAINER LABELS");
            print_list(fp, tcl);
            fprintf(fp, "\n");
        }
        if (tccl && verbose) {
            title(fp, "TUPLE CONDITIONAL CONTAINER LABELS");
            print_list(fp, tccl);
            fprintf(fp, "\n");
        }
        if (tml && verbose) {
            title(fp, "APPENDED TUPLE MEMBERS");
            print_list(fp, tml);
            fprintf(fp, "\n");
        }
        if (opt || ns_opt) {
            title(fp, "OPTIONS");
            if (ns_opt && strlen(ns_opt) > 0) {
                size_t len = strlen(ns_opt) + 2; 
                char * ns = (char *) malloc(len+1);
                if (!ns) {
                    error_print("failed print_module_help calloc of ns");
                    return 0;
                }
                snprintf(ns, len+1, "<%s>", ns_opt); 
                print_wrap(fp, ns, WRAP_WIDTH, 4);
                free(ns);
            }
            if (!opt) {
                DLCLOSE(sh_file_handle);
                return 1;
            }
            int i = 0;
            while (opt[i].option != ' ') {
                size_t sz = 1024;
                char * buf = (char *) calloc(1,sz);
                char * p = buf;
                         
                int read = 0;
                if (opt[i].option != '\0') {
                    read = snprintf(p, 4, "[-%c", opt[i].option);  
                }
                else {
                    while ((read+3+strlen(opt[i].long_option)+1) > sz) {
                        sz += 1024;
                        buf = (char *) realloc(buf, sz);
                        if (!buf) {
                            error_print("failed opt.long_option realloc"); 
                        }
                        p = buf+read;
                    }
                    read = snprintf(p,3+strlen(opt[i].long_option)+1,
                        "[--%s", opt[i].long_option);
                }
                p = p + read;
                if (opt[i].argument && strlen(opt[i].argument)) {
                    while ((read+3+strlen(opt[i].argument)+1) > sz) {
                        sz += 1024;
                        buf = (char *) realloc(buf, sz);
                        if (!buf) {
                            error_print("failed opt.argument realloc"); 
                        }
                        p = buf+read;
                    }
                    read = snprintf(
                        p,
                        3+strlen(opt[i].argument) + 1,
                        " <%s>",
                        opt[i].argument);
                    p = p + read;
                }
                while ((read+2+strlen(opt[i].description)+1) > sz) {
                    sz += 1024;
                    buf = (char *) realloc(buf, sz);
                    if (!buf) {
                        error_print("failed opt.description realloc"); 
                    }
                    p = buf+read;
                }
                snprintf(p,2+strlen(opt[i].description)+1,
                    "] %s", opt[i].description);
                print_wrap(fp, buf, WRAP_WIDTH, 4);
                i++;
                free(buf);
            }
        }
        // close this shared-module handle
        DLCLOSE(sh_file_handle);
        return 1;
    }
    else {
        return 0;
    }
}

int print_module_help_search(
        FILE * fp,
        const char * spath,
        const char * keyword)
{
     DLHANDLE sh_file_handle;
     //snprintf(spath,255,"%s/module_%s.so", g_pLibPath, modulename);
     if (spath == NULL) { return 0;}

     // try to locate the passed module name
     if ((sh_file_handle = DLOPEN(spath))) {
          // find out if the module is deprecated
          const int * is_deprecated = (const int *)DLSYM(
              sh_file_handle,"is_deprecated");
          // get the information from the module that we
          // need to print help
          const char ** proc_alias  = (const char**)DLSYM(
              sh_file_handle,"proc_alias");
          const char * proc_name    = (const char *)DLSYM(
              sh_file_handle,"proc_name");
          const char * proc_purpose = (const char *)DLSYM(
              sh_file_handle,"proc_purpose");
          const char ** proc_synopsis  = (const char**)DLSYM(
              sh_file_handle,"proc_synopsis");
          const char * proc_description = (const char *)DLSYM(
              sh_file_handle,"proc_description");
          const char ** proc_tags  = (const char**)DLSYM(
              sh_file_handle,"proc_tags");
          const proc_example_t * examples = (const proc_example_t*)DLSYM(
              sh_file_handle,"proc_examples"); 
          const char * proc_requires = (const char *)DLSYM(
              sh_file_handle,"proc_requires");
          const char * proc_version = (const char *)DLSYM(
              sh_file_handle,"proc_version");
          const proc_option_t * opt = (const proc_option_t*)DLSYM(
              sh_file_handle,"proc_opts");
          const char * ns_opt = (const char*)DLSYM(
              sh_file_handle,"proc_nonswitch_opts");
          const char ** itypes = (const char**)DLSYM(
              sh_file_handle,"proc_input_types");
          const char ** otypes = (const char**)DLSYM(
              sh_file_handle,"proc_output_types");

          const proc_port_t * ports = (const proc_port_t*)DLSYM(
              sh_file_handle,"proc_input_ports");

          const char ** tml = (const char **)DLSYM(
              sh_file_handle,"proc_tuple_member_labels");
          const char ** tcl = (const char **)DLSYM(
              sh_file_handle,"proc_tuple_container_labels");
          const char ** tccl = (const char **)DLSYM(
              sh_file_handle,
              "proc_tuple_conditional_container_labels");

          title(fp, "PROCESSOR NAME");
          if (proc_purpose) {
               // concat the name and purpose into one
               // string for word wrapping
               char * name_purpose, * p;
               if (is_deprecated) {
                    _asprintf(
                        &name_purpose,
                        "%s - DEPRECATED: %s",
                        proc_name,
                        proc_purpose);
               }
               else {
                    _asprintf(
                        &name_purpose,
                        "%s - %s",
                        proc_name,
                        proc_purpose);
               }
               p = name_purpose;
               name_purpose = search_and_highlight(
                   outfp,name_purpose, keyword);
               if (name_purpose)
                   free(p);
               else
                   name_purpose = p;
               print_wrap(fp, name_purpose, WRAP_WIDTH, 4);
               free(name_purpose);
          } else { 
               char * name_purpose, * p;
               if (is_deprecated) {
                    _asprintf(
                        &name_purpose,
                        "%s - DEPRECATED",
                        proc_name);
               }
               else {
                    _asprintf(
                        &name_purpose,
                        "%s -",
                        proc_name);
               }
               p = name_purpose;
               name_purpose = search_and_highlight(
                   outfp,name_purpose, keyword);
               if (name_purpose)
                   free(p);
               else
                   name_purpose = p;
               print_wrap(fp, name_purpose, WRAP_WIDTH, 4);
               free(name_purpose);
          }
          if (proc_synopsis) {
               title(fp, "SYNOPSIS");
               print_list_search(fp, proc_synopsis, keyword);
               fprintf(fp, "\n");
          }
          if (proc_description) {
               title(fp, "DESCRIPTION");
               const char * p = proc_description;
               proc_description = search_and_highlight(
                   outfp,proc_description, keyword);
               if (!proc_description)
                   proc_description = p;
               print_wrap(fp, proc_description, WRAP_WIDTH, 4);
               fprintf(fp, "\n");
          }
          if (proc_tags && proc_tags[0]) {
               title(fp, "TAGS");
               print_list_search(fp, proc_tags, keyword);
               fprintf(fp, "\n");
          }
          if (examples) {
               title(fp, "EXAMPLES");
               int i = 0;
               while (examples[i].text != NULL) {
                    if (*examples[i].text != '\0') {
                         char * p;
                         char * q;
                         highlight(p, examples[i].text, keyword, fp, WRAP_WIDTH, 4);
                         highlight(q, examples[i].description, keyword, fp, WRAP_WIDTH, 8);
                    }
                    i++;
               }
               fprintf(fp, "\n");
          }
          if (proc_alias && proc_alias[0]) {
               title(fp, "ALIAS");
               print_list_search(fp, proc_alias, keyword);
               fprintf(fp, "\n");
          }
          title(fp, "VERSION");
          if (proc_version) {
               fprintf(fp, "    %s\n", proc_version);
               fprintf(fp, "\n");
          }
          if (proc_requires) {
               title(fp, "REQUIRES");
               const char * p = proc_requires;
               proc_requires = search_and_highlight(outfp,proc_requires, keyword);
               if (!proc_requires) proc_requires = p;
               print_wrap(fp, proc_requires, WRAP_WIDTH, 4);
               fprintf(fp, "\n");
          }
          if (itypes) {
               title(fp, "INPUT TYPES");
               print_list_search(fp, itypes, keyword);
               fprintf(fp, "\n");
          }
          if (otypes) {
               title(fp, "OUTPUT TYPES");
               print_list_search(fp, otypes, keyword);
               fprintf(fp, "\n");
          }
          if (ports) {
               title(fp, "INPUT PORTS");
               print_portlist_search(fp, ports, keyword);
               fprintf(fp, "\n");
          }
          if (tcl) {
               title(fp, "TUPLE CONTAINER LABELS");
               print_list_search(fp, tcl, keyword);
               fprintf(fp, "\n");
          }
          if (tccl) {
               title(fp, "TUPLE CONDITIONAL CONTAINER LABELS");
               print_list_search(fp, tccl, keyword);
               fprintf(fp, "\n");
          }
          if (tml) {
               title(fp, "APPENDED TUPLE MEMBERS");
               print_list_search(fp, tml, keyword);
               fprintf(fp, "\n");
          }
          if (opt || ns_opt) {
               title(fp, "OPTIONS");
               if (ns_opt && strlen(ns_opt) > 0) {
                    size_t len = strlen(ns_opt) + 2; 
                    char * ns = (char *) malloc(len+1);
                    if (!ns) {
                         error_print("failed print_module_help_search calloc of ns");
                         return 0;
                    }
                    snprintf(ns, len+1, "<%s>", ns_opt); 
                    char *p;
                    highlight(p, ns, keyword, fp, WRAP_WIDTH, 4);
                    free(ns);
               }
               if (opt) {
                    int i = 0;
                    while (opt[i].option != ' ') {
                         int sz = 1024;
                         char * buf = (char *) calloc(1,sz);
                         char * p = buf;
                         int read = 0;
                         if (opt[i].option != '\0') {
                              read = snprintf(p, 4, "[-%c", opt[i].option);  
                         }
		         else {
                            while ((read+3+strlen(opt[i].long_option)+1) > sz) {
                                   sz += 1024;
                                   buf = (char *) realloc(buf, sz);
                                   if (!buf) {
                                        error_print("failed opt.long_option realloc"); 
                                   }
                                   p = buf+read;
                              }
                              read = snprintf(p,3+strlen(opt[i].long_option)+1,
                                   "[--%s", opt[i].long_option);
                         }
                         p = p + read;
                         if (opt[i].argument && strlen(opt[i].argument)) {
                              while ((read+3+strlen(opt[i].argument)+1) > sz) {
                                   sz += 1024;
                                   buf = (char *) realloc(buf, sz);
                                   if (!buf) {
                                        error_print("failed opt.argument realloc"); 
                                   }
                                   p = buf+read;
                              }
                              read = snprintf(p,3+strlen(opt[i].argument)+1,
                                   " <%s>", opt[i].argument);
                              p = p + read;
                         }
                         while ((read+2+strlen(opt[i].description)+1) > sz) {
                             sz += 1024;
                             buf = (char *) realloc(buf, sz);
                             if (!buf) {
                                  error_print("failed opt.description realloc"); 
                             }
                             p = buf+read;
                         }

                         snprintf(p,2+strlen(opt[i].description)+1,
                              "] %s", opt[i].description);
                         char * ptr;
                         highlight(ptr, buf, keyword, fp, WRAP_WIDTH, 4);
                         i++;
                    }
               }
          }

          // close this shared-module handle
          DLCLOSE(sh_file_handle);
          return 1;
     }
     else {
          DLERROR(errbuf,2047);
          fprintf(stderr,"%s\n", errbuf);
          fprintf(stderr,"unable to open module %s\n", spath);
          return 0;
     }
}

int print_module_help_short(FILE * fp, const char *spath) {
     DLHANDLE sh_file_handle;
     if (spath == NULL) {
          return 0;
     }
     // try to locate the passed module name
     if ((sh_file_handle = DLOPEN(spath))) {
          // find out if module is deprecated
          int * is_deprecated = (int *)DLSYM(sh_file_handle,"is_deprecated");
          char * proc_name    = (char *) DLSYM(sh_file_handle,"proc_name");
          char * proc_purpose = (char *) DLSYM(sh_file_handle,"proc_purpose");
          if (proc_purpose) {
               if (is_deprecated) {
                    fprintf(fp, "%-20s\t- DEPRECATED: %s", proc_name, proc_purpose);
               }
               else {
                    fprintf(fp, "%-20s\t- %s", proc_name, proc_purpose);
               }
               fprintf(fp, "\n");
          } else {
               if (is_deprecated) {
                    fprintf(fp, "%-20s\t- DEPRECATED", proc_name);
               }
               else {
                    fprintf(fp, "%-20s\t-", proc_name);
               }
               fprintf(fp, "\n");
          }

          DLCLOSE(sh_file_handle);
          return 1;
     }
     else {
          DLERROR(errbuf,2047);
          fprintf(stderr,"%s\n", errbuf);
          fprintf(stderr,"unable to open module %s\n", spath);
          return 0;
     }
}
// find the right .so filename to load as a datatype
int datatype_file_filter(const char* name) {
     size_t len;
     if (strncmp(name, "proc_", 5) != 0)
          return 0;
     len = strlen(name);
     if ( (strncmp(name+(len-5),"ws_so", 5) == 0)
         || (strncmp(name+(len-6),"wsp_so",6) == 0)) {
          return 1;
     }
     return 0;
}

void datatype_file_callback(const char*,const char*,void*);
void load_proc_dir_term(
        const char * dirname,
        const char * term)
{
    int n;
    //char spath[5000];

    if (dirname == NULL) {
         fprintf(stderr,"no datatype path.. set environment");
         return;
    }
    n = wsdir_scan(
         dirname,
         datatype_file_filter,
         datatype_file_callback,
         (void*)term);
    if (n < 0) {
         perror("scandir");
    }
}

void datatype_file_callback(
          const char* dirname,
          const char* name,
          void* opaque) {
     char spath[5000];
     char * term = (char*)opaque;
     size_t dirlen = strlen(dirname);
     memcpy(spath, dirname, dirlen);
     spath[dirlen] = '/';
     int namelen;
     int divider = 0;

     int index = strchr(name, '.') - name;
     char * kid = _strndup(name, index);

     // Check if the last kid we found was the serial
     // version of the same kid. We don't want to process
     // the same kid twice.
     if (last_kid) {
          int same_kid = (strcmp(kid, last_kid) == 0);
          free(last_kid);
          last_kid = kid;
//          if (same_kid) continue;
// TODO: does this break it?
            if (same_kid) return;
     }
     last_kid = kid;

     namelen = strlen(name);
     memcpy(spath + dirlen + 1, name, namelen);
     spath[dirlen+namelen + 1] = '\0';

     if (tag_search && search_for_tag(spath, term)) {
          if (verbose) {
               if (divider) {
                    print_divider(outfp);
                    divider = 0;
               }
               print_module_help(outfp, spath);
               divider = 1;
               fprintf(outfp,"\n");
          } else {
               print_module_help_short(outfp, spath);
          }
     }
     if (input_type_search && search_for_input_type(spath, term)) {
          if (verbose) {
               if (divider) {
                    print_divider(outfp);
                    divider = 0;
               }
               print_module_help(outfp, spath);
               divider = 1;
               fprintf(outfp,"\n");
          } else {
               print_module_help_short(outfp, spath);
          }
     }

     if (output_type_search && search_for_output_type(spath, term)) {
          if (verbose) {
               if (divider) {
                    print_divider(outfp);
                    divider = 0;
               }
               print_module_help(outfp, spath);
               divider = 1;
               fprintf(outfp,"\n");
          } else {
               print_module_help_short(outfp, spath);
          }
     }
     if (keyword_search && search_for_keyword(spath, term)) {
          if (divider) {
              print_divider(outfp);
              divider = 0;
          }
          print_module_help_search(outfp, spath, term);
          divider = 1;
          fprintf(outfp,"\n");
     }
}

void load_proc_dir(const char * dirname) {

     if (dirname == NULL) {
          fprintf(stderr,"no datatype path.. set environment");
          return;
     }
     int n = wsdir_scan(
          dirname,
          datatype_file_filter,
          datatype_file_callback,
          NULL);
     if (n < 0) {
          perror("scandir");
     }
}

#define DIRSEP_STR ":"
void load_multiple_dirs(const char * libpath) {
     size_t len = strlen(libpath);
     char * buf = (char*) calloc(1,len);
     strcpy(buf, libpath);
     char * dir = _strsep(&buf, DIRSEP_STR);

     while (dir) {
          load_proc_dir(dir);
          dir = _strsep(&buf, DIRSEP_STR);
     }
}

void load_multiple_dirs_term(
        const char * libpath,
        const char * term)
{
     size_t len = strlen(libpath);
     char * buf = (char*) calloc(1,len);
     strcpy(buf, libpath);
     char * dir = _strsep(&buf, DIRSEP_STR);

     while (dir) {
          load_proc_dir_term(dir, term);
          dir = _strsep(&buf, DIRSEP_STR);
     }
}


void find_specific_module(
        const char * kid,
        const char * libpath)
{
    size_t len = strlen(libpath);
    char * buf = (char*) calloc(1,len);
    strcpy(buf, libpath);
    char * dir = _strsep(&buf, DIRSEP_STR);

    char * mpath_serial;
    char * mpath_parallel;
    char * mps_alias;
    char * mpp_alias;
    while (dir) {
        mpath_serial = get_module_path(kid, dir);
        mpath_parallel = get_module_path_parallel(
            kid, dir);
        if (check_kid) {
            if (check_kid_documentation(outfp, mpath_serial, kid)) {
                free(mpath_serial);
                free(mpath_parallel);
                return;
            }
            if (check_kid_documentation(outfp, mpath_parallel, kid)) {
                free(mpath_serial);
                free(mpath_parallel);
                return;
            }

            // our previous lookups failed: let's see if
            // this is an alias for another kid
            const char * aliased_kid = map_find(map, kid);

            if (aliased_kid) {
                mps_alias = get_module_path(
                    aliased_kid, dir);
                mpp_alias = get_module_path_parallel(
                    aliased_kid, dir);

                if (check_kid_documentation(outfp, mps_alias, kid)) {
                    free(mpath_serial);
                    free(mpath_parallel);
                    free(mps_alias);
                    free(mpp_alias);
                    return;
                }

                if (check_kid_documentation(outfp, mpp_alias, kid)) {
                    free(mpath_serial);
                    free(mpath_parallel);
                    free(mps_alias);
                    free(mpp_alias);
                    return;
                }
            }

            fprintf(stderr, "FAILURE: could not find %s\n", kid);
        } else {
            if (print_module_help(outfp, mpath_serial)) {
                free(mpath_serial);
                free(mpath_parallel);
                return;
            }
            if (print_module_help(outfp, mpath_parallel)) {
                free(mpath_serial);
                free(mpath_parallel);
                return;
            }

            // couldn't find kid so far - is this an
            // alias?
            const char * aliased_kid = map_find(map, kid);

            if (aliased_kid) {
                // Yep, this is an alias
                mps_alias = get_module_path(
                    aliased_kid, dir);
                mpp_alias = get_module_path_parallel(
                    aliased_kid, dir);

                if (print_module_help(outfp, mps_alias)) {
                    free(mpath_serial);
                    free(mpath_parallel);
                    free(mps_alias);
                    free(mpp_alias);
                    return;
                }
                if (print_module_help(outfp, mpp_alias)) {
                    free(mpath_serial);
                    free(mpath_parallel);
                    free(mps_alias);
                    free(mpp_alias);
                    return;
                }
            }

            // we've done our best, but we can't find this kid.
            DLERROR(errbuf,2047);
            fprintf(stderr,"%s\n", errbuf);
            fprintf(stderr,"unable to open module %s\n", kid);
        }
         
        free(mpath_serial);
        free(mpath_parallel);
        dir = _strsep(&buf, DIRSEP_STR);
    }
}

int build_alias_map(const char * alias_path) {
     char buf[1000];

     map = map_create();
     if (!map) return 1;

     FILE * fp = fopen(alias_path, "r");
     if (!fp) return 1;

     while (fgets(buf, 1000, fp)) {
          char * line = trim(buf);
          char * kid_name = line;
          char * aliases = strstr(line, ":");
          // replace the ":" with a null-terminator to split the string
          *aliases= 0;
          aliases+= 2;
          char * token = strtok (aliases, " ,");
          while (token != NULL) {
               map_insert(map, token, kid_name);
               token = strtok (NULL, " ,");
          }
     }

     fclose(fp);

     return 0;
}


// entry point (possibly receive a quoted command line
int main(int argc, char *argv[]) {

    const char * alias_path;
    const char * lib_path;

    // check for preferred pager
    const char * pager = getenv("PAGER");
    if (pager && strlen(pager) > 0) {
       if (NULL == (outfp = popen(pager, "w"))) {
           fprintf(stderr, "pipe-open on pager failed...defaulting to stdout\n");
           outfp = stdout;
       } else {
          is_pager = 1;
       }
    } else {
       outfp = stdout;
    }

    set_default_env();

    if ((lib_path = getenv(ENV_WS_PROC_PATH)) == NULL) {
        fprintf(stderr,
                "Please set the %s path variable... ", 
                ENV_WS_PROC_PATH); 
        fprintf(stderr,"taking a guess\n\n"); 
        lib_path = "./procs";
    }

    if ((alias_path = getenv(ENV_WS_ALIAS_PATH)) == NULL) {
        fprintf(stderr,
                "Please set the %s path variable... ",
                ENV_WS_ALIAS_PATH);
        fprintf(stderr,"taking a guess\n\n");
        alias_path = "./procs/.wsalias";
    }

    build_alias_map(alias_path);

    int op;
    int force_verbose = 0;
    while ((op = getopt(argc, argv, "csiotvVhmM?")) != EOF) {
        switch (op) {
        case 'c':
            check_kid = 1;
            break;
        case 's':
            keyword_search = 1;
            break;
        case 't':
            tag_search = 1;
            break;
        case 'i':
            input_type_search = 1;
            break;
        case 'o':
            output_type_search = 1;
            break;
        case 'v':
        case 'V':
            verbose = 1;
            force_verbose = 1;
            break;
        case 'm':
        case 'M':
            verbose = 0;
            break;
        case 'h':
        case '?':
            print_usage(outfp);
            return 0;
        default:
            return 0;
        }
    }

    if (!verbose) {
        print_helpful_message(stderr);
    }

    argc -= optind;
    argv += optind;

    if (!argc && !force_verbose) {
        verbose = 0;
    }

    if (argc > 0) {
        int x;
        for (x = 0; x < argc; x++) {
            if (x > 1) {
                fprintf(outfp, "\n");
            }
            char * p = argv[x];
            for ( ; *p; p++)
                *p = tolower(*p);

            if (keyword_search) {
                fprintf(outfp,"Searching for \"%s\"...\n\n", argv[x]);
                load_multiple_dirs_term(lib_path, argv[x]);
                //can only search a single term, so break after first iter
                break;
            } else if (check_kid) {
                find_specific_module(argv[x], lib_path);
            } else if (tag_search) {
                load_multiple_dirs_term(lib_path, argv[x]);
            } else if (input_type_search) {
                load_multiple_dirs_term(lib_path, argv[x]);
            } else if (output_type_search) {
                load_multiple_dirs_term(lib_path, argv[x]);
            } else {
                find_specific_module(argv[x], lib_path);
            }
            fflush(outfp);
        }
    } else if (keyword_search) {
        fprintf(stderr, "ERROR: Keyword search requires an argument\n");
    } else if (check_kid) {
        fprintf(stderr, "ERROR: Kid check requires an argument\n");
    } else if (tag_search) {
          fprintf(stderr, "ERROR: Tag search requires an argument\n");
    } else if (input_type_search) {
        fprintf(stderr, "ERROR: Input type search requires an argument\n");
    } else if (output_type_search) {
        fprintf(stderr, "ERROR: Output type search requires an argument\n");
    } else {
        //print all modules in path
        load_multiple_dirs(lib_path);
    }

    // close pager
    if (is_pager)
        pclose(outfp);

    fprintf(stdout,"--- ok\n");
    return 0;
}
#ifdef __cplusplus
CPP_CLOSE
#endif

