# microVAD

Self-contained voice activity detector (VAD) that uses the machine learning architecture from [microWakeWord](https://github.com/kahrendt/microWakeWord/).

``` python
from pymicro_vad import MicroVad

vad = MicroVad()
threshold = 0.9

# Process 10ms chunks of 16-bit mono PCM @16Khz
while audio := get_10ms_of_audio():
    assert len(audio) == 160 * 2  # 160 samples
    speech_prob = vad.Process10ms(audio)
    if speech_prob < 0:
        print("Need more audio")
    elif speech_prob > threshold:
        print("Speech")
    else:
        print("Silence")
```


## Building

See `script/build`
