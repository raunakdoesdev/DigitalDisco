# import matplotlib.pyplot as plt
from scipy.signal import stft
from scipy.io import wavfile
import numpy as np
import time
def H(x):
    return (x+abs(x))/2

def spectral_difference(X):
    #stft shape: (freqs, times/windows): X[k,n]
    T = X.shape[1] #number of windows
    N = X.shape[0] #number of freqs
    SD = np.zeros(T) #one for each window
    abs_x = abs(X)
    for n in range(T):
        if n == 0:
            SD[n] = sum((H(abs_x[:,n]))**2 )
        else: 
            SD[n] = np.sum(   (H( abs_x[:,n]-abs_x[:,n-1]) )**2)
    return SD

def find_peaks(x, t, threshold, min_spacing):
    '''
    returns numpy array of indices of the peaks
    '''
    x = x.copy()
    output = []
    cand = max(x)
    i_find = {elt: i for i, elt in enumerate(x)}
    while cand > threshold:
        i = i_find[cand]
        # output_times.append(t[i])
        output.append(i)
        for j in range(i-min_spacing+1, i+min_spacing):
            if 0 <= j < len(x):
                x[j] = 0
        cand = max(x)
    return sorted(output)


def k_for_note(X, m_start, m_stop):
    max_k = [k_at_time(X, m) for m in range(m_start+1, m_stop)]
    freqs = dict()
    for t, k in enumerate(max_k):
        if k not in freqs:
            freqs[k] = [1, t]
        else:
            freqs[k][0] += 1
    max = [0,0,1000000] #k, f, t
    for k in freqs:
        f = freqs[k][0]
        t = freqs[k][1]
        if f > max[1]:
            max = [k,f,t]
        elif f == [1]:
            if t < max[2]:
                max = [k,f,t]
    return max[0]

# def plot_spectral_difference(file, window_size=2048, step_size=256):
#     sampling_rate,samples = wav_read(file)
#     f,t,X = stft(samples,sampling_rate,nperseg=window_size, noverlap=(window_size-step_size) )
#     print('here')
#     SD = spectral_difference(X)
#     plt.plot(t,SD)
#     plt.show()

'''
wav_read/write: source: 6.003 library
'''
_normalize_funcs = {
    "int32": lambda d: d / 2147483648,
    "int16": lambda d: d / 32768,
    "uint8": lambda d: d / 256 - 0.5,
}


def wav_read(fname):
    """
    Read a wave file.  This will always convert to mono. (from 6.003 lib)

    Arguments:
      * fname: a string containing a file name of a WAV file.

    Returns a tuple with 2 elements:
      * a 1-dimensional NumPy array with floats in the range [-1, 1]
        representing samples.  the length of this array will be the number of
        samples in the given wave file.
      * an integer containing the sample rate
    """
    # load in data, convert to float
    fs, data = wavfile.read(fname)
    type_ = data.dtype.name
    data = data.astype(float)

    # convert stereo to mono
    if len(data.shape) > 2:
        raise Exception("too many channels!")
    elif len(data.shape) == 2:
        data = data.mean(1)

    # normalize to range [-1, 1]
    try:
        data = _normalize_funcs[type_](data)
    except KeyError:
        pass

    return fs,data


def wav_write(samples, fs, fname):
    """
    Write a wave file (from 6.003 lib)

    Arguments:
      * samples: a Python list of numbers in the range [-1, 1], one for each
                 sample in the output WAV file.  Numbers in the list that are
                 outside this range will be clipped to -1 or 1.
      * fs: an integer representing the sampling rate of the output
            (samples/second).
      * fname: a string containing a file name of the WAV file to be written.
    """
    out = np.array(samples)
    out[out > 1.0] = 1.0
    out[out < -1.0] = -1.0
    out = (out * 32767).astype("int16")
    wavfile.write(fname, fs, out)


def beats(file, window_size=2048, step_size=512):
    start_time = time.time()
    sampling_rate,samples = wav_read(file)
    print(sampling_rate, samples.shape)
    f,t,X = stft(samples,sampling_rate,nperseg=window_size, noverlap=(window_size-step_size) )
    print(time.time()-start_time)
    SD = spectral_difference(X)
    min_spacing = int(0.23*sampling_rate/step_size) #TODO must tune
    print(min_spacing)
    threshold = 0.003 #TODO must tune 
    print(time.time()-start_time)
    indices = find_peaks(SD, t, threshold, min_spacing)
    if indices[0] != 0:
        indices = [0] + indices
    indices.append(len(t)-1)
    times = np.take(t, indices)
    print(time.time()-start_time)
    return times


def create_beat_track(file, window_size=2048, step_size=512):
    sampling_rate,samples = wav_read(file)
    times = beats(file, window_size=window_size, step_size=step_size)
    drum_sampling_rate,drum = wav_read('snare_8k_clipped.wav')
    len_beat_track = int(len(samples)/sampling_rate*drum_sampling_rate)
    output = np.zeros(len_beat_track)
    for i in range(len(times)-1):
        m_start = times[i]
        m_end = times[i+1]
        # print(freq)
        start_sample = int(m_start *drum_sampling_rate)
        end_sample = int(m_end *drum_sampling_rate)
        for n in range(start_sample, end_sample):
            if (n-start_sample)< len(drum):
                output[n] = drum[n-start_sample]
    f = file[:-4] + '_beats.wav'
    wav_write(output,drum_sampling_rate,f )




