/*
  Basic template for working with a stock MEAP board.
 */

#define CONTROL_RATE 128  // Hz, powers of 2 are most reliable
#include <Meap.h>         // MEAP library, includes all dependent libraries, including all Mozzi modules

#include "SynthOscillator.h"  //Adds Synth Oscillator class

#include <tables/sq8192_32harm_int8.h>
mOscil<sq8192_32harm_int8_NUM_CELLS, AUDIO_RATE, int8_t> osc1(sq8192_32harm_int8_DATA);      //Bass
mOscil<sq8192_32harm_int8_NUM_CELLS, AUDIO_RATE, int8_t> osc2(sq8192_32harm_int8_DATA);      //Mid
mOscil<sq8192_32harm_int8_NUM_CELLS, AUDIO_RATE, int8_t> osc3(sq8192_32harm_int8_DATA);      //Melody
mOscil<sq8192_32harm_int8_NUM_CELLS, AUDIO_RATE, int8_t> noiseOsc(sq8192_32harm_int8_DATA);  //Noise

//oscillator envelopes
ADSR<AUDIO_RATE, AUDIO_RATE> env1;
ADSR<AUDIO_RATE, AUDIO_RATE> env2;
ADSR<AUDIO_RATE, AUDIO_RATE> env3;

//noise envelope
ADSR<CONTROL_RATE, AUDIO_RATE> noiseEnv;

//RANDOM VARIABLES:
int randTempo = 40.0 + ((float)rand() / RAND_MAX) * 140.0;         // random 40–180 BPM
int randTranspose = -12 + (int)(((float)rand() / RAND_MAX) * 25);  // -12 to 12;

#include <stdlib.h>  // for rand()

int oscMode = 0;  // keeps track of oscillator modes

int arpMode = 0;   // keeps track of what arp mode to use
int bassMode = 0;  // keeps track of what bass mode to use

// Original volumes from mode selection:
float osc1Volume_orig = 0.8;
float osc2Volume_orig = 0.2;
float osc3Volume_orig = 0.6;
float noiseOscVolume_orig = 0.5;

// Active volumes used for audio:
float osc1Volume;
float osc2Volume;
float osc3Volume;
float noiseOscVolume;

// Mute toggles:
bool osc1Muted = false;
bool osc2Muted = false;
bool osc3Muted = false;
bool oscNoiseMuted = false;

bool halfTimeToggle = false;

uint16_t lfsr = 0xACE1u;  // Initial value (seed) for the 16-bit LFSR, must be non-zero
int lfsr_divisor = 0;
int lfsr_countdown = 0;
int lfsr_last = 0;

bool short_mode = false;

int drumStep = 0;  // keeps track of which drum to play
enum DrumType {
  KICK,
  SNARE,
  HIHAT,
  TOM,
  CLAP,
  RIM,
  COWBELL
};

// DrumType drumSequence1[]  = { KICK, HIHAT, SNARE, HIHAT };
// DrumType drumSequence2[]  = { KICK, SNARE, HIHAT };
// DrumType drumSequence3[]  = { KICK, HIHAT, KICK, HIHAT, SNARE, HIHAT };
// DrumType drumSequence4[]  = { KICK, CLAP };
// DrumType drumSequence5[]  = { KICK, HIHAT, SNARE, HIHAT, KICK };
// DrumType drumSequence6[]  = { KICK, HIHAT, HIHAT, SNARE };
// DrumType drumSequence7[]  = { KICK, HIHAT, SNARE, HIHAT, CLAP, HIHAT, TOM };
// DrumType drumSequence8[]  = { KICK, RIM, HIHAT };
// DrumType drumSequence9[]  = { KICK, HIHAT, KICK, HIHAT, KICK, HIHAT };
// DrumType drumSequence10[] = { SNARE, HIHAT, KICK };
// DrumType drumSequence11[] = { KICK, SNARE, KICK, SNARE };
// DrumType drumSequence12[] = { HIHAT, HIHAT, HIHAT, HIHAT };
// DrumType drumSequence13[] = { KICK, HIHAT, SNARE, HIHAT, KICK, TOM, HIHAT, CLAP };
// DrumType drumSequence14[] = { KICK };
// DrumType drumSequence15[] = { KICK, HIHAT, KICK, HIHAT, SNARE };
// DrumType drumSequence16[] = { KICK, SNARE, HIHAT, HIHAT, KICK, RIM };

