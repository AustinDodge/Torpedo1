/*
Torpedo 1 v 0.5
Heavily based off the Helios 1 from Blog Hoskins


v 0.5
- Added software exponential filtering of pot values for more stable and reliable readings
- This also simplifies construction, since additional resistors aren't required before or after the pots
- Improved MIDI note handling. Releasing an old note won't stop the current note, and the highest note has priority
- Additionally, MIDI tracking happens in the audio step, greatly reducing lag and glitches.
- Added pitch bend
- Changed filter LFO behavior - instead of a second notch filter, the LFO value is directly added to the destination value, and LFO frequency updates every step.
- Envelope can be sent to filter OR pitch
- Second oscillator with a range that can be tuned via a 7th pot  from -12 to +5 steps.
- Envelope sustain can be turned off so attack immediately leads to release, and...
- Amp can be switch between envelope control or a simple gate. This is handy because...
- Envelope can be used as a mod source for either pitch or filter cutoff, with depth controlled by an 8th pot.
- Changed from automap to regular map function for all pots. Automap can return unexpected results until you swipe through the whole pot range, using map gives you the patch on the knobs the moment you turn the synth on.
- Added a specific MIDI channel to listen on

*/


#include <MIDI.h>
#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <mozzi_midi.h>
#include <ADSR.h>
#include <LowPassFilter.h> // You need this for the LPF
#include <AutoMap.h> // to track and reassign the pot values

#include <tables/saw4096_int8.h> // saw table for oscillator
#include <tables/square_no_alias_2048_int8.h> // square table for oscillator

//*****************LFO******************************************************************************************
#include <tables/cos2048_int8.h> // sine wave for filter modulation
//#include <StateVariable.h>
#include <mozzi_rand.h> // for rand()
#include <AutoMap.h> // To track pots **************************************************************************



/*using #define instead of a const or other variable takes up no memory or additional processing. They're not great in many
 * contexts but in an arduino it's a good way to save memory.
 */

#define DEBUG 0//set to 1 to use the serial port for monitoring rather than MIDI data

#define MIDICHANNEL 8//set to 0 for all channels
 
//********Define switch input pins *******************************************************************
#define SWITCH_OSCSHAPE 8
#define SWITCH_SUSTAIN 6
#define SWITCH_AMPENVGATE 5
#define SWITCH_PITCHENVELOPE 4
#define SWITCH_PITCHLFO 2
#define SWITCH_OSC2ON 7
#define SWITCH_FILTERENVELOPE 3
#define SWITCH_FILTERLFO 10

//******** Define potentiometer pins ************************************************************
/*Arduino automatically chooses whether to read from the digital or analog pins based on the function used to check it
 * so there's no need to specify whether you're checking A5 or D5.
 */
 #define POT_DETUNE 2
 #define POT_CUTOFF 3
 #define POT_RESONANCE 4
 #define POT_ATTACK 1
 #define POT_DECAY 0
 #define POT_ENVDEPTH 7
 #define POT_LFORATE 5
 #define POT_LFODEPTH 6

//******** Define potentiometer min and max values ************************************************
/*All pots are 10k and will reliably return a value between 0-1023. The raw returned value is converted to values more useful for synthesis
 * which are defined here.
 */

//Attack and decay times are defined in milliseconds.
#define ATTACKMIN 0
#define ATTACKMAX 3000

#define DECAYMIN 0
#define DECAYMAX 3000

#define ENVDEPTHMIN -999
#define ENVDEPTHMAX 999

#define LFORATEMIN 16//divide by 256 to get LFO rate in hz. Lower values might not update properly.
#define LFORATEMAX 4200

#define LFODEPTHMIN 0
#define LFODEPTHMAX 255

#define CUTOFFMIN 10
#define CUTOFFMAX 255

#define RESONANCEMIN 0
#define RESONANCEMAX 255

#define DETUNEMIN 500//down an octave and up a fifth. This also conveniently puts 0 at 12 o'clock.
#define DETUNEMAX 1500

// use #define for CONTROL_RATE, not a constant
#define CONTROL_RATE 128 // powers of 2 please, larger number means more responsive but more computationally intense.


//bend ranges, currently set for approx +/- 2 semitones. These will be divided by 10000 later but for now they need to
//work with integer math to be compatible with the map() function.
#define BENDMIN 8909
#define BENDMAX 11220


