/*================================================================================

    csmi/src/wm/cmd/allocation_create.c

  © Copyright IBM Corporation 2015,2016. All Rights Reserved

    This program is licensed under the terms of the Eclipse Public License
    v1.0 as published by the Eclipse Foundation and available at
    http://www.eclipse.org/legal/epl-v10.html

    U.S. Government Users Restricted Rights:  Use, duplication or disclosure
    restricted by GSA ADP Schedule Contract with IBM Corp.

================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include "csmi/include/csm_api_workload_manager.h"
/*Needed for CSM logging*/
#include "csmutil/include/csmutil_logging.h"
/* Command line macros for ease of use. */
#include "csmi/src/common/include/csmi_internal_macros.h"

#include <assert.h>
#include <sys/types.h> // For user details.
#include <pwd.h>       // For user details.
#include <unistd.h>    // For UID

///< For use as the usage variable in the input parsers.
#define USAGE  csm_free_struct_ptr(csmi_allocation_t, allocation); help

struct option longopts[] = {
	{"help",             no_argument,       0, 'h'},
	{"verbose",          required_argument, 0, 'v'},
	{"primary_job_id",   required_argument, 0, 'j'},
	{"secondary_job_id", required_argument, 0, 'J'},
	{"state",            required_argument, 0, 's'},
	{"type",             required_argument, 0, 't'},
	{"user_name",        required_argument, 0, 'u'},
	{"user_id",          required_argument, 0, 'U'},
	{"node_range",       required_argument, 0, 'n'},
	{"isolated_cores",   required_argument, 0, 'c'},
    {"shared",           no_argument,       0, 'S'},
	{"launch_node_name", required_argument, 0, 'l'},
    //{"user_flags",       required_argument, 0, 'A'},/// FIXME REMOVE IN FINAL 
    //{"system_flags",     required_argument, 0, 'a'},/// FIXME REMOVE IN FINAL 
	{0, 0, 0, 0}
};

static void help() {
	puts("_____CSM_ALLOCATION_CREATE_CMD_HELP_____");
	puts("USAGE:");
	puts("  csm_allocation_create ARGUMENTS [OPTIONS]");
	puts("  csm_allocation_create -j primary_job_id -n \"node01,node02\" [-J secondary_job_id] [-s state] [-t type] [-u user_name] [-U user_id] [-h] [-v verbose_level] [--create_cgroup] [-l launch_node_name]");
	puts("");
	puts("SUMMARY: Used to create an allocation.");
	puts("");
	puts("EXIT STATUS:");
	puts("  0  if OK,");
	puts("  1  if ERROR.");
	puts("");
	puts("ARGUMENTS:");
	puts("  MANDATORY:");
	puts("    csm_allocation_create expects 2 mandatory arguments");
	puts("    Argument                | Example value      | Description  ");                                                 
	puts("    ------------------------|--------------------|--------------");
	puts("    -j, --primary_job_id    | 1                  | (LONG INTEGER) Primary job id [>0] (for lsf this will be the lsf job id).");
	puts("                            |                    | ");
	/*The following lines have 2 extra spaces to account for the escaped quotes. This way it lines up in the command line window.*/
	puts("    -n, --node_range        | \"node01,node02\"    | (STRING) The node_names to be allocated. Separated by comma.");
	puts("                            |                    | (at least 1 node (node01,node02,...) is required)");
	puts("                            |                    | ");
	puts("  OPTIONAL:");
	puts("    csm_allocation_create can have 5 optional arguments");
	puts("    Argument               | Example value   | Description  ");                                                 
	puts("    -----------------------|-----------------|--------------");
	puts("    -J, --secondary_job_id | 0               | (INTEGER) Secondary job id (for lsf this will be the lsf job index for job arrays) [>=0].");
	puts("                           |                 | (default = '0')");
	puts("                           |                 | ");
	puts("    -s, --state            | \"running\"       | (STRING) The state of the allocation.");
	puts("                           |                 | (default = \"running\")");
	puts("                           |                 | Valid values: \"staging-in\", \"running\"");
	puts("                           |                 | ");
	puts("    -t, --type             | \"user-managed\"  | (STRING) The type of allocation.");
	puts("                           |                 | (default = \"user-managed\")");
	puts("                           |                 | Valid values: \"user-managed\",\"jsm\",\"jsm-cgroup-step\", or \"diagnostics\"");
	puts("                           |                 | ");
	puts("    -u, --user_name        |                 | (STRING) The owner of this allocation's Linux user name.");
	puts("                           |                 | (default is invoking user)");
	puts("                           |                 | ");
	puts("    -l, --launch_node_name | \"some_host\"     | (STRING) The hostname of the launch node for the allocation.");
	puts("    -U, --user_id          | 0               | (INTEGER) The owner of this allocation's Linux user id.");
	puts("                           |                 | ");
	puts("    --isolated_cores       | 0               | (INTEGER) Specifies the number of cores ot isolate in the system cgroup ( if 0 the system cgroup will not be created ).");
	puts("                           |                 | Valid values: [0 - 4]");
	puts("");
	puts("GENERAL OPTIONS:");
	puts("[-h, --help]                  | Help.");
	puts("[-v, --verbose verbose_level] | Set verbose level. Valid verbose levels: {off, trace, debug, info, warning, error, critical, always, disable}");
	puts("");
	puts("EXAMPLE OF USING THIS COMMAND:");
	puts("  csm_allocation_create -j 1 -n \"node01\"");
	puts("____________________");
}