// Generates a pseudo-random Gameboy-style noise sample using a 15-bit Linear Feedback Shift Register (LFSR)
// Basically this outputs 127 or -127 for each sample. this back and forth creates a white noise effect
int8_t gameboyNoise() {
  if (--lfsr_countdown <= 0) {
    lfsr_countdown = lfsr_divisor;
    uint8_t bit = ((lfsr >> 0) ^ (lfsr >> 1)) & 1;  // XOR the first two bits to create feedback
    lfsr = (lfsr >> 1) | (bit << 14);               // Shift LFSR right and insert feedback bit at the top
    if (short_mode) {
      lfsr = (lfsr & 0b10111111) | (bit << 6);
    }
    lfsr_last = (lfsr & 1) ? 127 : -127;
  }
  return lfsr_last;  // Output +127 or -127 as a noise sample (audio amplitude for a Gameboy-style hiss)
}

//updates the oscillator volumes for mute functionality
void updateOscillatorVolumes() {
  osc1Volume = osc1Muted ? 0.0 : osc1Volume_orig;
  osc2Volume = osc2Muted ? 0.0 : osc2Volume_orig;
  osc3Volume = osc3Muted ? 0.0 : osc3Volume_orig;
  noiseOscVolume = oscNoiseMuted ? 0.0 : noiseOscVolume_orig;
}

//changes the oscillator mode so that different instruments play each time the randomize button is pressed
void oscModeUpdater() {
  switch (oscMode) {

    case 1:  // Mode 1: all 4 on
      osc1Muted = false;
      osc2Muted = false;
      osc3Muted = false;
      oscNoiseMuted = false;
      break;

    case 2:  // Mode 2: bass + lead
      osc1Muted = false;
      osc2Muted = true;
      osc3Muted = false;
      oscNoiseMuted = true;
      break;

    case 3:  // Mode 3: lead only
      osc1Muted = true;
      osc2Muted = true;
      osc3Muted = false;
      oscNoiseMuted = true;
      break;

    case 4:  // Mode 4: synth (mid) + bass
      osc1Muted = false;
      osc2Muted = false;
      osc3Muted = true;
      oscNoiseMuted = true;
      break;

    case 5:  // Mode 5: just middle synth
      osc1Muted = true;
      osc2Muted = false;
      osc3Muted = true;
      oscNoiseMuted = true;
      break;

    case 6:  // Mode 6: just white noise
      osc1Muted = true;
      osc2Muted = true;
      osc3Muted = true;
      oscNoiseMuted = false;
      break;
  }
}

//randomizes the oscillator envelopes each time this is called
void randomizeEnvelopes() {
  // --- osc1 — lead ---
  env1.setADLevels(
    255,             // attack level stays max
    random(40, 150)  // random decay level
  );
  env1.setTimes(
    random(3, 40),   // attack time
    random(10, 80),  // decay time
    random(10, 60),  // sustain time
    random(5, 40)    // release time
  );

  // --- osc2 — mid synth ---
  env2.setADLevels(
    255,
    random(60, 180));
  env2.setTimes(
    random(10, 60),
    random(20, 90),
    random(10, 80),
    random(10, 60));

  // --- osc3 — bass ---
  env3.setADLevels(
    255,
    random(10, 70));
  env3.setTimes(
    random(1, 20),
    random(5, 40),
    random(5, 30),
    random(5, 30));
}

Meap meap;                                            // creates MEAP object to handle inputs and other MEAP library functions
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);  // defines MIDI in/out ports

// ---------- YOUR GLOBAL VARIABLES BELOW ----------
float bpm = 100.0;       // default starting tempo
float sixteenth_length;  // seconds per 16th note

// --- BASS VARIABLES ---
bool useFifth = false;           // toggles between root and fifth
int bassNote = 0;                // current bass note MIDI number
int bassChord = 0;               // chord being used for bass
unsigned long lastBassTime = 0;  // last time bass note triggered
int bassInterval = 250;          // time between notes (ms) ~120BPM

// --- BANJO VARIABLES ---
int banjoStep = 0;
int banjoLastNote = -1;

const int numPatterns = 4;
int currentPattern = 0;  // select pattern per chord

int transposer = 0;  // global semitone transpose

int mode = 1;  // 0 = fixed progressions, 1 = random transition

int currentProgression = 1;  // index of the active chord progression
int progressionIndex = 0;    // index within that progression

int currentChord = 0;  // the currently playing chord
bool playRoot = true;  // alternates root/fifth

EventDelay metro;       // metronome for bass + chord timing
int beatsPerChord = 8;  // change chord every 8 beats
int beatCounter = 0;

/*****ADDS ALL CHORD AND SCALE DATA*****/
#include "ChordData.h"

bool useMinorMode = false;  // set true for G minor, false for G major

int (*chordNotes)[4] = useMinorMode ? chordNotes_minor : chordNotes_major;
int (*nextChord)[4] = useMinorMode ? nextChord_minor : nextChord_major;
int (*chordProbabilities)[4] = useMinorMode ? chordProbabilities_minor : chordProbabilities_major;

int numProgressions = 5;

