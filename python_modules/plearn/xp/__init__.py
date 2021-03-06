"""xp 0.1

usage::
    xp [mode [options]] expkey*

Default mode is listdir

Supported modes are:

    1. L{duplicates}:
    2. L{listdir}:  
    3. L{group}:    
    4. L{running}:  

Type 'xp mode --help' for further help on a mode
"""
__version_id__ = '$Id$'

import new, os, sys
from plearn import pyplearn
from Experiment import *
from ConfigParser import ConfigParser
from plearn.vmat.PMat import PMat
from plearn.utilities.moresh import *
from plearn.utilities.toolkit import vsystem
from plearn.utilities.ModeAndOptionParser import *

#
#  Classes
#
class XpMode( Mode ):
    pass

## class link( XpMode ):
##     def __init__( self, targets, options ):
##         for target in targets:
##             dirlist = os.listdir( target )
##             for fname in dirlist:
##                 if fname.startswith( Experiment._expdir_prefix ):
##                     exppath = os.path.join( target, fname )
##                     os.symlink
                        
class merge( XpMode ):
    """\ls -1 Rank*/expdir* | grep expdir > tmp.sh"""
    def option_groups( cls, parser ):
        return directory_options( cls, parser )
    option_groups = classmethod( option_groups )
    

    def __init__( self, targets, options ):
        super(merge, self).__init__(targets, options)
        if options.name:
            dirname = options.name
        else:
            dirname = 'Merge_%s' % '_'.join(targets)

        if not os.path.isdir( dirname ):
            os.makedirs( dirname )

        for target in targets:
            dirlist = os.listdir( target )
            for fname in dirlist:
                if fname.startswith( Experiment._expdir_prefix ):
                    migrate( os.path.join(target, fname), dirname, options.move )

class mkdir( XpMode ):
    def __init__( self, targets, options ):
        super(mkdir, self).__init__(targets, options)
        dirname = targets[0]
        
        vsystem( 'mkdir %s'             % dirname )
        vsystem( 'mkdir %s/Experiments' % dirname )
        vsystem( 'mkdir %s/Reports'     % dirname )
        
        config = ConfigParser( )
        config.add_section( 'EXPERIMENTS' )
        config.set( 'EXPERIMENTS', 'expdir_root', 'Experiments' )
        config.set( 'EXPERIMENTS', 'report_root', 'Reports'     )

        config_path = '%s/.pyplearn' % dirname
        config_fp   = open( config_path, 'w' )
        config.write( config_fp )        
        config_fp.close()
        print '+++', config_path, 'created.'
        
        lname  = '%s/dispatch' % dirname
        import dispatch_template
        template_file = dispatch_template.__file__.replace( '.pyc', '.py' )
        vsystem( 'cp %s %s' % (template_file, lname) )
        vsystem( 'chmod u=rwx %s' % lname )
        print '+++', lname, 'created.'
        

class running( XpMode ):
    def __init__( self, targets, options ):
        super(running, self).__init__(targets, options)
        assert not targets
        
        r = 0
        for exp in ls():
            if exp.startswith( Experiment._expdir_prefix ) and \
               not os.path.exists( os.path.join( exp, Experiment._metainfos_fname ) ):
                if r == 0:
                    print
                r += 1
                print exp

        print( "\n(%d experiment%s %s running)"
               % ( r, toolkit.plural(r),
                   toolkit.plural(r, 'is', 'are') )
               )
        

#
#  Modes Parsing ExpKeys
#
class ExpKeyMode( XpMode ):
    def __init__( self, targets, options ):
        super(ExpKeyMode, self).__init__(targets, options)
        # MOVED IN Experiment.cache_experiments...
        # exproot = pyplearn.config.get_option( 'EXPERIMENTS', 'expdir_root' )
        # if exproot:
        #     os.chdir( exproot )
        self.routine( ExpKey( targets ), options, Experiment.match( targets ) )