int main(int argc, char *argv[])
{
    uint32_t             i;
    int                opt;
    int                indexptr = 0;
    int                return_value;
    int                test_value = 0;
    csmi_allocation_t*  allocation = NULL;
    //char             **compute_nodes = NULL;
    csm_api_object    *csm_obj = NULL;
    time_t             t;
    struct tm         *tm;
    char               tbuf[32];
	char *arg_check = NULL; ///< Used in verifying the long arg values.

    csm_init_struct_ptr(csmi_allocation_t, allocation);
    allocation->user_id = INT32_MAX;
    allocation->state   = CSM_RUNNING; 

    while ((opt = getopt_long(argc, argv, "hv:j:J:n:s:Sc:t:u:U:l:", longopts, &indexptr)) != -1) 
    {
        switch (opt) 
        {
		    case 'h':
                // EARLY RETURN
                USAGE();
                return CSMI_HELP;
		    case 'v':
                csm_set_verbosity( optarg, USAGE )
				break;
            case 'j':
                csm_optarg_test( "-j, --primary_job_id", optarg, USAGE );
                csm_str_to_int64( allocation->primary_job_id, optarg, arg_check,
                                "-j, --primary_job_id", USAGE );
                break;
            case 'J':
                csm_optarg_test( "-J, --secondary_job_id", optarg, USAGE );
                csm_str_to_int32( allocation->secondary_job_id, optarg, arg_check, 
                                "-J, --secondary_job_id", USAGE );
                break;
            case 'c':
                csm_optarg_test( "--isolated_cores", optarg, USAGE );
                csm_str_to_int32( allocation->isolated_cores, optarg, arg_check,
                                "--isolated_cores", USAGE );
                break;
            case 'S':
                allocation->shared = CSM_TRUE;
                break;
            case 'n':
            {
                csm_optarg_test( "-n, --node_range", optarg, USAGE );
                printf("optarg: %s\n", optarg);
                puts("Looking for noderange.");

                char* bracket_pointer = NULL;
                int reserved_chars = 0;
                printf("bracket_pointer: %p\n", bracket_pointer);
                bracket_pointer = strchr(optarg,'[');
                printf("bracket_pointer: %p\n", bracket_pointer);

                if(bracket_pointer == NULL)
                {
                    //No range was found
                    puts("noderange not found. default behavior.");
                   csm_parse_csv( optarg, allocation->compute_nodes, allocation->num_nodes, char*,
                            csm_str_to_char, NULL, "-n, --node_range", USAGE ); 
                }else{
                    //range was found
                    puts("noderange found. noderange behavior activated.");

                    //variables
                    int range_digits = 0;
                    int range_counter = 0;
                    char range_counter_buffer[512];
                    char* base_node_name = NULL;





                    printf ("'[' found at %li\n",bracket_pointer-optarg+1);
                    //record the number of characters in the string before the '['
                    printf ("reserved_chars: %i\n",reserved_chars);
                    reserved_chars = bracket_pointer-optarg;
                    printf ("reserved_chars: %i\n",reserved_chars);

                    //find the dash
                    char* dash_pointer = NULL;
                    printf("dash_pointer: %p\n", dash_pointer);
                    dash_pointer = strchr(optarg,'-');
                    printf("dash_pointer: %p\n", dash_pointer);

                    if(dash_pointer == NULL)
                    {
                        //bad
                    }else
                    {
                        //good - found more format
                        printf ("'-' found at %li\n", dash_pointer-optarg+1);

                        //calculate the range digits
                        printf("range_digits: %i\n", range_digits);
                        range_digits = (dash_pointer-optarg+1) - (bracket_pointer-optarg+1) -1;
                        printf("range_digits: %i\n", range_digits);
                    }

                    printf("char at '[': %c\n", *bracket_pointer);

                    char* first_node_digit = NULL;

                    first_node_digit = (char*)calloc(range_digits+1,sizeof(char));
                    //memcpy the stuff out of optarg
                    memcpy(first_node_digit, bracket_pointer+1, range_digits);

                    printf("first_node_digit: %s\n", first_node_digit);


                    //grab the first node digit. save to int. use this later to find total number of nodes

                    int first_node_digit_int = 0;
                    printf("first_node_digit_int: %i\n", first_node_digit_int);
                    first_node_digit_int = atoi(first_node_digit);
                    printf("first_node_digit_int: %i\n", first_node_digit_int);

                    //find end of range
                    char* last_node_digit = NULL;

                    last_node_digit = (char*)calloc(range_digits+1,sizeof(char));
                    //memcpy the stuff out of optarg
                    memcpy(last_node_digit, dash_pointer+1, range_digits);

                    printf("last_node_digit: %s\n", last_node_digit);

                    //grab the last node digit. save to int. use this later to find total number of nodes

                    int last_node_digit_int = 0;
                    printf("last_node_digit_int: %i\n", last_node_digit_int);
                    last_node_digit_int = atoi(last_node_digit);
                    printf("last_node_digit_int: %i\n", last_node_digit_int);

                    int totalNumberOfNodesInNodeRange = 0;
                    printf("totalNumberOfNodesInNodeRange: %i\n", totalNumberOfNodesInNodeRange);
                    totalNumberOfNodesInNodeRange = last_node_digit_int - first_node_digit_int + 1;
                    printf("totalNumberOfNodesInNodeRange: %i\n", totalNumberOfNodesInNodeRange);

                    //now that you know the total number of nodes in the range
                    //set the num nodes
                    allocation->num_nodes = totalNumberOfNodesInNodeRange;
                    //allocate the array
                    //so we can start filling up the node names
                    allocation->compute_nodes = (char**)calloc(allocation->num_nodes,sizeof(char*));

                    //grab the base node name for the range
                    base_node_name = (char*)calloc(reserved_chars+1,sizeof(char));

                    printf("base_node_name: %s\n", base_node_name);
                    memcpy(base_node_name, optarg, reserved_chars);
                    printf("base_node_name: %s\n", base_node_name);

                    //fill up all the node names in the range

                    //set up the first range counter
                    printf("first range_counter: %i\n", range_counter);
                    range_counter = first_node_digit_int;
                    printf("first range_counter: %i\n", range_counter);

                    char* full_node_name = NULL;

                    int i = 0;
                    for(i = 0; i < allocation->num_nodes; i++)
                    {


                        //reserve a node name with enough space for characters
                        // the reserved chars from above, plus the range digits, plus one for null terminated char
                        // example: c123f01p[01-10]
                        // reserved chars: c123f01p = 8
                        // range_digits: 01 = 2
                        // total space for malloc = 11
                        full_node_name = (char*)calloc(reserved_chars+range_digits+1,sizeof(char));

                        printf("full_node_name: %s\n", full_node_name);

                        //memcpy the base node name
                        memcpy(full_node_name, base_node_name, reserved_chars);

                        printf("full_node_name: %s\n", full_node_name);


                        //temp var for help
                        printf("range_counter: %i\n", range_counter);
                        sprintf(range_counter_buffer, "%0*d", range_digits, range_counter);
                        printf("range_counter_buffer: %s\n", range_counter_buffer);

                        //memcpy the node digits
                        memcpy(full_node_name + reserved_chars, range_counter_buffer, range_digits);


                         printf("full_node_name: %s\n", full_node_name);




                        allocation->compute_nodes[i] = strdup(full_node_name);

                        range_counter++;
                        free(full_node_name);
                    }

                    printf("allocation->num_nodes: %i\n", allocation->num_nodes);
                    printf("all the nodes in allocation->compute_nodes:\n");

                    for(i = 0; i < allocation->num_nodes; i++)
                    {
                        printf("allocation->compute_nodes[%i]: %s\n", i, allocation->compute_nodes[i]);
                    }
                    
                    



                    

                    


                    
                    
                }

                break;
            }
            case 's':
                csm_optarg_test( "-s, --state", optarg, USAGE )
                test_value = csm_get_enum_from_string(csmi_state_t, optarg);

                if ( test_value != -1 )
                    allocation->state = test_value;
                else
                {
                    printf( "Invalid argument, state : %s \n", optarg );
                    USAGE();
                    return CSMERR_INVALID_PARAM;
                }

                break;    
            case 't':
                csm_optarg_test( "-t, --type", optarg, USAGE )
                test_value = csm_get_enum_from_string( csmi_allocation_type_t, optarg);

                if ( test_value != -1 )
                    allocation->type = test_value;
                else
                {
                    printf( "Invalid argument, type : %s \n", optarg );
                    USAGE();
                    return CSMERR_INVALID_PARAM;
                }
                break;
            case 'u':
                csm_optarg_test( "-u, --user_name", optarg, USAGE )
                allocation->user_name = strdup(optarg);
                break;
            case 'U':
                csm_optarg_test( "-U, --user_id", optarg, USAGE )
                csm_str_to_int32( allocation->user_id, optarg, arg_check, "-U, --user_id", USAGE);
                break;
            case 'l':
                csm_optarg_test( "-l, --launch_node_name", optarg, USAGE )
                allocation->launch_node_name = strdup(optarg);
                break;
            /*
            case 'a':
                csm_optarg_test( optarg, USAGE )
                allocation->system_flags = strdup(optarg);
                
                break;
            case 'A':
                csm_optarg_test( optarg, USAGE )
                allocation->user_flags = strdup(optarg);
                
                break;
                */
            default:
                csmutil_logging(error, "unknown arg: '%c'\n", opt);
                USAGE();
                return CSMERR_INVALID_PARAM;
        }
    }

    if (allocation->num_nodes == 0) 
    {
        printf( "\nMore than one node must be specified:\n" );
        csm_free_struct_ptr(csmi_allocation_t, allocation);
        help();
        return CSMERR_INVALID_PARAM;
    }


    // -----------------------------------------------------
    // Computed values
    // -----------------------------------------------------

    // Handle the user account data.
    // =====================================================
    struct passwd *pw;
    
    // If the user id was not specified  attempt to get it.
    if ( allocation->user_id == INT32_MAX )
    {
        if ( allocation->user_name == NULL )
        {
            allocation->user_id = getuid();
        }
        else
        {
            pw = getpwnam(allocation->user_name);
            if (pw) 
            {
                allocation->user_id  = pw->pw_uid;
                allocation->user_group_id = pw->pw_gid;
            }
        }
    }

    // Determine the username from uid.
    if ( allocation->user_name == NULL && 
            allocation->user_id != INT32_MAX )
    {
        pw = getpwuid(allocation->user_id);
        if (pw)
        {
            allocation->user_name = strdup(pw->pw_name);
            allocation->user_group_id  = pw->pw_gid;
        }
    }

    // If the user name was empty exit.
    if ( allocation->user_name == NULL || 
           allocation->user_id == INT32_MAX )
    {
        printf("\t FAILED: Could not find user: \nUser Name: %s\nUser ID: %i\n", 
            allocation->user_name,
            allocation->user_id);

        csm_free_struct_ptr(csmi_allocation_t, allocation);
        return -1;
    }
    // =====================================================
    
    // Set Defaults
    if (allocation->primary_job_id <= 0)
    {
        csmutil_logging(warning, "Invalid 'primary_job_id supplied (<= 0), setting to 1.");
        allocation->primary_job_id = 1;
    }

    // Compute the current time for submit times.
    time(&t);
    tm = localtime(&t);
    strftime(tbuf, sizeof(tbuf), "%F %T", tm);

    allocation->job_submit_time = strdup(tbuf);
    // -----------------------------------------------------

    // Init a connection to the daemon.
	return_value = csm_init_lib();
	if ( return_value != 0 )
    { 
		csmutil_logging(error, "%s-%d:", __FILE__, __LINE__);
		csmutil_logging(error, "  csm_init_lib rc= %d, Initialization failed. Success is required to be able to communicate between library and daemon. Are the daemons running?",return_value);

        csm_free_struct_ptr(csmi_allocation_t, allocation);
		return return_value;
	}
    
    // Execute the api
    return_value = csm_allocation_create(&csm_obj, allocation);
    if ( return_value == CSMI_SUCCESS ) 
    {
    
        printf("---\nallocation_id: %" PRId64 "\n", allocation->allocation_id);
        printf("num_nodes: %" PRId32 "\n", allocation->num_nodes);

        for (i = 0; i < allocation->num_nodes; i++) 
        {
            printf("- compute_nodes: %s\n", allocation->compute_nodes[i]);
        }

        printf("user_name: %s\n", allocation->user_name);
        printf("user_id: %" PRId32 "\n", allocation->user_id);
        printf("state: %s\n", csm_get_string_from_enum(csmi_state_t, allocation->state));
        printf("type: %s\n", csm_get_string_from_enum(csmi_allocation_type_t,allocation->type));
        printf("job_submit_time: %s\n...\n", allocation->job_submit_time);
    }
    else 
    {
        printf("%s FAILED: returned: %d, errcode: %d errmsg: %s\n", argv[0], return_value,
             csm_api_object_errcode_get(csm_obj),
             csm_api_object_errmsg_get(csm_obj));
    }

    // Free up the csm object and our struct.
    csm_api_object_destroy(csm_obj);
    csm_free_struct_ptr(csmi_allocation_t, allocation);

    // Cleanup the library and print the error.
	int lib_return_value = csm_term_lib();
	if( lib_return_value != 0 )
    {
		csmutil_logging(error, "csm_term_lib rc= %d, Initialization failed. Success "
            "is required to be able to communicate between library and daemon. Are the "
            "daemons running?", lib_return_value);
		return lib_return_value;
	}

    return return_value;
}
