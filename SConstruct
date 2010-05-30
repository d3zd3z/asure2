import os
flags = ['-g', '-O2']

# Make warnings fatal, and warn about a lot.
flags += ['-Wall', '-Werror', '-Wextra']

# However, unused parameters are quite common in OO.
flags += ['-Wno-unused']

# flags += ['-fdump-class-hierarchy']

# This option is useful, but Boost violates the warnings, so it isn't
# particularly practical to enable for most things.
# flags += ['-Weffc++']

# Enable for slower compilation and possibly smaller executables.
# flags += ['-frepo']

env = Environment(ENV = os.environ, CXXFLAGS = flags)
#print env.Dump()

asure = env.Program('asure', Glob('*.cc'),
	LIBS=['crypto', 'gc', 'boost_iostreams'])
Clean(asure, Glob('*.rpo'))
