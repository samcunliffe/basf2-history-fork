import os, re, platform
from distutils import sysconfig
import config


# get the sub directory name for architecture and build option dependent files
if not os.environ.has_key('BELLE2_SUBDIR'):
    print 'Belle II software environment is not set up.'
    print '-> Execute "setuprel" in your local release directory.'
    Exit(1)
subdir = os.environ['BELLE2_SUBDIR']

# create construction environment
global_env = Environment(ENV = os.environ,
                         tools = ['default', 'root_dict', 'process_dir'],
                         BUILDERS = {},
                         BUILDDIR = os.path.join('#build', subdir),
                         INCDIR = '#include',
                         LIBDIR = os.path.join('#lib', subdir),
                         BINDIR = os.path.join('#bin', subdir),
                         DATADIR = '#data',
                         EXTDIR = '#externals',
                         EXTINCDIR = os.path.join('$EXTDIR', 'include'),
                         EXTLIBDIR = os.path.join('$EXTDIR', 'lib', subdir),
                         EXTBINDIR = os.path.join('$EXTDIR', 'bin', subdir),
                         CCFLAGS = ['-Wall'],
                         CPPDEFINES = {'_PACKAGE_': '\\"$PACKAGE\\"'},
                         CPPPATH = ['include', '$EXTINCDIR', sysconfig.get_python_inc()],
                         LIBPATH = ['$LIBDIR', '$EXTLIBDIR'],
                         TOOLS_LIBS = {},
                         TOOLS_LIBPATH = {})

# check for debug flag
option = os.environ['BELLE2_OPTION']
if option == 'debug':
    global_env.Append(CCFLAGS = '-g')
elif option == 'opt':
    global_env.Append(CCFLAGS = '-O3')
else:
    print 'Aborted because of unknown build option:', option
    Exit(1)

# use central release as repository if it is set up
repository = None
if os.environ.has_key('BELLE2_RELEASE_DIR'):
    repository = os.environ['BELLE2_RELEASE_DIR']
    global_env.Repository(repository)

# set variant directory for object files
VariantDir(global_env['BUILDDIR'], '.', duplicate = 0)


# check for required packages and configure the environment
if (not global_env.GetOption('clean')) and (not global_env.GetOption('help')):
    if not config.configure(global_env):
        Exit(1)

# loop over directory entries and filter out package directories
exclude_dirs = ['build', 'include', 'lib', 'bin', 'data']
local_packages = []
for entry in os.listdir(os.getcwd()):
    if entry.find('.') > -1 or os.path.isfile(entry) or entry in exclude_dirs:
        continue

    # we are in a package directory
    global_env['PACKAGE'] = entry
    local_packages.append(entry)

    global_env.ProcessDirectory(entry)
