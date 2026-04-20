import sys, os
sys.stdout = open(os.devnull, 'w')
sys.stderr = open(os.devnull, 'w')

import sounddevice as sd
import numpy as np
import scipy.signal as signal
import scipy.io.wavfile as wavfile
import socket, subprocess, tempfile

NATIVE_RATE = 44100
TARGET_RATE = 16000
DURATION = 5
DEVICE = 0

socket_path = "/tmp/dhome_stt.sock"
sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
if os.path.exists(socket_path):
    os.unlink(socket_path)
sock.bind(socket_path)
sock.listen(1)
conn, _ = sock.accept()

while True:
    data = conn.recv(1)
    if not data:
        break
    if data == b'\x01':
        audio = sd.rec(int(DURATION * NATIVE_RATE), samplerate=NATIVE_RATE, channels=1, dtype='int16', device=DEVICE)
        sd.wait()

        audio_float = audio.astype(np.float32) / 32768.0
        resampled = signal.resample_poly(audio_float, TARGET_RATE, NATIVE_RATE)
        resampled_int = (resampled * 32768).astype(np.int16)

        with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as f:
            wav_path = f.name
        wavfile.write(wav_path, TARGET_RATE, resampled_int)

        model_path = os.path.expanduser("~/.local/share/whisper/models/ggml-base.en.bin")
        result = subprocess.run(
            ["whisper-cli", "-m", model_path, "-f", wav_path, "--no-timestamps", "-nt"],
            capture_output=True, text=True
        )
        os.unlink(wav_path)

        transcript = result.stdout.strip()
        msg = (transcript + "\n").encode("utf-8")
        conn.sendall(len(msg).to_bytes(4, 'little') + msg)