#!/usr/bin/env python2.3
__cvs_id__ = "$Id$"

import sys
import os.path
from plearn.pyplearn             import *
from plearn.pyplearn.pyplearn    import _parse_plargs
from plearn.pyplearn.plearn_repr import *

# Add the absolute directory portion of the current script to the path
sys.path = [os.path.dirname(os.path.abspath(sys.argv[1]))] + sys.path

pyplearn_file = open(sys.argv[1], 'U')
lines = pyplearn_file.read()
pyplearn_file.close()

if len(sys.argv) == 3 and sys.argv[2] == '--help':
    # Simply print the docstring of the pyplearn script
    lines += 'print __doc__\n'
elif len(sys.argv) >= 3 and '--PyPLearnScript' in sys.argv:
    # This is the mecanism used inside the run command to manage logs to
    # expdirs.
    sys.argv.remove('--PyPLearnScript')
    _parse_plargs(sys.argv[2:])
    lines += 'print PyPLearnScript( main() )\n'
else:
    # Default mode: simply dump the plearn_representation of this
    _parse_plargs(sys.argv[2:])
    lines += 'print main()\n'

del sys, os.path, _parse_plargs
exec ''.join(lines) in globals()