'''
Source: f_pitch, pool_pitch, compute_spec_log_freq
https://www.audiolabs-erlangen.de/resources/MIR/FMP/C3/C3S1_SpecLogFreq-Chromagram.html
'''
def f_pitch(p, pitch_ref=69, freq_ref=440.0):
    """Computes the center frequency/ies of a MIDI pitch

    Notebook: C3/C3S1_SpecLogFreq-Chromagram.ipynb

    Args:
        p (float): MIDI pitch value(s)
        pitch_ref (float): Reference pitch (default: 69)
        freq_ref (float): Frequency of reference pitch (default: 440.0)

    Returns:
        freqs (float): Frequency value(s)
    """
    return 2 ** ((p - pitch_ref) / 12) * freq_ref

def pool_pitch(p, Fs, N, pitch_ref=69, freq_ref=440.0):
    """Computes the set of frequency indices that are assigned to a given pitch

    Notebook: C3/C3S1_SpecLogFreq-Chromagram.ipynb

    Args:
        p (float): MIDI pitch value
        Fs (scalar): Sampling rate
        N (int): Window size of Fourier fransform
        pitch_ref (float): Reference pitch (default: 69)
        freq_ref (float): Frequency of reference pitch (default: 440.0)

    Returns:
        k (np.ndarray): Set of frequency indices
    """
    lower = f_pitch(p - 0.5, pitch_ref, freq_ref)
    upper = f_pitch(p + 0.5, pitch_ref, freq_ref)
    k = np.arange(N // 2 + 1)
    k_freq = k * Fs / N  # F_coef(k, Fs, N)
    mask = np.logical_and(lower <= k_freq, k_freq < upper)
    return k[mask]

def compute_spec_log_freq(Y, Fs, N):
    """Computes a log-frequency spectrogram

    Notebook: C3/C3S1_SpecLogFreq-Chromagram.ipynb

    Args:
        Y (np.ndarray): Magnitude or power spectrogram
        Fs (scalar): Sampling rate
        N (int): Window size of Fourier fransform

    Returns:
        Y_LF (np.ndarray): Log-frequency spectrogram
        F_coef_pitch (np.ndarray): Pitch values
    """
    Y_LF = np.zeros((128, Y.shape[1]))
    for p in range(128):
        k = pool_pitch(p, Fs, N)
        Y_LF[p, :] = Y[k, :].sum(axis=0)
    F_coef_pitch = np.arange(128)
    return Y_LF, F_coef_pitch

def compute_chromagram(Y_LF):
    """Computes a chromagram

    Notebook: C3/C3S1_SpecLogFreq-Chromagram.ipynb

    Args:
        Y_LF (np.ndarray): Log-frequency spectrogram

    Returns:
        C (np.ndarray): Chromagram
    """
    C = np.zeros((12, Y_LF.shape[1]))
    p = np.arange(128)
    for c in range(12):
        mask = (p % 12) == c
        C[c, :] = Y_LF[mask, :].sum(axis=0)
    return C


def colors_and_beats(file, window_size=2048, step_size=512):
    start_time = time.time()
    fs,samples = wav_read(file)
    # print(fs, samples.shape)
    f,t,X = stft(samples,fs,nperseg=window_size, noverlap=(window_size-step_size) )
    SD = spectral_difference(X)
    min_spacing = int(0.23*fs/step_size) #TODO must tune
    threshold = 0.003 #TODO must tune 
    # print(time.time()-start_time)
    indices = find_peaks(SD, t, threshold, min_spacing)
    if indices[0] != 0:
        indices = [0] + indices
    indices.append(len(t)-1)
    times = np.take(t, indices)
    # print(time.time()-start_time)
    Y = np.abs(X)**2
    Y_LF, F_coef_pitch = compute_spec_log_freq(Y, fs, window_size)
    chroma = compute_chromagram(Y_LF)
    max_k = np.argmax(Y_LF[0:95], axis=0)
    # max_k = np.argmax(chroma, axis=0)
    # print(Y_LF.shape, max_k.shape)
    #calculating max per note
    colors = np.zeros(len(indices)-1)
    resynth_fs = 8000
    resynth_output = np.zeros(int(times[-1]+1)*resynth_fs)
    for i in range(len(indices)-2):
        m_start = indices[i]
        m_end = indices[i+1]
        temp = max_k[m_start:m_end]
        colors[i] = np.amax(temp)
        # start_sample = int(times[i]*resynth_fs)
        # end_sample = int(times[i+1]*resynth_fs)
        # # freq = f_pitch(colors[i]+60)
        # freq = f_pitch(colors[i])
        # for n in range(start_sample, end_sample):
        #     resynth_output[n] = (0.5*np.cos(2*np.pi*freq/resynth_fs*n))
    # print(time.time()-start_time)
    # f = file[:-4] + '_colors.wav'
    # print(resynth_output.shape)
    # wav_write(resynth_output, resynth_fs, f)
    #evenly spread out across the color spectrum
    offset = np.amin(colors[:-1])
    diff = np.amax(colors)-offset
    colors_norm = np.clip((colors-offset)*255/diff, 0, 255)
    colors =colors_norm.astype(int)
    num_samples = len(colors)
    # print(num_samples)
    out_times = str(num_samples+1) + ','#str(len(times)) + ','
    for t in times[:num_samples+1]:
        out_times += '{:.4f}'.format(t) + ','
    out_colors = str(num_samples) + ','#str(len(colors)) + ','
    for c in colors[:num_samples]:
        out_colors += str(c) + ','
    # print(time.time()-start_time)
    return out_times[:-1]+'\n' + out_colors[:-1] + '\n '

