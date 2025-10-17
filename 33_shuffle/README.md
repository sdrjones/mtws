# Shuffle

Semi random shuffling of recorded input.

## Video manual

TODO

## Latest Release

TODO

## Source repo

https://github.com/sdrjones/mtws/tree/main/33_shuffle

## Note

Shuffle runs at a 192MHz clock speed. This is slightly less than the maximum supported clock speed in Pico
SDK 2.1.1 but in previous SDKs it would have been considered to be overclocked. If you want to run at a lower
clock speed without clicks then you would need to reduce the maximum number of grains and rebuild the code.

## Summary

* Audio is being recorded to a small buffer. With a clock input the length of the buffer will be the biggest number of clock beats that can fit in a two second buffer. 

* The input shall be splicedand rearranged prior to playback.

## Cheat Sheet

TODO

## Controls

**Z Switch**: Switch up or down to record to the loop.

The "Up" position is a useful hands-free continual record mode in which the grains will play back snippets
from the previous two recorded seconds.

The "Down" position is good for punch in/out style recording. e.g. patch one oscillator's output to
the left audio input, and the other oscillator's output to the right input. Pick a couple of frequencies
and flick the X switch down. A brief snippet of the output will be recorded to the loop, overwriting what
was there before. Rinse and repeat.

NB. There is no dry or monitor output from shuffle but you could patch that using stackables.

---

**Main Knob**: Controls the length of the spliced segments.

---

**X Knob**: Chance of pitch change.

---

**Y Knob**: Chance of repeat.

---

**CV1 in**: Modify the chance of the grains repeating themselves. Chance is maximum at 0V and decreases with
more positive voltage.

---

**CV2 in**: Chance of silent grains.

---

**Pulse 1 in**: Quantises length of splices. Record buffer is a whole number of splices.

---

**LEDs**: ????

## FAQ

## Credits

This card is an homage to [SupaTrigga](https://www.kvraudio.com/product/supatrigga_by_smart_electronix)