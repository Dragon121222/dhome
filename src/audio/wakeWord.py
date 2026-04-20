import sys, os
sys.stdout = open(os.devnull, 'w')
sys.stderr = open(os.devnull, 'w')

import sounddevice as sd
import numpy as np
import scipy.signal as signal
from openwakeword.model import Model
import socket

NATIVE_RATE = 44100
TARGET_RATE = 16000
TARGET_CHUNK = 1280  # 80ms at 16kHz
NATIVE_CHUNK = int(TARGET_CHUNK * NATIVE_RATE / TARGET_RATE)  # equivalent chunk at 44100

model = Model(wakeword_models=["hey_jarvis"], inference_framework="onnx")

socket_path = "/tmp/dhome_wake.sock"
sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
if os.path.exists(socket_path):
    os.unlink(socket_path)
sock.bind(socket_path)
sock.listen(1)
conn, _ = sock.accept()

def callback(indata, frames, time, status):
    audio = np.frombuffer(indata, dtype=np.int16).astype(np.float32) / 32768.0
    resampled = signal.resample_poly(audio, TARGET_RATE, NATIVE_RATE)
    resampled_int = (resampled * 32768).astype(np.int16)
    prediction = model.predict(resampled_int)
    for key, val in prediction.items():
        if val > 0.3:
            try:
                conn.send(b'\x01')
            except BrokenPipeError:
                pass

with sd.RawInputStream(samplerate=NATIVE_RATE, channels=1, dtype='int16',
                       blocksize=NATIVE_CHUNK, callback=callback, device=0):
    sd.sleep(999999999)