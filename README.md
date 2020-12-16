# Torpedo1
 Torpedo1 is an Arduino Nano-based synthesizer originally based on the Blog Hoskins Helios 1 synth (https://bloghoskins.blogspot.com/2020/11/20-synth-project-complete-build-guide.html). Changes include:
 
 - Second oscillator that can be detuned -12/+5 semitones or turned off
- Vastly improved MIDI behavior
- Improved filter behavior
- LFO and envelope can be used as modulation destinations for either pitch or filter, with varying modulation depth
- Sustain switch that changes envelope behavior
- Amp can be modulated by envelope OR by simple gate on-off behavior
- Lots of source code modifications and optimizations (but it's way bigger now so it's kind of a wash)
- And more!

The primary change is in the source code. The following changes were made to the physical construction:
- 2 additional potentiometers
- 6 additional switches
- No resistors before or after potentiometer/switch lugs, software value filtering makes them unnecessary
- Volume pot changed from 1M log to 100K linear.
