"""Speech probabilities are independent of the feature ring position."""

import wave
from pathlib import Path

import pytest

from pymicro_vad import MicroVad

CHUNK_BYTES = 160 * 2
WINDOW_FRAMES = 74


def probabilities(audio: bytes, leading_silence_frames: int) -> list[float]:
    vad = MicroVad()
    for _ in range(leading_silence_frames):
        vad.process_10ms(bytes(CHUNK_BYTES))

    return [
        vad.process_10ms(audio[i : i + CHUNK_BYTES])
        for i in range(0, len(audio) - CHUNK_BYTES + 1, CHUNK_BYTES)
    ]


@pytest.mark.parametrize("extra_frames", [1, 17, 37, 73, 74])
@pytest.mark.parametrize("file_name", ["speech.wav", "silence.wav"])
def test_leading_silence_does_not_rotate_model_input(
    file_name: str, extra_frames: int
) -> None:
    with wave.open(str(Path(__file__).parent / file_name), "rb") as wav_file:
        assert (
            wav_file.getframerate(),
            wav_file.getsampwidth(),
            wav_file.getnchannels(),
        ) == (
            16000,
            2,
            1,
        )
        audio = wav_file.readframes(wav_file.getnframes())

    # Warm up the frontend and fill the input window with silence. Additional
    # silence changes only the ring position, not the following audio history.
    # A full window (74 frames) is the control: it restores the same position.
    expected = probabilities(audio, 2 * WINDOW_FRAMES)
    shifted = probabilities(audio, 2 * WINDOW_FRAMES + extra_frames)
    assert shifted == pytest.approx(expected, rel=0, abs=1e-6)