SynthOscillator bass;
SynthOscillator banjo;
SynthOscillator melody;
SynthOscillator noise;

// Global state to keep track of current pattern and step
int currentBanjoPattern = 0;

void setup() {
  Serial.begin(115200);                                                             // begins Serial communication with computer
  meap.begin();                                                                     // sets up MEAP object
  Serial1.begin(31250, SERIAL_8N1, meap.MEAP_MIDI_IN_PIN, meap.MEAP_MIDI_OUT_PIN);  // sets up MIDI: baud rate, serial mode, rx pin, tx pin
  startMozzi(CONTROL_RATE);                                                         // starts Mozzi engine with control rate defined above

  sixteenth_length = (60.0 / bpm) / 4.0;  // seconds per 16th
  metro.start(sixteenth_length * 1000);   // start metro (ms)

  //randomizes oscillator envelopes
  randomizeEnvelopes();
}

int getNextChord() {
  if (mode == 0) {  // predefined progression mode
    int chord = chordProgressions[currentProgression][progressionIndex];
    progressionIndex++;
    if (chordProgressions[currentProgression][progressionIndex] == -1)
      progressionIndex = 0;
    return chord;

  } else {  // random transition mode
    int randVal = rand() % 100;
    for (int i = 0; i < 4; i++) {
      if (nextChord[currentChord][i] == -1) break;
      if (randVal < chordProbabilities[currentChord][i])
        return nextChord[currentChord][i];
    }
  }
  return 0;  // fallback to G
}

// Returns the root note of the chord without any octave changes
int getBassNote(int chordIndex) {
  return chordNotes[chordIndex][0];
}

// Returns the root note of the chord with octave changes
int getBassNoteOct(int chordIndex) {
  int root = chordNotes[chordIndex][0];

  int r = random(0, 4);  //
  if (r == 0) {          // 25% chance
    root += 12;          // up an octave
  } else if (r == 1) {   // 25% chance
    root -= 12;          // down an octave
  }

  return root;
}

// Returns the bass note of the chord with alternating root fifth changes
int getBassNoteRootFifth(int chordIndex) {
  static bool returnRoot = true;  // remembers last choice

  int root = chordNotes[chordIndex][0];
  int fifth = root + 7;

  if (random(0, 2) == 0) {  // randomly drop fifth an octave
    fifth -= 12;
  }

  int result = returnRoot ? root : fifth;
  returnRoot = !returnRoot;  // alternate next time
  return result;
}

// Call this to get the next note for the banjo
int getBanjoNote(int chordIndex) {
  // Select a random pattern if we're at the start of a new roll
  if (banjoStep == 0) {
    currentBanjoPattern = random(0, sizeof(banjoPatterns) / sizeof(banjoPatterns[0]));
  }

  const int* pattern = banjoPatterns[currentBanjoPattern];

  // Find the valid length of the pattern (stop at -1)
  int patternLength = 0;
  for (int i = 0; i < 6; i++) {
    if (pattern[i] == -1) break;
    patternLength++;
  }

  // Get the note index from the pattern
  int noteIndex = pattern[banjoStep % patternLength];

  // Advance to the next step for next call
  banjoStep = (banjoStep + 1) % patternLength;

  // Return the actual note from the chord
  int* chord = chordNotes[chordIndex];
  if (noteIndex >= 0 && noteIndex < 4 && chord[noteIndex] != -1) {
    return chord[noteIndex];
  }

  // Fallback: return root if pattern index is invalid
  return chord[0];
}

// Call this to get the next note for a continuously ascending 8-note arpeggio per chord (max 2 octaves, resets)
int getArpNoteUp(int chordIndex) {
  static int arpStep = 0;       // total step count within phrase
  static int chordLength = 0;   // length of current chord
  static int* chord = nullptr;  // pointer to current chord
  static int octaveOffset = 0;  // octave shift for arpeggio

  // Select chord depending on major/minor mode
  int* currentChordNotes = useMinorMode ? chordNotes_minor[chordIndex] : chordNotes_major[chordIndex];

  // Load chord if it's a new chord
  if (chord != currentChordNotes) {
    chord = currentChordNotes;

    // Count valid chord notes
    chordLength = 0;
    for (int i = 0; i < 4; i++) {
      if (chord[i] == -1) break;
      chordLength++;
    }
    if (chordLength == 0) chordLength = 1;  // fallback

    arpStep = 0;       // reset step at new chord
    octaveOffset = 0;  // reset octave
  }

  // Determine which note to play (always ascending)
  int noteIndex = arpStep % chordLength;
  int note = chord[noteIndex] + octaveOffset;

  // Step to next note
  arpStep++;

  // When we cycle through the chord, increase octave for ascending arpeggio
  if (arpStep % chordLength == 0) {
    octaveOffset += 12;
    if (octaveOffset > 24) {  // max 2 octaves
      octaveOffset = 0;       // reset to starting octave
    }
  }

  return note;
}

