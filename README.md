# ADX2WAV

An adx2wav c-extension for Python based on [adx2wavmod3](https://hcs64.com/files/adx2wavmod3v2.zip) found on [hcs64.com/vgm_ripping](https://hcs64.com/vgm_ripping.html).

## Installation

### PIP

```cmd
pip install adx2wav
```

### Manual

```cmd
python setup.py install
```


## Example

```python
from adx2wav import adx2wav

with open("test.adx" , "rb") as f:
    data = f.read()

wav = adx2wav(data)

with open("test.wav", "wb") as f:
    f.write(wav)
```
