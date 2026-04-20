import sounddevice as sd
import numpy as np
import scipy.signal as signal
import scipy.io.wavfile as wavfile
import subprocess, tempfile, os

NATIVE_RATE = 44100
TARGET_RATE = 16000
DURATION = 5
DEVICE = 0

print("Recording...")
audio = sd.rec(int(DURATION * NATIVE_RATE), samplerate=NATIVE_RATE, channels=1, dtype='int16', device=DEVICE)
sd.wait()
print(f"Raw max amplitude: {np.max(np.abs(audio))}")

# resample to 16kHz
audio_float = audio.astype(np.float32) / 32768.0
resampled = signal.resample_poly(audio_float, TARGET_RATE, NATIVE_RATE)
resampled_int = (resampled * 32768).astype(np.int16)
print(f"Resampled max amplitude: {np.max(np.abs(resampled_int))}")

with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as f:
    wav_path = f.name
wavfile.write(wav_path, TARGET_RATE, resampled_int)

model_path = os.path.expanduser("~/.local/share/whisper/models/ggml-base.en.bin")
result = subprocess.run(
    ["whisper-cli", "-m", model_path, "-f", wav_path, "--no-timestamps", "-nt"],
    capture_output=True, text=True
)
print(f"Transcript: '{result.stdout.strip()}'")
os.unlink(wav_path)