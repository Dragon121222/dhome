import sounddevice as sd
import numpy as np
from openwakeword.model import Model
import socket

CHUNK = 1280  # 80ms at 16kHz, required by openWakeWord
RATE = 16000

# model = Model(wakeword_models=["/usr/lib/python3.14/site-packages/openwakeword/resources/models/Ok_GLaDOS.onnx"], inference_framework="onnx")
model = Model(wakeword_models=["hey_jarvis"], inference_framework="onnx")


sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
sock.bind("/tmp/dhome_wake.sock")
sock.listen(1)
print("Waiting for dhome to connect...")
conn, _ = sock.accept()
print("Connected. Listening for wake word...")


def callback(indata, frames, time, status):
    audio = np.frombuffer(indata, dtype=np.int16)
    prediction = model.predict(audio)
    for key, val in prediction.items():
        if val > 0.3:
            print(f"Wake word detected: {key} ({val:.2f})")
            try:
                conn.send(b'\x01')
            except BrokenPipeError:
                pass

with sd.RawInputStream(samplerate=RATE, channels=1, dtype='int16',
                       blocksize=CHUNK, callback=callback):
    print("Mic open.")
    sd.sleep(999999999)