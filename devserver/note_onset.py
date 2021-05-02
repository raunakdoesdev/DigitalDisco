import matplotlib.pyplot as plt
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
    for n in range(T):
        if n == 0:
            SD[n] = sum((H(abs(X[k,n])))**2 for k in range(N))
        else: 
            SD[n] = sum(   (H( abs(X[k,n])-abs(X[k,n-1])  ))**2 for k in range(N)    )
    return SD

def find_peaks(x, t, threshold, min_spacing):
    x = x.copy()
    output = []
    cand = max(x)
    i_find = {elt: i for i, elt in enumerate(x)}
    while cand > threshold:
        i = i_find[cand]
        output.append(t[i])
        for j in range(i-min_spacing+1, i+min_spacing):
            if 0 <= j < len(x):
                x[j] = 0
        cand = max(x)
    return sorted(output)

def k_at_time(X, m):
    l = [(k, abs(X_k)**2) for k, X_k in enumerate(X[m])]
    return max(l, key=lambda x: x[1])[0]


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

def plot_spectral_difference(file, window_size=2048, step_size=256):
    sampling_rate,samples = wav_read(file)
    f,t,X = stft(samples,sampling_rate,nperseg=window_size, noverlap=(window_size-step_size) )
    print('here')
    SD = spectral_difference(X)
    plt.plot(t,SD)
    plt.show()

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

# plot_spectral_difference('Never Gonna Give You Up.wav')

def beats(file, window_size=2048, step_size=512):
    start_time = time.time()
    sampling_rate,samples = wav_read(file)
    print(sampling_rate, samples.shape)
    try:
        f,t,X = stft(samples,sampling_rate,nperseg=window_size, noverlap=(window_size-step_size) )
    except:
        print(window_size, window_size-step_size)
        raise Exception
    print(time.time()-start_time)
    SD = spectral_difference(X)
    min_spacing = int(0.23*sampling_rate/step_size) #TODO must tune
    print(min_spacing)
    threshold = 0.003 #TODO must tune 
    print(time.time()-start_time)
    times = find_peaks(SD, t, threshold, min_spacing)
    print(time.time()-start_time)
    if times[0] != 0:
        times = [0] + times
    times.append(t[-1]) #append last time
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
