import sox
from autotune import autotune
from os import remove

def chorus(inputPath):
    tfm = sox.Transformer()
    tfm.pitch(n_semitones=8.3, quick=False)
    tfm.pad(start_duration=0.3, end_duration=1.9)
    tfm.chorus(
    gain_in=0.5,  # Ganho de entrada
    gain_out=0.6,  # Ganho de saída
    delays=[23, 20],  # Atrasos em ms
    decays=[0.4, 0.7],  # Decaimento (como o sinal enfraquece)
    speeds=[0.4, 0.3],  # Velocidade de modulação
    depths=[0.55, 0.8],  # Profundidade do efeito
    n_voices=2  # Número de vozes no chorus
    )
    reverb_params = {
    'reverberance': 80,
    'room_scale': 10,
    'pre_delay': 50,
    'wet_gain': -10,
    'stereo_depth': 100,
    'high_freq_damping': 50
     }
    tfm.reverb(**reverb_params)
    tfm.build(inputPath, 'chorus.wav')
    
def reverbRoom1(inputPath):
    tfm = sox.Transformer()
    reverb_params = {
    'reverberance': 80,
    'room_scale': 90,
    'pre_delay': 50,
    'wet_gain': -10,
    'stereo_depth': 100,
    'high_freq_damping': 50
     }
    tfm.reverb(**reverb_params)
    tfm.build(inputPath, 'reverbRoom1.wav')
    
def radioVoice(inputPath):
    tfm = sox.Transformer()
    tfm.pitch(n_semitones=8, quick=False)
    tfm.pad(start_duration=0.6, end_duration=1.7)
    reverb_params = {
        'reverberance': 90, 'room_scale': 90, 'pre_delay': 50,
        'wet_gain': -10, 'stereo_depth': 100, 'high_freq_damping': 50
    }
    tfm.reverb(**reverb_params)
    tfm.vol(0.5)
    tfm.build(inputPath, 'radioVoice.wav')

def vocal(inputPath):
    tfm = sox.Transformer()
    tfm.vol(0.22)
    tfm.pad(start_duration=0.6, end_duration=0.1)
    tfm.build(inputPath, 'vocal.wav')

def revStageAB(inputPath):
    autotune(inputPath, scale='A:major')
    tfm = sox.Transformer()
    tfm.pad(start_duration=0.75, end_duration=1.05)
    reverb_params = {
    'reverberance': 90,
    'room_scale': 90,
    'pre_delay': 50,
    'wet_gain': -10,
    'stereo_depth': 100,
    'high_freq_damping': 50
     }
    tfm.reverb(**reverb_params)
    tfm.vol(0.3)
    tfm.build("tempAutotune.wav", 'revStageAB.wav')
    remove("tempAutotune.wav")
    
def revStageC(inputPath):
    autotune(inputPath, scale='C:major')
    tfm = sox.Transformer()
    tfm.pad(start_duration=0.75, end_duration=1.05)
    reverb_params = {
    'reverberance': 90,
    'room_scale': 80,
    'pre_delay': 50,
    'wet_gain': -10,
    'stereo_depth': 100,
    'high_freq_damping': 50
    }
    tfm.reverb(**reverb_params)
    tfm.vol(0.5)
    tfm.build("tempAutotune.wav", 'revStageC.wav')
    remove("tempAutotune.wav")
    
def revStageE(inputPath):
    autotune(inputPath, scale='E:major')
    tfm = sox.Transformer()
    tfm.pad(start_duration=1, end_duration=1.8)
    reverb_params = {
    'reverberance': 90,
    'room_scale': 80,
    'pre_delay': 50,
    'wet_gain': -10,
    'stereo_depth': 100,
    'high_freq_damping': 50
    }
    tfm.reverb(**reverb_params)
    tfm.vol(0.6)
    tfm.build("tempAutotune.wav", 'revStageE.wav')
    remove("tempAutotune.wav")
    
def revStageG(inputPath):
    autotune(inputPath, scale='D:major')
    tfm = sox.Transformer()
    tfm.pad(start_duration=0.75, end_duration=1.05)
    reverb_params = {
    'reverberance': 90,
    'room_scale': 80,
    'pre_delay': 50,
    'wet_gain': -10,
    'stereo_depth': 100,
    'high_freq_damping': 50
    }
    tfm.reverb(**reverb_params)
    tfm.vol(0.5)
    tfm.build("tempAutotune.wav", 'revStageG.wav')
    remove("tempAutotune.wav")
