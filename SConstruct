env = Environment()
Export('env')

SConscript('fft/SConscript', build_dir='build/fft', duplicate=0)
env.Install('build/fft', 'fft/fft_2048.cl');

