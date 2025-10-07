import wave
from pathlib import Path
from typing import List, Optional

from pymicro_vad import MicroVad

BYTES_PER_CHUNK = 160 * 2  # 10ms @ 16Khz (16-bit mono)

_DIR = Path(__file__).parent


def read_wav(file_name: str) -> bytes:
    """Return audio bytes from WAV file (16Khz, 16-bit mono)."""
    with wave.open(str(_DIR / file_name), "rb") as wav_file:
        assert wav_file.getframerate() == 16000
        assert wav_file.getsampwidth() == 2
        assert wav_file.getnchannels() == 1

        return wav_file.readframes(wav_file.getnframes())


def run_vad(file_name: str, vad: Optional[MicroVad] = None) -> List[float]:
    """Run VAD on WAV file and return speech probabiltiies."""
    probs: List[float] = []
    if vad is None:
        vad = MicroVad()
    audio = read_wav(file_name)
    i = 0
    while (i + BYTES_PER_CHUNK) < len(audio):
        chunk = audio[i : i + BYTES_PER_CHUNK]
        prob = vad.process_10ms(chunk)
        if prob >= 0:
            # Only keep valid probabilities.
            # process_10ms returns -1 if more audio is needed.
            probs.append(prob)
        i += BYTES_PER_CHUNK

    return probs


def test_silence() -> None:
    """Run VAD on known silence."""
    probs = run_vad("silence.wav")
    assert all(p < 0.5 for p in probs)


def test_music() -> None:
    """Run VAD on music without speech."""
    probs = run_vad("music.wav")
    assert all(p < 0.9 for p in probs)


def test_speech() -> None:
    """Run VAD on speech and clip it out."""
    probs = run_vad("speech.wav")

    # Determine start/end of speech
    speech_left = 10
    in_command = False
    silence_left = 10
    start_i = 0
    end_i = 0

    for i, prob in enumerate(probs):
        if in_command:
            if prob <= 0.9:
                silence_left -= 1
                if silence_left <= 0:
                    end_i = i
                    in_command = False
                    break
        elif prob > 0.9:
            speech_left -= 1
            if speech_left <= 0:
                in_command = True
                start_i = i

    # Speech should be about a second long.
    # Each chunk is 10ms.
    assert (end_i - start_i) > 100
    assert (end_i - start_i) < 150


def test_reset() -> None:
    """Test VAD reset"""
    vad = MicroVad()
    probs1 = run_vad("speech.wav", vad)
    probs2 = run_vad("speech.wav", vad)

    # Probabilities are different due to maintained state
    assert probs1 != probs2

    # Same state after reset
    vad.reset()
    probs3 = run_vad("speech.wav", vad)
    assert probs1 == probs3
