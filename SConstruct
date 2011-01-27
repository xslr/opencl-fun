
env = Environment()
Repository('./common')
env.ParseConfig('pkg-config --cflags --libs glib-2.0')
env.Append( LIBS='OpenCL', CPPDEFINES='PROFILING', CPPPATH='#/include', CFLAGS = Split("-g -O0 -Wall"))
env.Append()

Export('env')

SConscript('common/SConscript', variant_dir='work/common', duplicate=0)

SConscript('mdct/SConscript', variant_dir='work/mdct', duplicate=0)
#SConscript('fft/SConscript', variant_dir='work/fft', duplicate=0)

SConscript('tests/SConscript', variant_dir='work/tests', duplicate=0)
Import('mdct_test')

#env.Install('build/fft', 'fft/fft_2048.cl')
#env.Install('build', 'mdct/mdct.h')
env.Install('build', mdct_test)
env.Install('build', 'mdct/mdct_2048.cl')
