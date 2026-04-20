import sounddevice as sd
import numpy as np
import socket
import os
import subprocess
import tempfile
import scipy.io.wavfile as wavfile

RATE = 16000
DURATION = 5

socket_path = "/tmp/dhome_stt.sock"

sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
if os.path.exists(socket_path):
    os.unlink(socket_path)
sock.bind(socket_path)
sock.listen(1)
print("Waiting for dhome to connect...")
conn, _ = sock.accept()
print("Connected. Waiting for record trigger...")

while True:
    data = conn.recv(1)
    if not data:
        break
    if data == b'\x01':
        print("Recording...")
        audio = sd.rec(int(DURATION * RATE), samplerate=RATE, channels=1, dtype='int16')
        sd.wait()

        with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as f:
            wav_path = f.name
        wavfile.write(wav_path, RATE, audio)

        result = subprocess.run(
            ["whisper-cli", "-m", os.path.expanduser("~/.local/share/whisper/models/ggml-base.en.bin"),
             "-f", wav_path, "--no-timestamps", "-nt"],
            capture_output=True, text=True
        )
        os.unlink(wav_path)

        transcript = result.stdout.strip()
        print(f"Transcript: {transcript}")

        msg = (transcript + "\n").encode("utf-8")
        conn.sendall(len(msg).to_bytes(4, 'little') + msg)