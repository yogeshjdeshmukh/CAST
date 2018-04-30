#!/bin/python
# encoding: utf-8
# ================================================================================
#
# csm_bds_context.py
#
#  © Copyright IBM Corporation 2015,2016. All Rights Reserved
#
#    This program is licensed under the terms of the Eclipse Public License
#    v1.0 as published by the Eclipse Foundation and available at
#    http://www.eclipse.org/legal/epl-v10.html
#
#    U.S. Government Users Restricted Rights:  Use, duplication or disclosure
#    restricted by GSA ADP Schedule Contract with IBM Corp.
#
# ================================================================================
'''
.. module::csm_bds_context
:platform: Linux
:synopsis: Sets up the csm_bds_context for development environments, should not affect production.
.. moduleauthor:: John Dunham (jdunham@us.ibm.com)
'''

import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

import logging
import logging.config

DEFAULT_LOG_CONF = dict({
    'version' : 1,
    'disable_existing_loggers': False,
    'formatters' : {
        'f':{ 
            'format' : '%(levelname)s!%(name)s[%(lineno)d]! %(message)s' 
        }
    },
    'handlers' : {
        'h':{ 
            'class'   : 'logging.StreamHandler',
            'formatter' : 'f',
            'level'     : logging.INFO
        }
    },

    'root': {
        'handlers' : ['h'],
        'level'    : logging.INFO,
        'propagate': True
    }
}
)

def load_default_log_conf():
    logging.config.dictConfig(DEFAULT_LOG_CONF)