class duplicates( ExpKeyMode ):
    def aliases( cls ):
        return ['dup']
    aliases = classmethod( aliases )

    def option_groups( cls, parser ):
        duplicates_options = OptionGroup( parser, "Mode Specific Options --- %s" % cls.__name__,
                                          "Available under %s mode only." % cls.__name__ )

        duplicates_options.add_option( "--sh",
                                       default = False,
                                       action  = 'store_true',
                                       help    = ""
                                       )

        return [ duplicates_options ]
    option_groups = classmethod( option_groups )

    def routine( self, expkey, options, experiments ):
        if options.sh:
            shfile = open('duplicates.sh', 'w')
            
        while experiments:
            exp = experiments.pop()
            duplicates = []
            for x in experiments:
                if keycmp(x, exp, expkey) == 0:
                    duplicates.append( x.path )
            if duplicates:
                if options.sh:
                    shfile.write( 'rm -rf ' + "\nrm -rf ".join(duplicates) + '\n' )
                else:
                    print exp.toString( expkey, True )
                    print "Duplicated by", " ".join(duplicates)
                    print

class listdir( ExpKeyMode ):
    """List matching experiments directory. B{Default mode.}
    
    Among experiments in the current directory, this mode lists the
    experiments that matches the experiment key::

        xp [listdir [key[=value]]*]

    Providing the listdir mode name is optional since the listdir mode is
    the default one.

    If no key is provided, all experiments in the directory are listed and
    their settings reported. If a key is provided without value, all
    experiment having the given key in their settings are matched.
    """
    def aliases( cls ):
        return ['ls']
    aliases = classmethod( aliases )
    
    def routine( self, expkey, options, experiments ):
        experiments.sort( lambda x1, x2: keycmp(x1, x2, expkey) )
        if options.full or not expkey:
            print "\n","\n".join([ str(x) for x in experiments ])
        else:
            for x in experiments:
                print '\n%s' % x.toString( expkey )
            
        print "(%d experiments)" % len(experiments)

    def option_groups( cls, parser ):
        listdir_options = OptionGroup( parser, "Mode Specific Options --- %s" % cls.__name__,
                                     "Available under listdir mode only." )

        listdir_options.add_option( "--full",
                                  default = False,
                                  action  = 'store_true',
                                  help    = "Listdir's default behaviour is to restrict the printing of the metainfos to "
                                    "the key provided as target. Use this option to print all metainfos."                       
                                  )
        
        return [ listdir_options ]
    option_groups = classmethod( option_groups )


class group( ExpKeyMode ):
    def option_groups( cls, parser ):
        return directory_options( cls, parser )
    option_groups = classmethod( option_groups )
    
    def routine( self, expkey, options, experiments ):    
        reffunc = os.symlink
        if options.move:
            reffunc = lambda src,dest: os.system("mv %s %s"%(src,dest))        

        for exp in experiments:
            subkey = exp.getKey( expkey )
            if options.name is None:
                dirname = "_".join([ "%s=%s" % (lhs, str(rhs))
                                     for (lhs, rhs) in subkey.iteritems() ])
            else:
                dirname = options.name

            if not exp.path.startswith( Experiment._expdir_prefix ):
                dirname = os.path.join( os.path.dirname(exp.path), dirname )

            if not os.path.exists( dirname ):
                os.mkdir( dirname )

            pushd( dirname )
            if not os.path.exists( exp.path ):
                expdir = os.path.basename( exp.path )
                assert expdir.startswith( Experiment._expdir_prefix ), expdir
                try:
                    reffunc( os.path.join('..',expdir), expdir )
                except OSError, err:
                    raise OSError( '%s\n in %s\n with expdir=%s'
                                   % (str(err), os.getcwd(), expdir)
                                   )
            popd( )

#
#  Helper functions
#
def directory_options( cls, parser ):
    directory_options = OptionGroup( parser, "Mode Specific Options --- %s" % cls.__name__,
                                     "Available under %s mode only." % cls.__name__ )

    directory_options.add_option( "--move",
                                  default = False,
                                  action  = 'store_true',
                                  help    = "Should the experiments directory be moved (instead of linked) in "
                                  "the created directory."                       
                                  )
    
    directory_options.add_option( "--name",
                                  default = None,
                                  help    = "The name that should be given the created directory. The default "
                                  "name is built from the experiment key."                       
                                  )

    return [ directory_options ]


#            
#  Main program
#
def main( xp_version = lambda : '0.1', default_mode_name='listdir' ):
    parser = ModeAndOptionParser( usage = ( "%prog [mode [options]] expkey*\n" +
                                            (' '*7) +"Default mode is listdir" ),
                                  version = "%prog " + xp_version( )   )

    #
    # Actual Launch
    #
    options, targets = parser.parse_args( default_mode_name=default_mode_name )

    print "xp %s" % xp_version( )    
    parser.selected_mode( targets, options )

if __name__ == '__main__':
    main( )
