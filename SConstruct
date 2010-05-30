import os
flags = ['-g', '-O2', '-Wall', '-Werror']
# flags += ['-frepo']
env = Environment(ENV = os.environ, CXXFLAGS = flags)
#print env.Dump()

asure = env.Program('asure', Glob('*.cc'),
	LIBS=['crypto', 'gc', 'boost_iostreams'])
Clean(asure, Glob('*.rpo'))