// Call this to get the next note for a continuously descending 8-note arpeggio per chord (2 octaves, resets)
int getArpNoteDown(int chordIndex) {
  static int arpStep = 0;        // total step count within phrase
  static int chordLength = 0;    // length of current chord
  static int* chord = nullptr;   // pointer to current chord
  static int octaveOffset = 24;  // start 2 octaves above root

  // Select chord depending on major/minor mode
  int* currentChordNotes = useMinorMode ? chordNotes_minor[chordIndex] : chordNotes_major[chordIndex];

  // Load chord if it's a new chord
  if (chord != currentChordNotes) {
    chord = currentChordNotes;

    // Count valid chord notes
    chordLength = 0;
    for (int i = 0; i < 4; i++) {
      if (chord[i] == -1) break;
      chordLength++;
    }
    if (chordLength == 0) chordLength = 1;  // fallback

    arpStep = 0;        // reset step at new chord
    octaveOffset = 24;  // start 2 octaves above root
  }

  // Determine which note to play (always descending)
  int noteIndex = chordLength - 1 - (arpStep % chordLength);  // reverse order
  int note = chord[noteIndex] + octaveOffset;

  // Step to next note
  arpStep++;

  // When we cycle through the chord, decrease octave for descending arpeggio
  if (arpStep % chordLength == 0) {
    octaveOffset -= 12;
    if (octaveOffset < 0) {  // max 2 octaves down
      octaveOffset = 24;     // reset to top octave
    }
  }

  return note;
}

// Call this to get the next note for a continuously ascending then descending 8-note arpeggio per chord (2 octaves, resets)
int getArpNotePingPong(int chordIndex) {
  static int arpStep = 0;        // total step count within phrase
  static int chordLength = 0;    // length of current chord
  static int* chord = nullptr;   // pointer to current chord
  static int octaveOffset = 0;   // current octave shift
  static bool ascending = true;  // direction flag

  // Select chord depending on major/minor mode
  int* currentChordNotes = useMinorMode ? chordNotes_minor[chordIndex] : chordNotes_major[chordIndex];

  // Load chord if it's a new chord
  if (chord != currentChordNotes) {
    chord = currentChordNotes;

    // Count valid chord notes
    chordLength = 0;
    for (int i = 0; i < 4; i++) {
      if (chord[i] == -1) break;
      chordLength++;
    }
    if (chordLength == 0) chordLength = 1;  // fallback

    arpStep = 0;
    octaveOffset = 0;  // start at root octave
    ascending = true;  // start ascending
  }

  // Determine which note to play
  int noteIndex = ascending ? (arpStep % chordLength) : (chordLength - 1 - (arpStep % chordLength));
  int note = chord[noteIndex] + octaveOffset;

  // Step to next note
  arpStep++;

  // When we finish a chord cycle
  if (arpStep % chordLength == 0) {
    if (ascending) {
      octaveOffset += 12;        // move up one octave
      if (octaveOffset >= 24) {  // reached top octave
        ascending = false;       // start descending
      }
    } else {
      octaveOffset -= 12;       // move down one octave
      if (octaveOffset <= 0) {  // back to root octave
        ascending = true;       // start ascending again
      }
    }
  }

  return note;
}

// Call this to get the next note for a continuously descending then ascending 8-note arpeggio per chord (2 octaves, resets)
int getArpNoteReversePingPong(int chordIndex) {
  static int arpStep = 0;         // total step count within phrase
  static int chordLength = 0;     // length of current chord
  static int* chord = nullptr;    // pointer to current chord
  static int octaveOffset = 24;   // start 2 octaves above root
  static bool descending = true;  // direction flag

  // Select chord depending on major/minor mode
  int* currentChordNotes = useMinorMode ? chordNotes_minor[chordIndex] : chordNotes_major[chordIndex];

  // Load chord if it's a new chord
  if (chord != currentChordNotes) {
    chord = currentChordNotes;

    // Count valid chord notes
    chordLength = 0;
    for (int i = 0; i < 4; i++) {
      if (chord[i] == -1) break;
      chordLength++;
    }
    if (chordLength == 0) chordLength = 1;  // fallback

    arpStep = 0;
    octaveOffset = 24;  // start at top octave
    descending = true;  // start descending
  }

  // Determine which note to play
  int noteIndex = descending ? (chordLength - 1 - (arpStep % chordLength)) : (arpStep % chordLength);
  int note = chord[noteIndex] + octaveOffset;

  // Step to next note
  arpStep++;

  // When we finish a chord cycle
  if (arpStep % chordLength == 0) {
    if (descending) {
      octaveOffset -= 12;       // move down one octave
      if (octaveOffset <= 0) {  // reached root octave
        descending = false;     // start ascending
      }
    } else {
      octaveOffset += 12;        // move up one octave
      if (octaveOffset >= 24) {  // back to top octave
        descending = true;       // start descending again
      }
    }
  }

  return note;
}

