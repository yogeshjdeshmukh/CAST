/*================================================================================

    csmi/src/bb/src/csmi_bb_cmd.c

  © Copyright IBM Corporation 2015-2017. All Rights Reserved

    This program is licensed under the terms of the Eclipse Public License
    v1.0 as published by the Eclipse Foundation and available at
    http://www.eclipse.org/legal/epl-v10.html

    U.S. Government Users Restricted Rights:  Use, duplication or disclosure
    restricted by GSA ADP Schedule Contract with IBM Corp.

================================================================================*/
/*
 * Author: Nick Buonarota
 * Email:  nbuonar@us.ibm.com
 */
 
/*C includes*/
#include <string.h>
#include <ctype.h>

/*CSM includes*/
/*Needed for structs and functions*/
#include "csmi/include/csm_api_burst_buffer.h" 

/*Needed for infrastructure*/
#include "csmi/src/common/include/csmi_api_internal.h" 
 
#include "csmi/src/common/include/csmi_common_utils.h" 

/*Needed for infrastructure logging*/
#include "csmutil/include/csmutil_logging.h"
#include "csmutil/include/timing.h"

#define API_PARAMETER_INPUT_TYPE csm_bb_cmd_input_t 
#define API_PARAMETER_OUTPUT_TYPE csm_bb_cmd_output_t

// Performs a test to see if a string is alpha_numeric.
#define flag_test( str, debug_str ) \
    if ( str ) {                    \
        char* current = str;        \
        char  illegal = 0;          \
                                    \
        for ( ; !illegal && *current; current++ )   \
            illegal |= !( isalnum(*current)      || \
            *current == ' ' || *current == '.'   || \
            *current == '_' || *current == '-'   || \
            *current == '/' || *current == '='   || \
            *current == ',' || *current == ':'   || \
            *current == '^');                       \
                                                    \
        if ( illegal ) {                                                \
            csmutil_logging(error,                                      \
            "%s-%d: Invalid "                                           \
            debug_str                                                   \
            " parameter: Character '%c' is not a legal character",      \
                __FILE__, __LINE__, *(current-1));                      \
            csm_api_object_errcode_set(*handle, CSMERR_INVALID_PARAM); \
            csm_api_object_errmsg_set(*handle,                         \
                strdup( csm_get_string_from_enum(csmi_cmd_err_t,CSMERR_INVALID_PARAM)));        \
            return CSMERR_INVALID_PARAM;                                \
        }                                                               \
    }

const static csmi_cmd_t expected_cmd = CSM_CMD_bb_cmd;

void csmi_bb_cmd_destroy(csm_api_object *handle);

int csm_bb_cmd(
    csm_api_object** handle,
    API_PARAMETER_INPUT_TYPE* input,
    API_PARAMETER_OUTPUT_TYPE** output)
{
    START_TIMING()

    // Declare variables that we will use below.
    char     *buffer               = NULL;
    uint32_t  buffer_length        = 0;
    char     *return_buffer        = NULL;
    uint32_t  return_buffer_length = 0;
    int       error_code           = CSMI_SUCCESS;

    // EARLY RETURN
    // Create a csm_api_object and sets its csmi cmd and the destroy function
    create_csm_api_object(handle, expected_cmd, csmi_bb_cmd_destroy);

    // EARLY RETURN
    if ( !input ||
            input->node_names_count < 1 || input->node_names == NULL )
    {
        csmutil_logging(error, "Invalid parameter: 'node_names' and 'node_names_count'"
            " must represent more than one node.");

        csm_api_object_errcode_set(*handle, CSMERR_INVALID_PARAM);
        csm_api_object_errmsg_set(*handle,
            strdup(csm_get_string_from_enum(csmi_cmd_err_t,CSMERR_INVALID_PARAM)));
        
        return CSMERR_INVALID_PARAM;
    }
    
    flag_test(input->command_arguments, "command arguments")

    // EARLY RETURN
    // Construct the buffer.
    csm_serialize_struct(API_PARAMETER_INPUT_TYPE, input, &buffer, &buffer_length);
    test_serialization( handle, buffer );
    
    // Send a Message to the Backend.
    error_code = csmi_sendrecv_cmd(*handle, expected_cmd,
        buffer, buffer_length, &return_buffer, &return_buffer_length);

    if ( error_code == CSMI_SUCCESS )
    {  
        if ( csm_deserialize_struct(API_PARAMETER_OUTPUT_TYPE, output,
                    return_buffer, return_buffer_length ) == 0 )
        {
            csm_api_object_set_retdata(*handle, 1, *output);
        }
        else
        {
            csmutil_logging(error, "Deserialization failed");
            csm_api_object_errcode_set(*handle, CSMERR_MSG_UNPACK_ERROR);
            csm_api_object_errmsg_set(*handle,
                strdup(csm_get_string_from_enum(csmi_cmd_err_t, CSMERR_MSG_UNPACK_ERROR)));
            error_code = CSMERR_MSG_UNPACK_ERROR;
        }
    }    
    else
    {
        csmutil_logging(error, "csmi_sendrecv_cmd failed: %d - %s",
            error_code, csm_api_object_errmsg_get(*handle));
    }
    
    // Free the buffers.
    if ( return_buffer ) free(return_buffer);
    free(buffer);

    END_TIMING( csmapi, trace, csm_api_object_traceid_get(*handle), expected_cmd, api )
	
	return error_code;
}

void csmi_bb_cmd_destroy(csm_api_object *handle)
{
	/* Function variables. */
    csmi_api_internal *csmi_hdl;
    API_PARAMETER_OUTPUT_TYPE *output = NULL;

	/* Verify it exists */
    if (handle == NULL || handle->hdl == NULL)
    {                                                                                   
        csmutil_logging(warning, "%s-%d: csm_api_object not valid", __FILE__, __LINE__);
        return;
    }

	/* Verify its the correct obj */
    csmi_hdl = (csmi_api_internal *) handle->hdl;
    if (csmi_hdl->cmd != expected_cmd)
    {
        csmutil_logging(error, "%s-%d: Unmatched CSMI cmd\n", __FILE__, __LINE__);
        return;
    }

    /* Do the free */
	/* Set the data. */
    output = (API_PARAMETER_OUTPUT_TYPE *) csmi_hdl->ret_cdata;
	/* Free the API output struct */
	csm_free_struct_ptr(API_PARAMETER_OUTPUT_TYPE, output);
}