//************ Exponential filter setup ****************************************************
//The previous pot value needs to be stored to filter out the noise.

int LFOrateVal = 0;
int oldLFORateVal = 0;

int LFOamountVal = 0;
int oldLFOAmtVal = 0;

int atkVal = 0;       // variable to store the value coming from the pot
int oldAtkVal = 0; //store the old value for filtering

int dkyVal = 0;       // variable to store the value coming from the pot
int oldDkyVal = 0; //store the previous value for filtering

int envelopeAmountVal = 0;
int oldEnvelopeAmountVal = 0;

int cutVal = 0;
int oldCutVal = 0;

int resVal = 0;
int oldResVal = 0;

int detuneAmountVal = 0;
int oldDetuneAmountVal = 0;


float fDetuneAmountVal = 0;


//********Create LFO*************************************************************************
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> oscLFO(COS2048_DATA);
int lfoValue = 0;//updated in the audio section

//******* Create audio oscillators *************************************************
Oscil <SAW4096_NUM_CELLS, AUDIO_RATE> oscil1; //Saw Wav
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> oscill2; //Sqr wave
//********* oscillator modulation variables *****************************************
float pitchMod = 0;
float freq = 1;
float bendval = 1;


//Create an instance of a low pass filter *********************************************
LowPassFilter lpf; 

// envelope generator ****************************************************************
ADSR <CONTROL_RATE, AUDIO_RATE> envelope;
int envelopeValue = 0;//updated int the audio section

float envelopeDepth = 0.5;//-1 to 1
bool envelopeToFilter = 1;//to filter or pitch

int gate = 0;//when not using the envelope to control the amplifier, use the gate to determine if the volume is high or low
bool ampUseEnvelope = 0;


#define LED 13 // Internal LED lights up if MIDI is being received

//MIDI stuff*******************************************************************
MIDI_CREATE_DEFAULT_INSTANCE();

//arrays for holding pressed MIDI notes
byte notePressed[127];
byte noteVelocity[127];
byte currentNote = 0;//the note currently pressed



void setup() {
  pinMode(LED, OUTPUT);  // built-in arduino led lights up with midi sent data 
  // Connect the HandleNoteOn function to the library, so it is called upon reception of a NoteOn.
  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.setHandlePitchBend(HandlePitchBend);

  envelope.setADLevels(255,255); // Max attack and decay levels, different from time. Mozzi is weird about it.
  
  for (int i = 0; i < 128; i++){
    notePressed[i] = 0;
    noteVelocity[i] = 0;
  }

  oscil1.setFreq(440); // default frequency

  pinMode(2, INPUT_PULLUP); 
  pinMode(3, INPUT_PULLUP); 
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  
  startMozzi(CONTROL_RATE);

//initialize synth values
  atkVal = 10;
  oldAtkVal = atkVal;
  dkyVal = 100;
  oldDkyVal = 0;
  envelope.setTimes(atkVal,dkyVal,60000,dkyVal);

  cutVal = 0;
  oldCutVal = cutVal;
  resVal = 0;
  oldResVal = 0;
  lpf.setCutoffFreqAndResonance(cutVal,resVal);

  if (DEBUG == 0){
    // Initiate MIDI communications
    MIDI.begin(MIDICHANNEL);
  }else{
    Serial.begin(9600);//for listening during debug
  }
  currentNote = 100;
}

void setPitch(){
   //oscillator 1
  freq = mtof(float(currentNote));
  freq += freq*pitchMod;//add the LFO, envelope, and pitch bend detune.
  if (freq < 1){
      freq = 1;
    }
  //oscillator 2
  oscil1.setFreq(freq);//set oscillator 1 frequency
  //Osc 2 pitch
  float freq2;
  freq2 = freq*fDetuneAmountVal;//oscilaltor 2 detune
  oscill2.setFreq(freq2);//set oscillator 2 frequency
  }

//note bends
//The Arduino MIDI library automatically combines the MSB and LSB into a single integer
//and normalizes it from 0-16384 to -8192-8192
void HandlePitchBend(byte channel, int value){
  bendval = float(map(value,-8192,8192,BENDMIN,BENDMAX)) / 10000.0;
}

void HandleNoteOn(byte channel, byte note, byte velocity) { 
    if (note <= 100){
    notePressed[note] = 1;
    noteVelocity[note] = velocity;
  
    playNote();
    }
}