// Procedurally generates the melody
int getMelodyNote(int chordIndex) {
  static int melodyStep = 0;
  static int phraseStep = 0;
  static int lastNote = 72;  // start safely in melody range
  static bool silentPhrase = false;
  static int phraseLength = 12;

  // --- Call and response variables ---
  static int callPhrase[16];  // stores current phrase notes
  static int callLength = 0;
  static bool inResponse = false;
  static int responseStep = 0;

  // --- phrase system ---
  if (phraseStep == 0) {
    phraseLength = random(4, 9);         // generate short phrase (4–8 notes)
    silentPhrase = (random(0, 4) == 0);  // ~25% chance of silent phrase

    // Generate new "call" phrase
    for (int i = 0; i < phraseLength; i++) {
      int* scale = useMinorMode
                     ? chordScales_minor[chordIndex]
                     : chordScales_major[chordIndex];
      int scaleLength = 0;
      while (scaleLength < 8 && scale[scaleLength] != -1) scaleLength++;

      int step = random(-2, 3);  // small step from lastNote
      int lastPC = lastNote % 12;
      int lastIndex = 0;
      for (int j = 0; j < scaleLength; j++) {
        if ((scale[j] % 12) == lastPC) {
          lastIndex = j;
          break;
        }
      }

      int noteIndex = (lastIndex + step + scaleLength) % scaleLength;
      int nextNote = scale[noteIndex];

      // Safe octave
      while (nextNote < 60) nextNote += 12;
      while (nextNote > 84) nextNote -= 12;

      callPhrase[i] = nextNote;
      lastNote = nextNote;
    }

    inResponse = false;
    responseStep = 0;
  }

  int currentNote;

  if (!inResponse) {
    // --- Call phase ---
    currentNote = callPhrase[phraseStep % phraseLength];

    // Move to response after finishing call
    if ((phraseStep + 1) % phraseLength == 0) {
      inResponse = true;
      responseStep = 0;
    }
  } else {
    // --- Response phase: slight variation on call phrase ---
    currentNote = callPhrase[responseStep % phraseLength];

    // Random slight pitch variation for response
    if (random(0, 2) == 0) currentNote += 1;       // +1 semitone
    else if (random(0, 2) == 1) currentNote -= 1;  // -1 semitone

    // Occasionally shift octave
    if (random(0, 10) == 0) currentNote += (random(0, 2) == 0 ? 12 : -12);

    // Clamp to melody range
    if (currentNote < 60) currentNote += 12;
    if (currentNote > 84) currentNote -= 12;

    responseStep++;
    if (responseStep >= phraseLength) {
      // Finished response, next phrase will be new call
      inResponse = false;
    }
  }

  phraseStep = (phraseStep + 1) % phraseLength;

  // --- apply silent phrase as a volume envelope instead of -1 ---
  if (silentPhrase) {
    // Phrase is silent → ALWAYS silence the melody oscillator
    osc3Volume = 0.0;

  } else if (osc3Muted) {
    // Manual mute toggle (from DIP/button)
    osc3Volume = 0.0;

  } else {
    // Normal active melody state
    osc3Volume = osc3Volume_orig;
  }


  melodyStep++;
  return currentNote;
}

//TO DO: edit drum sounds
void playDrum(DrumType drum) {
  switch (drum) {
    case KICK:
      noiseEnv.setADLevels(255, 0);
      noiseEnv.setTimes(1, 100, 0, 50);
      lfsr_divisor = 128;
      lfsr_countdown = lfsr_divisor;
      break;

    case SNARE:
      noiseEnv.setADLevels(200, 0);
      noiseEnv.setTimes(1, 50, 0, 50);
      lfsr_divisor = 32;
      lfsr_countdown = lfsr_divisor;
      break;

    case HIHAT:
      noiseEnv.setADLevels(100, 0);
      noiseEnv.setTimes(0, 20, 0, 20);
      lfsr_divisor = 2;
      lfsr_countdown = lfsr_divisor;
      break;

    case TOM:
      noiseEnv.setADLevels(255, 0);
      noiseEnv.setTimes(2, 120, 0, 40);
      lfsr_divisor = 95;
      lfsr_countdown = lfsr_divisor;
      break;

    case CLAP:
      noiseEnv.setADLevels(180, 0);
      noiseEnv.setTimes(0, 35, 0, 60);
      lfsr_divisor = 12;
      lfsr_countdown = lfsr_divisor;
      break;

    case RIM:
      noiseEnv.setADLevels(150, 0);
      noiseEnv.setTimes(0, 15, 0, 10);
      lfsr_divisor = 8;
      lfsr_countdown = lfsr_divisor;
      break;

    case COWBELL:
      noiseEnv.setADLevels(220, 0);
      noiseEnv.setTimes(1, 60, 0, 30);
      lfsr_divisor = 45;
      lfsr_countdown = lfsr_divisor;
      break;
  }
  noiseEnv.noteOn();
}

