# microVAD

Self-contained voice activity detector (VAD) that uses the machine learning architecture from [microWakeWord](https://github.com/kahrendt/microWakeWord/).


## Installation

``` sh
pip install pymicro-vad
```


## Usage

``` python
from pymicro_vad import MicroVad

vad = MicroVad()
threshold = 0.5

# Process 10ms chunks of 16-bit mono PCM @16Khz
while audio := get_10ms_of_audio():  # you define this
    assert len(audio) == 160 * 2  # 160 samples
    speech_prob = vad.process_10ms(audio)
    if speech_prob < 0:
        print("Need more audio")
    elif speech_prob > threshold:
        print("Speech")
    else:
        print("Silence")
```


## Building

Ensure you have `python3-dev` and `build-essential` installed.

Run `script/setup` to create a virtual environment, then `script/build` to build the extension locally.
