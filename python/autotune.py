import librosa
import librosa.display
import numpy as np
import soundfile as sf
import scipy.signal as sig
import psola

def obterPitchAproximado(value, scale):
    if np.isnan(value): return np.nan

    degrees = librosa.key_to_degrees(scale)

    # if the value is very close to the start of the next octave,
    # ensure that we round up correctly 
    degrees = np.concatenate((degrees, [degrees[0] + 12]))

    midi_note = librosa.hz_to_midi(value)
    degree = librosa.hz_to_midi(value) % 12
    closest_pitch_class = np.argmin(np.abs(degrees-degree))

    # get diff btwn input vs desired pitch class
    degree_diff = degree - degrees[closest_pitch_class]

    midi_note -= degree_diff
    return librosa.midi_to_hz(midi_note) # convert back to hertz


def calcularPitchCorreto(f0, scale):
    closest = np.zeros_like(f0)

    # for each pitch, get the closest pitch on the scale
    for i in range(f0.shape[0]):  closest[i] = obterPitchAproximado(f0[i], scale)

    # smooth over time
    med_filtered = sig.medfilt(closest, kernel_size=11)
    med_filtered[np.isnan(med_filtered)] = closest[np.isnan(med_filtered)]
    return med_filtered


def autotune(inputPath, scale, freqmin='C2', freqmax='C7'):
    y, sr = librosa.load(inputPath, mono=True)
    fmin, fmax = librosa.note_to_hz(freqmin), librosa.note_to_hz(freqmax) # recommended by pyin docs
    frame_length = 2048
    hop_length = frame_length // 4

    f0, voiced_flag, voiced_prob = librosa.pyin(y, fmin=fmin, fmax=fmax, sr=sr, frame_length=frame_length, hop_length=hop_length) # 1. pitch detection using PYIN
    corrected_f0 = calcularPitchCorreto(f0, scale)                                                                                # 2. calculate the correct pitch
    pitch_shifted = psola.vocode(y, sample_rate=int(sr), target_pitch=corrected_f0, fmin=fmin, fmax=fmax)                         # 3. pitch shifting using PSOLA

    sf.write(str(f"tempAutotune.wav"), pitch_shifted, sr)