void loop() {
  audioHook();  // handles Mozzi audio generation behind the scenes
}

/** Called automatically at rate specified by CONTROL_RATE macro, most of your code should live in here
 */
void updateControl() {
  meap.readInputs();

  // Each time you read the potentiometer
  float potScaleTempo = meap.pot_vals[0] / 4095.0;  // 0.0–1.0

  // Adjust BPM based on pot, proportionally around randTempo
  // Let's say pot at 0 reduces BPM toward 40, pot at max increases BPM toward 180
  bpm = randTempo + (potScaleTempo - 0.5) * 2.0 * (randTempo - 40.0);

  // clamp to full range just in case
  if (bpm < 40.0) bpm = 40.0;
  if (bpm > 180.0) bpm = 180.0;

  if (halfTimeToggle) {
    sixteenth_length = (60.0 / bpm) / 2.0;  // recalc sixteenth note length
  } else {
    sixteenth_length = (60.0 / bpm) / 4.0;  // recalc sixteenth note length
  }

  // Read potentiometer and normalize
  float potScale = meap.pot_vals[1] / 4095.0;  // 0.0–1.0

  // Map pot so middle = randTranspose, extremes = +/-12 relative to center
  int transposer = randTranspose + (int)((potScale - 0.5) * 2.0 * 12);

  // Clamp to stay within valid range -12 → +12
  if (transposer < -12) transposer = -12;
  if (transposer > 12) transposer = 12;

  static bool lastMode = useMinorMode;

  if (useMinorMode != lastMode) {
    lastMode = useMinorMode;

    chordNotes = useMinorMode ? chordNotes_minor : chordNotes_major;
    nextChord = useMinorMode ? nextChord_minor : nextChord_major;
    chordProbabilities = useMinorMode ? chordProbabilities_minor : chordProbabilities_major;
  }

  static int bassCounter = 0;  // keep track of bass timing

  if (metro.ready()) {
    metro.start(sixteenth_length * 1000);  // reset metro

    // --- Bass ---
    bassCounter++;
    if (bassCounter % 2 == 0) {  // only trigger every 2 metro ticks (half speed)

      int bassNote;

      switch (bassMode) {
        case 1:
          bassNote = getBassNote(currentChord);
          break;

        case 2:
          bassNote = getBassNoteOct(currentChord);
          break;

        case 3:
          bassNote = getBassNoteRootFifth(currentChord);
          break;

        default:
          bassNote = getBassNote(currentChord);
          break;
      }

      bassNote -= 24;  // pitch notes down two octaves to mimic bass range
      playRoot = !playRoot;
      osc1.setFreq(mtof(bassNote + transposer));  // apply global transpose
      env1.noteOn();
      //Serial.print("Bass note: ");
      //Serial.println(bassNote);
    }

    // --- Banjo & Arps ---
    // int banjoNote = getBanjoNote(currentChord);
    // banjoNote += 12;                             // pitch notes up one octave to mimic banjo range
    // osc2.setFreq(mtof(banjoNote + transposer));  // apply global

    //Serial.print("Banjo note: ");
    //Serial.println(banjoNote);

    int arpNote;

    switch (arpMode) {
      case 1:  // Banjo note
        arpNote = getBanjoNote(currentChord);
        break;

      case 2:  // Arpeggio ascending
        arpNote = getArpNoteUp(currentChord);
        break;

      case 3:  // Arpeggio descending
        arpNote = getArpNoteDown(currentChord);
        break;

      case 4:  // Ping-pong arpeggio
        arpNote = getArpNotePingPong(currentChord);
        break;

      case 5:  // Reverse ping-pong arpeggio
        arpNote = getArpNoteReversePingPong(currentChord);
        break;

      default:
        arpNote = getBanjoNote(currentChord);
        break;
    }

    arpNote += 12;                             // pitch notes up one octave to mimic banjo range
    osc2.setFreq(mtof(arpNote + transposer));  // apply global transpose
    env2.noteOn();

    // --- Melody ---
    int melodyNote = getMelodyNote(currentChord);
    melodyNote += 12;                             // pitch notes up one octave to mimic banjo range
    osc3.setFreq(mtof(melodyNote + transposer));  // apply global transpose
    env3.noteOn();

    // --- Chord letters ---
    const char* chordLetters_major[] = { "G", "C", "D", "D7", "A7", "B7", "Em", "Am", "Bm" };
    const char* chordLetters_minor[] = { "Gm", "Cm", "Dm", "D7", "A7", "B7", "Ebm", "Abm", "Bbm" };

    // --- Chord changes ---
    beatCounter++;
    if (beatCounter % beatsPerChord == 0) {
      currentChord = getNextChord();

      const char** chordLetters = useMinorMode ? chordLetters_minor : chordLetters_major;

      Serial.print("New chord: ");
      Serial.println(chordLetters[currentChord]);
    }

    DrumType drumSequence[] = { KICK, HIHAT, SNARE, HIHAT };  // cycles through

    // --- Drum pattern ---
    playDrum(drumSequence[drumStep]);
    drumStep++;
    if (drumStep >= 3) drumStep = 0;  // loop sequence

    // // --- Trigger noise synced to metronome ---
    // // current config: trigger every quarter note
    // static int noiseCounter = 0;
    // noiseCounter++;
    // if (noiseCounter % 4 == 0) {
    //   noiseEnv.setADLevels(255, 0);
    //   noiseEnv.setTimes(5, 100, 0, 0);
    //   noiseEnv.noteOn();
    // }
  }

  // Update volumes
  updateOscillatorVolumes();
  // Update envelopes
  noiseEnv.update();
}

