import os
flags = ['-g', '-O2']

# Make warnings fatal, and warn about a lot.
flags += ['-Wall', '-Wextra']
flags += ['-Werror']

# However, unused parameters are quite common in OO.
flags += ['-Wno-unused']

# flags += ['-fdump-class-hierarchy']

# This option is useful, but Boost violates the warnings, so it isn't
# particularly practical to enable for most things.
# flags += ['-Weffc++']

# Enable for slower compilation and possibly smaller executables.
# TODO: This doesn't currently work with the separate compilation.
# flags += ['-frepo']

env = Environment(ENV = os.environ, CXXFLAGS = flags,
	CPPPATH=['#lib'],
	LIBPATH=['#build/lib'])
#print env.Dump()

Export('env')
SConscript('lib/SConscript',
	variant_dir='build/lib', duplicate=0)
main = SConscript('main/SConscript',
	variant_dir="build/main", duplicate=0)
test = SConscript('test/SConscript',
	variant_dir="build/test", duplicate=0)

test_run = env.Command('test', test, "./$SOURCE")
AlwaysBuild(test_run)
Default(main, test)

# asure = env.Program('asure', Glob('*.cc'),
# 	LIBS=['crypto', 'gc', 'boost_iostreams'])
# Clean(asure, Glob('*.rpo'))
