import os
env = Environment(ENV = os.environ, CXXFLAGS = ['-g', '-O2', '-Wall', '-Werror', '-frepo'])
#print env.Dump()

asure = env.Program('asure', Glob('*.cc'),
	LIBS=['crypto', 'gc'])
Clean(asure, Glob('*.rpo'))