void HandleNoteOff(byte channel, byte note, byte velocity) { 
  notePressed[note] = 0;
  playNote();
}

//whenever a new note event happens, do this
//Determine what the highest currently held note is, set the master tune (currentNote) to that, and retrigger the envelope unless no note is pressed, in which case stop the envelope
void playNote(){
  byte highNote = 128;
  for (byte i = 0; i < 128; i++){
    if (notePressed[i] == 1){
        highNote = i;
        digitalWrite(LED,HIGH);
    }
  }
  if (highNote >= 120){//no note pressed
    digitalWrite(LED,LOW);
    envelope.noteOff();
    gate = 0;
  }else{
    envelope.setReleaseTime(0);
    envelope.noteOff();
    currentNote = highNote;
    envelope.noteOn();
    gate = 255;
    setPitch();
    }
 }

//exponential filter to smooth out pot values, 
int expFilter(int oldval, int newval, float weight){
    int i = weight * newval + (1-weight) * oldval;
    return i;
}


//main control loop
void updateControl(){
  // set attack/decay times
  oldAtkVal = atkVal;
  atkVal = mozziAnalogRead(POT_ATTACK);    // read the value from the attack pot
  atkVal = expFilter(oldAtkVal,atkVal,0.2);//filter the value
  int rAtkVal = map(atkVal,0,1023,ATTACKMIN,ATTACKMAX);
  
  oldDkyVal = dkyVal;
  dkyVal = mozziAnalogRead(POT_DECAY);    // read the value from the decay pot
  dkyVal = expFilter(oldDkyVal,dkyVal,0.1);//filter the value
  int rDkyVal = map(dkyVal,0,1023,DECAYMIN,DECAYMAX);

//if the "sustain" switch is high, these are used for attack and release, with full sustain. Otherwise, they're used for attack and decay, with no sustain.
  if (digitalRead(SWITCH_SUSTAIN) == HIGH){
    envelope.setSustainLevel(255);
    envelope.setTimes(rAtkVal,0,60000,rDkyVal); // 60000 is so the note will sustain 60 seconds unless a noteOff comes
    envelope.update();
  }else{
    envelope.setSustainLevel(255);
    envelope.setTimes(rAtkVal,0,0,rDkyVal);
    envelope.update();
  }

  if (digitalRead(SWITCH_AMPENVGATE) == HIGH){
    ampUseEnvelope = 1;
  }else{
    ampUseEnvelope = 0;
  }
  
  //***************** read LFO control pots ****************************************************************************
  oldLFORateVal = LFOrateVal;
  LFOrateVal = mozziAnalogRead(POT_LFORATE);    // Speed of LFO
  LFOrateVal = expFilter(oldLFORateVal,LFOrateVal,0.1);
  int rLFOrateVal = map(LFOrateVal,0,1023,LFORATEMIN,LFORATEMAX);         // map to 25-1300

  oscLFO.setFreq_Q24n8(rLFOrateVal);

  oldLFOAmtVal = LFOamountVal;
  LFOamountVal = mozziAnalogRead(POT_LFODEPTH);    // read the value from the attack pot
  LFOamountVal = expFilter(oldLFOAmtVal,LFOamountVal,0.1);
  float rLFOamtVal = map(LFOamountVal,0,1023,LFODEPTHMIN,LFODEPTHMAX);            // map to 0-255
  float LFOMultiplier = (rLFOamtVal)/255;//convert to betweel 0-1

//if LFO switch is on, add the LFO to the filter cutoff
  int LFOFilterAmt = 0;
  int LFOPitchAmt = 0;
  int lfoAmount = lfoValue * LFOMultiplier;
  
  if (digitalRead(SWITCH_FILTERLFO) == HIGH){//send LFO to filter
    LFOFilterAmt = lfoAmount;
  }
  if (digitalRead(SWITCH_PITCHLFO) == HIGH){
    LFOPitchAmt = lfoAmount;
  }


  //Calculate the amount of envelope modulation
  oldEnvelopeAmountVal = envelopeAmountVal;
  envelopeAmountVal = mozziAnalogRead(POT_ENVDEPTH);
  envelopeAmountVal = expFilter(oldEnvelopeAmountVal,envelopeAmountVal,0.1);
  float rEnvelopeAmountVal = map(envelopeAmountVal,0,1023,ENVDEPTHMIN,ENVDEPTHMAX);
  float envelopeMultiplier = (rEnvelopeAmountVal)/1000;//convert to -1-1


  int envelopeFilterAmt = 0;
  int envelopePitchAmt = 0;
  int envelopeAmount = envelopeValue * envelopeMultiplier;

  if (digitalRead(SWITCH_FILTERENVELOPE) == HIGH){//send envelope to filter
    envelopeFilterAmt = envelopeAmount;
  }
  if (digitalRead(SWITCH_PITCHENVELOPE) == HIGH){
    envelopePitchAmt = envelopeAmount;
  }
  
  //**************RESONANCE POT****************
  oldResVal = resVal;
  resVal = mozziAnalogRead(POT_RESONANCE);  // arduino checks pot value
  resVal = expFilter(oldResVal,resVal,0.1);
  int rResVal = map(resVal,0,1023,RESONANCEMIN,RESONANCEMAX);

  //**************CUT-OFF POT****************
  oldCutVal = cutVal;
  cutVal = mozziAnalogRead(POT_CUTOFF);  // arduino checks pot value
  cutVal = expFilter(oldCutVal,cutVal,0.1);
  int rCutVal = map(cutVal,0,1023,CUTOFFMIN,CUTOFFMAX);
  
  //add LFO 
  rCutVal +=  LFOFilterAmt;
  //add envelope
  rCutVal += envelopeFilterAmt;
  //correct so modulation doesn't put it out of range
  if (rCutVal < CUTOFFMIN){
      rCutVal = CUTOFFMIN;
  }
  if (rCutVal > CUTOFFMAX){
      rCutVal = CUTOFFMAX;
  }  

  lpf.setCutoffFreqAndResonance(rCutVal,rResVal);//change the values together

  //**********Detune pot***********************
  
  oldDetuneAmountVal = detuneAmountVal;
  detuneAmountVal = mozziAnalogRead(POT_DETUNE);
  detuneAmountVal = expFilter(oldDetuneAmountVal,detuneAmountVal,0.2);
  float rDetuneAmountVal = map(detuneAmountVal,0,1023,DETUNEMIN,DETUNEMAX);
  fDetuneAmountVal = rDetuneAmountVal/1000;//convert to 0.5-1.5

  
  //********** Determine pitch modulation values
  /*There is a small amount of lag in determining pitch during the control update instead of the audio update, but it's too complicated to do in
  the audio step. Trying to do it there results in lots of glitches and slowdown. We can compensate for incoming pitch by re-determining immediately when
  we receive MIDI data, but it's still apparent when modulating the pitch under certain settings. We'll just say it's part of that special digital crunch!
  */
  pitchMod = bendval;
  pitchMod += float(LFOPitchAmt)/128;//convert to -1-1 for +/- 1 octave
  pitchMod += (float(envelopePitchAmt)/32);

  setPitch();

  //oscillator shape
  if (digitalRead(SWITCH_OSCSHAPE) == LOW) // If switch is set to high, run this portion of code
  {
    oscil1.setTable(SAW4096_DATA);
    oscill2.setTable(SAW4096_DATA);
  }
  else  // If switch not set to high, run this portion of code instead
  {
    oscil1.setTable(SQUARE_NO_ALIAS_2048_DATA);
    oscill2.setTable(SQUARE_NO_ALIAS_2048_DATA);
  }
}

int updateAudio(){
    //checking MIDI in the audio step lets us get much more responsive control without calculating all the other control
    //things each step.
    if (DEBUG == 0){
      MIDI.read();
    }else{
    //put in what you want to monitor here
    Serial.println(5);
    }
    //update values
    envelopeValue = envelope.next();
    lfoValue = oscLFO.next();
    int oscs = 0;
    if (digitalRead(SWITCH_OSC2ON) == HIGH){
      oscs = (oscil1.next() + (oscill2.next())) >> 1;//mix oscillators
    }else{
      oscs = (oscil1.next() >> 1);//don't mix in oscillator 2 if there's no detune
    }
    int asig;
    if (ampUseEnvelope == 1){
      asig = (envelopeValue * oscs) >> 10;
    }else{
      asig = (gate*oscs) >> 10;
    }
    int asigLPF = lpf.next(asig); // LPF
    return (asigLPF); 
}

void loop() {
  audioHook(); // required here
} 
