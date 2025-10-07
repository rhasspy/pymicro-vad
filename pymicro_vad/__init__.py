"""Self-contained voice activity detector."""

# pylint: disable=no-name-in-module
from micro_vad_cpp import create_vad, process_10ms, reset_vad


class MicroVad:
    """Voice activity detector."""

    def __init__(self) -> None:
        """Initialize VAD."""
        self._vad = create_vad()

    def process_10ms(self, audio: bytes) -> float:
        """Process 10ms of audio (16Khz, 16-bit mono)."""
        return process_10ms(self._vad, audio)

    def reset(self) -> None:
        """Reset VAD."""
        reset_vad(self._vad)