/** Called automatically at rate specified by AUDIO_RATE macro, for calculating samples sent to DAC, too much code in here can disrupt your output
 */
AudioOutput_t updateAudio() {
  env1.update();
  env2.update();
  env3.update();
  int16_t noiseSample = (gameboyNoise() * noiseEnv.next());

  int16_t mixed = (osc1.next() * osc1Volume * env1.next()) + (osc2.next() * osc2Volume * env2.next())
                  + (osc3.next() * osc3Volume * env3.next()) + (noiseSample * noiseOscVolume);

  int64_t out_sample = mixed;  // extra effects + mixing if needed
  // int64_t out_sample = noiseSample;
  return StereoOutput::fromNBit(14, (out_sample * meap.volume_val) >> 12, (out_sample * meap.volume_val) >> 12);
}

/**
 * Runs whenever a touch pad is pressed or released
 *
 * int number: the number (0-7) of the pad that was pressed
 * bool pressed: true indicates pad was pressed, false indicates it was released
 */
void updateTouch(int number, bool pressed) {
  if (pressed) {  // Any pad pressed
  } else {        // Any pad released
  }
  switch (number) {
    case 0:
      if (pressed) {  // Pad 0 pressed
        Serial.println("t0 pressed ");
      } else {  // Pad 0 released
        Serial.println("t0 released");
      }
      break;
    case 1:
      if (pressed) {  // Pad 1 pressed
        Serial.println("t1 pressed");
      } else {  // Pad 1 released
        Serial.println("t1 released");
      }
      break;
    case 2:
      if (pressed) {  // Pad 2 pressed
        Serial.println("t2 pressed");
      } else {  // Pad 2 released
        Serial.println("t2 released");
      }
      break;
    case 3:
      if (pressed) {  // Pad 3 pressed
        Serial.println("t3 pressed");
      } else {  // Pad 3 released
        Serial.println("t3 released");
      }
      break;
    case 4:
      if (pressed) {  // Pad 4 pressed
        Serial.println("t4 pressed");
      } else {  // Pad 4 released
        Serial.println("t4 released");
      }
      break;
    case 5:
      if (pressed) {  // Pad 5 pressed
        Serial.println("t5 pressed");
      } else {  // Pad 5 released
        Serial.println("t5 released");
      }
      break;
    case 6:
      if (pressed) {  // Pad 6 pressed
        Serial.println("t6 pressed");
      } else {  // Pad 6 released
        Serial.println("t6 released");
      }
      break;
    case 7:
      if (pressed) {  // Pad 7 pressed
        Serial.println("t7 pressed");
      } else {  // Pad 7 released
        Serial.println("t7 released");
      }
      break;
  }
}

/**
 * Runs whenever a DIP switch is toggled
 *
 * int number: the number (0-7) of the switch that was toggled
 * bool up: true indicated switch was toggled up, false indicates switch was toggled
 */
