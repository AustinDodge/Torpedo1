<h1>DOCUMENTATION</h1>
This synth, including the source code and this documentatino, was originally designed and made as a present. If you're not the original recepient of that present some stuff in here might not make sense to you.
<h2>IMPORTANT SAFETY STUFF READ BEFORE USING</h2>
- Torpedo 1 requires a 7.5-12v center-positive power supply. The lower the better, ideally 9v or under, as higher voltages can increase heat and lower the overall lifespan of the microcontroller. If you don't have an appropriate power supply, you can power it via USB by removing the back cover and plugging a USB cable directly into the Arduino, but this will cause much more noise when playing.

- Remove the back and visually inspect the inside to make sure no wires came loose during shipping before plugging it in to your interface/controller. If nothing is obviously loose the MIDI plug is totally safe to connect to your interface/controller, but it might be wise to test the audio out with cheap headphones or speakers before plugging it into your interface.

- Torpedo 1 only receives MIDI on channel 8, and only via DIN-MIDI, not USB. The channel can be changed with a simple firmware modification, just changing a single clearly-labeled value near the top of the program. Unfortunately there wasn't an easy way to add channel selection to the hardware. It also uses pitch bend, and while it receives other MIDI data like mod wheel, velocity, and aftertouch, it doesn't do anything with them (it wouldn't be hard to change the code to use these, though)

- Torpedo 1 is a rejection of the capitalism-driven consumerist idealizations of our analog past and proudly embraces the democratized digital now. This means there's going to be some glitches and aliasing, especially with high notes and extreme settings (particularly when resonance is fully cranked and being modulated). If you record it it might be a good idea to filter out any frequencies above 14k or so, to ensure that irritating sounds outside of our old old ears's hearing range are eliminated.

<h2>OKAY THAT'S OUT OF THE WAY GO PLAY WITH IT OR READ ON IF YOU'RE A NERD</h2>

Welcome to Torpedo 1, a synthesizer by Jack Garner, based originally on the "Helios 1" synthesizer project on the Blog Hoskins blog. Torpedo 1 differs from Helios 1 in the following ways:
- Second oscillator that can be detuned -12/+5 semitones or turned off
- Vastly improved MIDI behavior
- Improved filter behavior
- LFO and envelope can be used as modulation destinations for either pitch or filter, with varying modulation depth
- Sustain switch that changes envelope behavior
- Amp can be modulated by envelope OR by simple gate on-off behavior
- Lots of source code modifications and optimizations (but it's way bigger now so it's harder to modify)
- And more!

<h2>TESTING</h2>

The first thing you should do is test that all functions are working. It's possible that some wires came loose during shipping. After visually inspecting the inside to make sure nothing's obviously broken, follow these steps:
1. Turn all knobs, including the volume knob, fully counter-clockwise. Set the filter knob fully clockwise. Turn all switches to the "down" position.
2. While playing a note, turn the volume knob slowly up until you've reached a comfortable volume. You should hear an unfiltered triangle wave that tracks the played note and stops when all keys are released. Not hearing anything? Follow these steps:
	- If plugged into an interface, make sure it's correctly set up. It may be easier to test it directly plugged in to your MIDI controller's DIN output until you're sure everything's working. Similarly it might be easier (and safer) to directly listen on headphones or speakers rather than through your interface and DAW until you're sure it's all working.
	- Make sure your MIDI device is transmitting on channel 8.
	- If it's still not working, open the back and watch the red LEDs on top of the Arduino as you play a note. There are four LEDs on top of the Arduino. The third LED should be on, indicating power. The first two LEDs should be on but much fainter than the third, indicating serial communication is active. When you press a note, the first two LEDs should blink extremely quickly, and the fourth LED should remain on until all notes are released. The first two LEDs should quickly blink again when a new note is pressed, a note is released, or any other MIDI data is sent (such as pitch bends, aftertouch or change in the mod wheel location). If you're not seeing this, double-check you're sending MIDI on channel 8. If channel is set up properly but you're still not seeing the LEDs change to indicate reception of MIDI data, check that the DIN connector has three wires soldered to it (they should be connected to the three middle pins, and from the top down, be orange, brown, and red).
	- If the LEDs are blinking to indicate MIDI data is being received, but you're not hearing anything, make sure the volume on your listening device is turned up.
	- If you're certain your listening device is set up properly, make sure the two lugs of the audio jack have wires soldered to them. The closest lug should have a black wire, the far lug red. Then check the volume potentiometer, the top lug should have two wires, one black and one brown, the black going to the audio out,and the brown going to the PCB. The middle lug should have one red wire that connects to the audio jack, and the bottom lug should have one red wire that connects to the PCB.
	- If you're still having troubles give me a call
2. Once you've established MIDI communication is working and audio is coming out, it's time to test the knobs and switches to make sure they all work! Start with the filter: turn the knob through the full range, and you should hear a classic filter sweep. Now leave the cutoff knob at about 9 o'clock, and turn the resonance knob - you should hear the sound's character change as the cutoff frequencies are emphasized. Once you're done playing with it, turn cutoff and resonance both to 12.
3. Flip the oscillator 2 switch (located underneath and left of the "tune" knob in the upper left) up. Oscillator 2 should come in an octave below the base pitch. Turn the knob fully clockwise, and oscillator 2 should go from an octave down to a 5th above the base pitch. 12 o'clock will put the oscillators at the same pitch, giving you the classic unison sound.
3. Flip the square/triangle switch, located to the right of the oscillator 2 switch. Both oscillators should change from a triangle to a square wave.
4. Next, we test envelope. Flip the env/gate switch in the bottom center up. With the "sus" switch still down and the atk/dky knobs still fully counter-clockwise, you should hear the sound briefly blip whenever you press a key. Now turn the "dky" knob to 12, and the sound should start loud, and slowly fade to silent (there might be a small glitch sound after it fully fades, this is unfortunately normal). Turn the "atk" knob to 12, and the should should gradually rise in volume then begin falling. Next, flip the "sus" switch, between the atk and decay knobs - the sound should rise in volume, stay at maximum volume for as long as you hold the key, then start to fade away when you release the key.
5. If all that's working, we'll start testing modulation! Flip the "sus" and "env/gate" switches back down.
6. The modulation routing is controlled by the four switches in the upper right. There are two LFO switches, marked F and P (for Filter and Pitch) and two Env switches marked the same. Flip the LFO - P switch up.
7. The LFO section is in the bottom right. Turn the "rate" knob to about 9 o'clock, then start slowly turning the "depth" knob clockwise. As you turn the depth knob you should first hear a vibratto, then a more extreme pitch shift as the LFO modulates the pitch up and down. Play with the rate knob to ensure the modulation becomes faster as you turn the rate knob counter-clockwise. The LFO has a frequency range of 0.0625-16.4hz, but the relatively slow rate of modulation updates means that the faster end acts more like repeating noise than a fast sine wave.
8. Turn off the LFO-P switch, and turn the filter cutoff to about 9 o'clock with the resonance still at 12. Flip up the LFO-F switch, and you should hear the filter cutoff being modulated at the same rate that the pitch was being modulated. Play with rate and depth again. The modulation is relative to the filter settings. Turn down the cutoff, turn up the resonance, set LFO depth to about 12 and play with the rate knob to get that wobbledy-wobbledy bass.
9. We're almost done, last thing to test is envelope modulation! Envelope can modulate both at positive and negative values, so for less depth, you don't set it fully counter-clockwise, but rather at 12 o'clock. Set the env depth knob to 12, fully open the filter, and make sure the env/gate and sus switches are down. Set atk and dky to about 9 o'clock. Flip up the Env-P switch, then turn the Env Depth knob clockwise. When you press a key, you should hear the pitch rise, then fall, then hold at the pressed value. Turn the knob counter-clockwise of 12, and you should hear the opposite, with the pitch lowering then rising back. Now turn off Env-P and turn on Env-F. Set cutoff to 12, resonance fully up, and env depth at 9. When you press a key, the filter should sweep down then return to normal. Now set cutoff to 9 and env depth to 3 - the filter should sweep up then back down.

If everything's working: congratulations! Experiment and have fun. 

If no knobs or switches work: The 5v/ground line has become disconnected. Trace the red and brown wires on the furthest left side of the PCB to the left and right lugs of the Atk pot and resolder as necessary.

If individual knobs and switches don't appear to work: There's a loose wire or two in there somewhere. It will need to be resoldered lug of the pot/switch. The center wires connect the sensor to the Arduino, and the left/right lugs connect it to 5v and ground(red and black) If both 5v and ground are disconnected, it's not a big deal if you get it backwards, the behavior will simply be reversed.  You or Liz are certainly capable of fixing it yourselves, but feel free to mail it back to me so I can do a better job, I'll cover the shipping cost.

For the center wires, you can determine which wire goes to which sensor by first checking which side of the Arduino the wire connects to the PCB on. Left side is switches, right side is potentiometers. For the pots, the wire colors are:

Detune - grey;
Cutoff - white;
Resonance - black;
LFO Rate - Green;
LFO depth - yellow;
Envelope depth - orange;
Envelope decay - blue;
Envelope attack - purple;

For the switches, colors are:

Env P - black;
Env F - white;
LFO P - grey;
LFO F - purple;
Osc shape - blue;
Osc on - green;
Sustain - yellow;
Amp env/gate - red;

It's not a big deal to get wires mixed up either, as long as you're not mixing up switches and pots. The pin assignments can be easily changed in firmware. Which takes us to...

<h2>FIRMWARE MODIFICATION</h2>
The firmware is written in the Arduino language, which is basically C++. The Arduino IDE includes all the drivers you need to program the board, and you can get the source from the Torpedo 1 github at:https://github.com/AustinDodge/Torpedo1

You should leave the Arduino Nano in the PCB when programming it, but you need to remove the yellow jumper on the far left side of the PCB, then replace it to play via MIDI. USB and MIDI are both serial communication protocols, but Arduino only has one serial port. If either the RX or TX pins of the Arduino are connected to a circuit, USB communication will be disabled. The yellow jumper connects the MIDI circuit to the RX pin of the Arduino.
