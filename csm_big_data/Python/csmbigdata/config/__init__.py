# encoding: utf-8
# ================================================================================
#
#  __init__.py
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

import logging
try: # Prevent the logger from failing if NullHandler isn't defined.
    from logging import NullHandler
except:
    class NullHandler (logging.Handler):
        def emit(record):
            pass
logging.getLogger(__name__).addHandler(logging.NullHandler())

__all__=['default_settings', 'settings', 'settings_option']