void updateDip(int number, bool up) {
  if (up) {  // Any DIP toggled up
  } else {   // Any DIP toggled down
  }
  switch (number) {
    case 0:
      if (up) {                                                        // DIP 0 up
        randTempo = 40.0 + ((float)rand() / RAND_MAX) * 140.0;         // random 40–180 BPM
        randTranspose = -12 + (int)(((float)rand() / RAND_MAX) * 25);  // -12 to 12
        useMinorMode = (rand() % 2 == 0);                              // Randomly set useMinorMode to true or false
        oscMode = 1 + rand() % 6;                                      // generates 1–6
        arpMode = 1 + rand() % 5;                                      // generates 1-5
        bassMode = 1 + rand() % 3;                                     // generates 1-3
        //updates the mode
        oscModeUpdater();
        randomizeEnvelopes();

        // --- PRINT RESULTS ---
        Serial.println("=== NEW RANDOM PARAMETERS ===");

        // ---- PRINT RANDOMIZED PARAMETERS ----
        Serial.print("Random Tempo (BPM): ");
        Serial.println(randTempo);

        Serial.print("Random Transpose: ");
        Serial.println(randTranspose);

        // Minor / Major
        Serial.print("Mode: ");
        Serial.println(useMinorMode ? "Minor" : "Major");

        Serial.print("Oscillator Mode: ");
        Serial.print(oscMode);
        Serial.print(" — ");

        switch (bassMode) {
          case 1:
            Serial.println("Bass Mode 1: Bass Note (Root note of the chord without any octave changes)");
            break;

          case 2:
            Serial.println("Bass Mode 2: Bass Note Octaves (Root note of the chord with octave changes)");
            break;

          case 3:
            Serial.println("Bass Mode 3: Bass Note Root/Fifth (Bass note of the chord with alternating 5th changes)");
            break;

          default:
            Serial.println("Bass Mode: Unknown mode");
            break;
        }


        switch (arpMode) {
          case 1:
            Serial.println("Arp Mode 1: Banjo note (single note from chord)");
            break;

          case 2:
            Serial.println("Arp Mode 2: Arp Up (ascending arpeggio)");
            break;

          case 3:
            Serial.println("Arp Mode 3: Arp Down (descending arpeggio)");
            break;

          case 4:
            Serial.println("Arp Mode 4: Ping-Pong arpeggio (forward/backward)");
            break;

          case 5:
            Serial.println("Arp Mode 5: Reverse Ping-Pong arpeggio");
            break;

          default:
            Serial.println("Arp Mode: Unknown mode");
            break;
        }


        switch (oscMode) {
          case 1:
            Serial.println("All oscillators ON (Bass + Mid + Lead + Noise)");
            break;

          case 2:
            Serial.println("Bass + Lead only (no Mid, no Noise)");
            break;

          case 3:
            Serial.println("Lead only (solo melody)");
            break;

          case 4:
            Serial.println("Bass + Mid synth");
            break;

          case 5:
            Serial.println("Mid synth only (banjo/mid)");
            break;

          case 6:
            Serial.println("White noise only (ocean sounds)");
            break;

          default:
            Serial.println("Unknown mode");
            break;
        }

        Serial.println("============================");

        Serial.println("d0 up");
      } else {  // DIP 0
        Serial.println("d0 down");
      }
      break;
    case 1:
      if (up) {  // DIP 1 up
        useMinorMode = true;
        Serial.print("Mode switched to minor");
        Serial.println("d1 up");
      } else {  // DIP 1 down
        useMinorMode = false;
        Serial.print("Mode switched to major");
        Serial.println("d1 down");
      }
      break;
    case 2:
      if (up) {  // DIP 2 up
        Serial.println("d2 up");
      } else {  // DIP 2 down
        Serial.println("d2 down");
      }
      break;
    case 3:
      if (up) {  // DIP 3 up
        halfTimeToggle = true;
        Serial.println("toggled half time (d3 up)");
        Serial.println("d3 up");
      } else {  // DIP 3 down
        halfTimeToggle = false;
        Serial.println("toggled normal time (d3 up)");
        Serial.println("d3 down");
      }
      break;
    case 4:  // Bass mute toggle
      if (up) {
        osc1Muted = true;
        Serial.println("Bass muted (d4 up)");
      } else {
        osc1Muted = false;
        Serial.println("Bass unmuted (d4 down)");
      }
      break;

    case 5:  // Banjo/Mid mute toggle
      if (up) {
        osc2Muted = true;
        Serial.println("Middle Osc muted (d5 up)");
      } else {
        osc2Muted = false;
        Serial.println("Middle Osc unmuted (d5 down)");
      }
      break;

    case 6:  // Melody mute toggle
      if (up) {
        osc3Muted = true;
        Serial.println("Melody muted (d6 up)");
      } else {
        osc3Muted = false;
        Serial.println("Melody unmuted (d6 down)");
      }
      break;

    case 7:  // Noise mute toggle
      if (up) {
        oscNoiseMuted = true;
        Serial.println("Noise muted (d7 up)");
      } else {
        oscNoiseMuted = false;
        Serial.println("Noise unmuted (d7 down)");
      }
      break;
  }
}