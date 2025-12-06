#define CONTROL_RATE 128 // Hz, powers of 2 are most reliable
#include <Meap.h> // MEAP library, includes all dependent libraries, including all Mozzi modules
#include <stdlib.h> // for rand()
#include <tables/sq8192_32harm_int8.h>

mOscil<sq8192_32harm_int8_NUM_CELLS, AUDIO_RATE, int8_t>
    osc1(sq8192_32harm_int8_DATA); // Bass
mOscil<sq8192_32harm_int8_NUM_CELLS, AUDIO_RATE, int8_t>
    osc2(sq8192_32harm_int8_DATA); // Mid
mOscil<sq8192_32harm_int8_NUM_CELLS, AUDIO_RATE, int8_t>
    osc3(sq8192_32harm_int8_DATA); // Melody

// Envelopes
ADSR<AUDIO_RATE, AUDIO_RATE> env1;
ADSR<AUDIO_RATE, AUDIO_RATE> env2;
ADSR<AUDIO_RATE, AUDIO_RATE> env3;
ADSR<AUDIO_RATE, AUDIO_RATE> noiseEnv;

// RANDOM VARIABLES:
int randTempo = 40.0 + ((float)rand() / RAND_MAX) * 140.0; // random 40–180 BPM
int randTranspose = -12 + (int)(((float)rand() / RAND_MAX) * 25); // -12 to 12;
int oscMode = 0;  // keeps track of oscillator modes
int arpMode = 0;  // keeps track of what arp mode to use
int bassMode = 0; // keeps track of what bass mode to use
int drumMode = 0; // keeps track of what bass mode to use
int randProg = 0; // keeps track of what chord progression to use

// MIXER
float osc1Volume_orig = 0.35;    // bass
float osc2Volume_orig = 0.1;     // mid
float osc3Volume_orig = 0.5;     // melody
float noiseOscVolume_orig = 0.4; // noise

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
static bool silentPhrase = false;

// ---------- LFSR STATE ----------
uint16_t lfsr = 0xACE1u; // 16-bit LFSR seed
int lfsr_divisor = 32;   // equivalent to reset value for timing
int lfsr_countdown = 0;  // counts down to next LFSR update
int lfsr_last = 0;       // last output sample (+127 or -127)
bool short_mode = false; // optional short LFSR mode

// DRUM SEQUENCING GLOBALS
int drumStep = 0;           // keeps track of which drum to play
int stepCounter = 0;        // counts beats
const int beatsPerStep = 4; // advance every 16 beats

enum DrumType {
  KICK,
  SNARE,
  HIHAT,
  TOM,
  CLAP,
  RIM,
  COWBELL,
};

// Map DrumType enum to string names
const char *drumTypeNames[] = {"KICK", "SNARE", "HIHAT",  "TOM",
                               "CLAP", "RIM",   "COWBELL"};

// ---------- Active sequence ----------
DrumType *activeSequence;
uint8_t sequenceLength;

Meap meap; // creates MEAP object to handle inputs and other MEAP library
           // functions
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1,
                     MIDI); // defines MIDI in/out ports

// ---------- YOUR GLOBAL VARIABLES BELOW ----------
float bpm = 100.0;      // default starting tempo
float sixteenth_length; // seconds per 16th note

// --- BASS VARIABLES ---
bool useFifth = false;          // toggles between root and fifth
int bassNote = 0;               // current bass note MIDI number
int bassChord = 0;              // chord being used for bass
unsigned long lastBassTime = 0; // last time bass note triggered
int bassInterval = 250;         // time between notes (ms) ~120BPM

// --- BANJO VARIABLES ---
int banjoStep = 0;
int banjoLastNote = -1;

const int numPatterns = 4;
int currentPattern = 0; // select pattern per chord

int transposer = 0; // global semitone transpose

int currentProgression = 1; // index of the active chord progression
int progressionIndex = 0;   // index within that progression

int currentChord = 0;    // the currently playing chord
int nextTargetChord = 0; // The UPCOMING chord (for lookahead)
bool playRoot = true;    // alternates root/fifth

EventDelay metro;      // metronome for bass + chord timing
int beatsPerChord = 8; // change chord every 8 beats
int beatCounter = 0;

/*****ADDS ALL CHORD AND SCALE DATA*****/
#include "ChordData.h"

bool useMinorMode = false; // set true for minor, false for major

int (*chordNotes)[4] = useMinorMode ? chordNotes_minor : chordNotes_major;
int (*nextChord)[6] = useMinorMode ? nextChord_minor : nextChord_major;
int (*chordProbabilities)[6] = useMinorMode ? chordProbabilities_minor
                                            : chordProbabilities_major;

int chordMode = 0;
bool newProg = false;

// Global state to keep track of current pattern and step
int currentBanjoPattern = 0;

// MIDI Note tracking
int lastBassMidi = -1;
int lastArpMidi = -1;
int lastMelodyMidi = -1;

// Melody Rhythm
int melodyWaitSteps = 0;

// Envelope State
int env3Atk = 10;
int env3Dec = 30;
int env3Sus = 100;
int env3Rel = 30;

// Melody Phrase State
int phraseStep = 0;
int phraseLength = 8;
bool inResponse = false;
int rhythmPhrase[16]; // Stores step duration for each note in phrase

void setup() {
  Serial.begin(115200); // begins Serial communication with computer
  meap.begin();         // sets up MEAP object
  Serial1.begin(31250, SERIAL_8N1, meap.MEAP_MIDI_IN_PIN,
                meap.MEAP_MIDI_OUT_PIN); // sets up MIDI: baud rate, serial
                                         // mode, rx pin, tx pin
  startMozzi(
      CONTROL_RATE); // starts Mozzi engine with control rate defined above

  sixteenth_length = (60.0 / bpm) / 4.0; // seconds per 16th
  metro.start(sixteenth_length * 1000);  // start metro (ms)

  // randomizes oscillator envelopes
  randomizeEnvelopes();

  // Prime the chord pipeline
  currentChord = 0;                 // Start on I
  nextTargetChord = getNextChord(); // Prepare next
}

// Helper to print the progression and arrow
void printProgression(int progID, int arrowIndex) {
  const char **chordLetters =
      useMinorMode ? chordLetters_minor : chordLetters_major;
  int(*chordProgressions)[8] =
      useMinorMode ? chordProgressions_minor : chordProgressions_major;

  // --- Print the full 8-chord progression ---
  printf("Progression %2d: ", progID);
  for (int i = 0; i < 8; i++) {
    int c = chordProgressions[progID][i];
    printf("%s ", chordLetters[c]);
  }
  printf("\n");

  // --- Print arrow under current chord ---
  printf("                "); // align under "Progression XX: " (16 chars)
  for (int i = 0; i < 8; i++) {
    int c = chordProgressions[progID][i];
    int len = strlen(chordLetters[c]);
    if (i == arrowIndex) {
      printf("^");
      for (int j = 1; j < len; j++)
        printf(" "); // pad to match chord width
    } else {
      for (int j = 0; j < len; j++)
        printf(" "); // spaces for other chords
    }
    printf(" "); // space between chords
  }
  printf("\n");
}

// chordMode: 0 = predefined progression, 1 = random transitions
// useMinorMode: true = minor, false = major
int getNextChord() {
  int chord = 0;
  const char **chordLetters =
      useMinorMode ? chordLetters_minor : chordLetters_major;
  int(*chordProgressions)[8] =
      useMinorMode ? chordProgressions_minor : chordProgressions_major;

  // If we're at the start of a new progression (progressionIndex == 0), pick a
  // random progression
  if (newProg) {
    currentProgression = randProg;
    progressionIndex = 0;
    newProg = false;
  }

  if (chordMode == 0) { // predefined progression
    chord = chordProgressions[currentProgression][progressionIndex];

    // increment index for next call
    progressionIndex = (progressionIndex + 1) % 8;

  } else { // random transition mode
    int randVal = rand() % 100;

    if (!useMinorMode) { // Major
      for (int i = 0; i < 6; i++) {
        if (nextChord_major[currentChord][i] == -1)
          break;
        if (randVal < chordProbabilities_major[currentChord][i]) {
          chord = nextChord_major[currentChord][i];
          currentChord = chord;
          break;
        }
      }
    } else { // Minor
      for (int i = 0; i < 6; i++) {
        if (nextChord_minor[currentChord][i] == -1)
          break;
        if (randVal < chordProbabilities_minor[currentChord][i]) {
          chord = nextChord_minor[currentChord][i];
          currentChord = chord;
          break;
        }
      }
    }
  }

  return chord; // fallback to 0 (G or Gm)
}

// Returns the root note of the chord without any octave changes
int getBassNote(int chordIndex) { return chordNotes[chordIndex][0]; }

// Returns the root note of the chord with octave changes
int getBassNoteOct(int chordIndex) {
  int root = chordNotes[chordIndex][0];

  int r = random(0, 4); //
  if (r == 0) {         // 25% chance
    root += 12;         // up an octave
  } else if (r == 1) {  // 25% chance
    root -= 12;         // down an octave
  }

  return root;
}

// Returns the bass note of the chord with alternating root fifth changes
int getBassNoteRootFifth(int chordIndex) {
  static bool returnRoot = true; // remembers last choice

  int root = chordNotes[chordIndex][0];
  int fifth = root + 7;

  if (random(0, 2) == 0) { // randomly drop fifth an octave
    fifth -= 12;
  }

  int result = returnRoot ? root : fifth;
  returnRoot = !returnRoot; // alternate next time
  return result;
}

// Call this to get the next note for the banjo
int getBanjoNote(int chordIndex) {
  // Select a random pattern if we're at the start of a new roll
  if (banjoStep == 0) {
    currentBanjoPattern =
        random(0, sizeof(banjoPatterns) / sizeof(banjoPatterns[0]));
  }

  const int *pattern = banjoPatterns[currentBanjoPattern];

  // Find the valid length of the pattern (stop at -1)
  int patternLength = 0;
  for (int i = 0; i < 6; i++) {
    if (pattern[i] == -1)
      break;
    patternLength++;
  }

  // Get the note index from the pattern
  int noteIndex = pattern[banjoStep % patternLength];

  // Advance to the next step for next call
  banjoStep = (banjoStep + 1) % patternLength;

  // Return the actual note from the chord
  int *chord = chordNotes[chordIndex];
  if (noteIndex >= 0 && noteIndex < 4 && chord[noteIndex] != -1) {
    return chord[noteIndex];
  }

  // Fallback: return root if pattern index is invalid
  return chord[0];
}

// Call this to get the next note for a continuously ascending 8-note arpeggio
// per chord (max 2 octaves, resets)
int getArpNoteUp(int chordIndex) {
  static int arpStep = 0;      // total step count within phrase
  static int chordLength = 0;  // length of current chord
  static int *chord = nullptr; // pointer to current chord
  static int octaveOffset = 0; // octave shift for arpeggio

  // Select chord depending on major/minor mode
  int *currentChordNotes = useMinorMode ? chordNotes_minor[chordIndex]
                                        : chordNotes_major[chordIndex];

  // Load chord if it's a new chord
  if (chord != currentChordNotes) {
    chord = currentChordNotes;

    // Count valid chord notes
    chordLength = 0;
    for (int i = 0; i < 4; i++) {
      if (chord[i] == -1)
        break;
      chordLength++;
    }
    if (chordLength == 0)
      chordLength = 1; // fallback

    arpStep = 0;      // reset step at new chord
    octaveOffset = 0; // reset octave
  }

  // Determine which note to play (always ascending)
  int noteIndex = arpStep % chordLength;
  int note = chord[noteIndex] + octaveOffset;

  // Step to next note
  arpStep++;

  // When we cycle through the chord, increase octave for ascending arpeggio
  if (arpStep % chordLength == 0) {
    octaveOffset += 12;
    if (octaveOffset > 24) { // max 2 octaves
      octaveOffset = 0;      // reset to starting octave
    }
  }

  return note;
}

// Call this to get the next note for a continuously descending 8-note arpeggio
// per chord (2 octaves, resets)
int getArpNoteDown(int chordIndex) {
  static int arpStep = 0;       // total step count within phrase
  static int chordLength = 0;   // length of current chord
  static int *chord = nullptr;  // pointer to current chord
  static int octaveOffset = 24; // start 2 octaves above root

  // Select chord depending on major/minor mode
  int *currentChordNotes = useMinorMode ? chordNotes_minor[chordIndex]
                                        : chordNotes_major[chordIndex];

  // Load chord if it's a new chord
  if (chord != currentChordNotes) {
    chord = currentChordNotes;

    // Count valid chord notes
    chordLength = 0;
    for (int i = 0; i < 4; i++) {
      if (chord[i] == -1)
        break;
      chordLength++;
    }
    if (chordLength == 0)
      chordLength = 1; // fallback

    arpStep = 0;       // reset step at new chord
    octaveOffset = 24; // start 2 octaves above root
  }

  // Determine which note to play (always descending)
  int noteIndex = chordLength - 1 - (arpStep % chordLength); // reverse order
  int note = chord[noteIndex] + octaveOffset;

  // Step to next note
  arpStep++;

  // When we cycle through the chord, decrease octave for descending arpeggio
  if (arpStep % chordLength == 0) {
    octaveOffset -= 12;
    if (octaveOffset < 0) { // max 2 octaves down
      octaveOffset = 24;    // reset to top octave
    }
  }

  return note;
}

// Call this to get the next note for a continuously ascending then descending
// 8-note arpeggio per chord (2 octaves, resets)
int getArpNotePingPong(int chordIndex) {
  static int arpStep = 0;       // total step count within phrase
  static int chordLength = 0;   // length of current chord
  static int *chord = nullptr;  // pointer to current chord
  static int octaveOffset = 0;  // current octave shift
  static bool ascending = true; // direction flag

  // Select chord depending on major/minor mode
  int *currentChordNotes = useMinorMode ? chordNotes_minor[chordIndex]
                                        : chordNotes_major[chordIndex];

  // Load chord if it's a new chord
  if (chord != currentChordNotes) {
    chord = currentChordNotes;

    // Count valid chord notes
    chordLength = 0;
    for (int i = 0; i < 4; i++) {
      if (chord[i] == -1)
        break;
      chordLength++;
    }
    if (chordLength == 0)
      chordLength = 1; // fallback

    arpStep = 0;
    octaveOffset = 0; // start at root octave
    ascending = true; // start ascending
  }

  // Determine which note to play
  int noteIndex = ascending ? (arpStep % chordLength)
                            : (chordLength - 1 - (arpStep % chordLength));
  int note = chord[noteIndex] + octaveOffset;

  // Step to next note
  arpStep++;

  // When we finish a chord cycle
  if (arpStep % chordLength == 0) {
    if (ascending) {
      octaveOffset += 12;       // move up one octave
      if (octaveOffset >= 24) { // reached top octave
        ascending = false;      // start descending
      }
    } else {
      octaveOffset -= 12;      // move down one octave
      if (octaveOffset <= 0) { // back to root octave
        ascending = true;      // start ascending again
      }
    }
  }

  return note;
}

// Call this to get the next note for a continuously descending then ascending
// 8-note arpeggio per chord (2 octaves, resets)
int getArpNoteReversePingPong(int chordIndex) {
  static int arpStep = 0;        // total step count within phrase
  static int chordLength = 0;    // length of current chord
  static int *chord = nullptr;   // pointer to current chord
  static int octaveOffset = 24;  // start 2 octaves above root
  static bool descending = true; // direction flag

  // Select chord depending on major/minor mode
  int *currentChordNotes = useMinorMode ? chordNotes_minor[chordIndex]
                                        : chordNotes_major[chordIndex];

  // Load chord if it's a new chord
  if (chord != currentChordNotes) {
    chord = currentChordNotes;

    // Count valid chord notes
    chordLength = 0;
    for (int i = 0; i < 4; i++) {
      if (chord[i] == -1)
        break;
      chordLength++;
    }
    if (chordLength == 0)
      chordLength = 1; // fallback

    arpStep = 0;
    octaveOffset = 24; // start at top octave
    descending = true; // start descending
  }

  // Determine which note to play
  int noteIndex = descending ? (chordLength - 1 - (arpStep % chordLength))
                             : (arpStep % chordLength);
  int note = chord[noteIndex] + octaveOffset;

  // Step to next note
  arpStep++;

  // When we finish a chord cycle
  if (arpStep % chordLength == 0) {
    if (descending) {
      octaveOffset -= 12;      // move down one octave
      if (octaveOffset <= 0) { // reached root octave
        descending = false;    // start ascending
      }
    } else {
      octaveOffset += 12;       // move up one octave
      if (octaveOffset >= 24) { // back to top octave
        descending = true;      // start descending again
      }
    }
  }

  return note;
}

// Generates a rhythmic pattern for the phrase
void generateRhythm(int len) {
  int patternType = random(0, 5);

  for (int i = 0; i < len; i++) {
    switch (patternType) {
    case 0: // Straight 16ths (Fast)
      rhythmPhrase[i] = 1;
      break;
    case 1: // Straight 8ths
      rhythmPhrase[i] = 2;
      break;
    case 2: // Gallop (16th, 16th, 8th) -> 4 steps total
      if (i % 3 == 0)
        rhythmPhrase[i] = 1; // 16th
      else if (i % 3 == 1)
        rhythmPhrase[i] = 1; // 16th
      else
        rhythmPhrase[i] = 2; // 8th
      break;
    case 3: // Syncopated 16ths (3-3-2 sixteenths)
      // This is tricky with step counts, let's do a repeating 8th-16th pattern
      if (i % 2 == 0)
        rhythmPhrase[i] = 2; // 8th
      else
        rhythmPhrase[i] = 1; // 16th
      break;
    case 4: // Mixed Fast
      if (random(0, 2) == 0)
        rhythmPhrase[i] = 1;
      else
        rhythmPhrase[i] = 2;
      break;
    }
  }
}

// -------------------------------------------------------------------------
// MELODY GENERATION ENGINE
// -------------------------------------------------------------------------
// This function is a bit weird, so here's an explanation of how it works.
//
// 1. PHRASE GENERATION (The Call):
//    - pick a random rhythm and length.
//    - look ahead at the next chord to find "Common Tones" so the melody
//      doesn't sound disjointed when the chord changes.
//    - Basically a random walk, but force the last note to resolve nicely.
//
// 2. PLAYBACK (The Response):
//    - When responding, just replay the call but mess it up slightly
//      (pitch shifts, octave jumps) to make it sound like an answer.
//
// 3. SAFETY NET (Harmonic Quantization):
//    - Before playing ANY note, check if it fits the CURRENT chord.
//    - If the phrase runs long and crashes into a new chord, snap the
//      note to the nearest valid scale tone so it doesn't sound bad.
// -------------------------------------------------------------------------
int getMelodyNote(int chordIndex) {
  static int melodyStep = 0;
  static int lastNote = 72;

  // --- Call and response variables ---
  static int callPhrase[16];
  static int callLength = 0;
  static int responseStep = 0;

  // --- phrase system ---
  if (phraseStep == 0 &&
      !inResponse) { // Only gen new parameters at start of CALL
    phraseLength = random(4, 9);
    silentPhrase = (random(0, 3) == 0);

    // Retrieve NEXT chord's scale for lookahead
    int *nextScale = useMinorMode ? chordScales_minor[nextTargetChord]
                                  : chordScales_major[nextTargetChord];

    // Generate Rhythm for this phrase (and the response)
    generateRhythm(phraseLength);

    // Generate new "call" phrase
    for (int i = 0; i < phraseLength; i++) {
      int *scale = useMinorMode ? chordScales_minor[chordIndex]
                                : chordScales_major[chordIndex];
      int scaleLength = 0;
      while (scaleLength < 8 && scale[scaleLength] != -1)
        scaleLength++;

      int lastPC = lastNote % 12;
      int lastIndex = 0;
      for (int j = 0; j < scaleLength; j++) {
        if ((scale[j] % 12) == lastPC) {
          lastIndex = j;
          break;
        }
      }

      int nextNote = -1;

      // --- SMART VOICE LEADING ---
      // If past the halfway point, try to find a note that helps bridge
      // to the next chord by looking for "Common Tones" (notes shared by
      // current and next scale).
      if (i > phraseLength / 2 &&
          random(0, 10) < 7) { // 70% chance to try common tone logic

        // Try neighbors
        for (int step = -2; step <= 2; step++) {
          if (step == 0)
            continue;
          int tryIndex = (lastIndex + step + scaleLength) % scaleLength;
          int tryNote = scale[tryIndex];
          int tryPC = tryNote % 12;

          // Check if this PC exists in nextScale
          for (int k = 0; k < 8; k++) {
            if (nextScale[k] == -1)
              break;
            if ((nextScale[k] % 12) == tryPC) {
              // FOUND COMMON TONE!
              nextNote = tryNote;
              break;
            }
          }
          if (nextNote != -1)
            break;
        }
      }

      // Fallback if no common tone selected
      if (nextNote == -1) {
        if (i == phraseLength - 1) {
          // --- RESOLUTION STEP (Existing Logic) ---
          int stableIndices[] = {0, 2, 4, 7};
          int stableIdx = stableIndices[random(0, 4)];
          nextNote = scale[stableIdx];

          // Match octave
          while (nextNote < lastNote - 6)
            nextNote += 12;
          while (nextNote > lastNote + 6)
            nextNote -= 12;
        } else {
          // Normal random walk
          int step = random(-2, 3);
          int noteIndex = (lastIndex + step + scaleLength) % scaleLength;
          nextNote = scale[noteIndex];
        }
      }

      // Safe octave clamp
      while (nextNote < 60)
        nextNote += 12;
      while (nextNote > 84)
        nextNote -= 12;

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
    if (random(0, 2) == 0)
      currentNote += 1; // +1 semitone
    else if (random(0, 2) == 1)
      currentNote -= 1; // -1 semitone

    // Occasionally shift octave
    if (random(0, 10) == 0)
      currentNote += (random(0, 2) == 0 ? 12 : -12);

    // Clamp to melody range
    if (currentNote < 60)
      currentNote += 12;
    if (currentNote > 84)
      currentNote -= 12;

    responseStep++;
    if (responseStep >= phraseLength) {
      // Finished response, next phrase will be new call
      inResponse = false;
    }
  }

  phraseStep = (phraseStep + 1) % phraseLength;

  if (silentPhrase) {
    // Phrase is silent: silence the melody oscillator
    osc3Volume = 0.0;

  } else if (osc3Muted) {
    // Manual mute toggle (from DIP/button)
    osc3Volume = 0.0;

  } else {
    // Normal active melody state
    osc3Volume = osc3Volume_orig;
  }

  melodyStep++;

  // --- HARMONIC QUANTIZATION ---
  // Ensure the note we are about to play actually fits the CURRENT chord scale.
  // This handles cases where a phrase generated for chord A flows over into
  // chord B.

  int *currentScale = useMinorMode ? chordScales_minor[chordIndex]
                                   : chordScales_major[chordIndex];
  int notePC = currentNote % 12;
  bool inScale = false;

  // Check if PC is in scale
  for (int k = 0; k < 8; k++) {
    if (currentScale[k] == -1)
      break;
    if ((currentScale[k] % 12) == notePC) {
      inScale = true;
      break;
    }
  }

  // If clashing, snap to nearest scale tone
  if (!inScale) {
    int minDist = 100;
    int bestMatch = currentNote;

    for (int k = 0; k < 8; k++) {
      if (currentScale[k] == -1)
        break;
      // need to compare against the scale note in the same octave as
      // currentNote Construct a candidate in the current octave
      int scalePC = currentScale[k] % 12;
      int candidate = currentNote - notePC + scalePC;

      // Check neighbors if boundary crossing
      if (abs(candidate - currentNote) > 6) {
        if (candidate > currentNote)
          candidate -= 12;
        else
          candidate += 12;
      }

      int dist = abs(candidate - currentNote);
      if (dist < minDist) {
        minDist = dist;
        bestMatch = candidate;
      }
    }
    currentNote = bestMatch;
  }

  return currentNote;
}

// Generates a pseudo-random Gameboy-style noise sample using a 15-bit Linear
// Feedback Shift Register (LFSR) Basically this outputs 127 or -127 for each
// sample. this back and forth creates a white noise effect
int8_t gameboyNoise() {
  if (--lfsr_countdown <= 0) {
    lfsr_countdown = lfsr_divisor;
    uint8_t bit = ((lfsr >> 0) ^ (lfsr >> 1)) &
                  1; // XOR the first two bits to create feedback
    lfsr = (lfsr >> 1) |
           (bit << 14); // Shift LFSR right and insert feedback bit at the top
    if (short_mode) {
      lfsr = (lfsr & 0b10111111) | (bit << 6);
    }
    lfsr_last = (lfsr & 1) ? 127 : -127;
  }
  return lfsr_last; // Output +127 or -127 as a noise sample (audio amplitude
                    // for a Gameboy-style hiss)
}

// updates the oscillator volumes for mute functionality
void updateOscillatorVolumes() {
  osc1Volume = osc1Muted ? 0.0 : osc1Volume_orig;
  osc2Volume = osc2Muted ? 0.0 : osc2Volume_orig;
  osc3Volume = osc3Muted ? 0.0 : osc3Volume_orig;
  noiseOscVolume = oscNoiseMuted ? 0.0 : noiseOscVolume_orig;
}

// changes the oscillator mode so that different instruments play each time the
// randomize button is pressed
void oscModeUpdater() {
  switch (oscMode) {

  case 1: // Mode 1: all 4 on
    osc1Muted = false;
    osc2Muted = false;
    osc3Muted = false;
    oscNoiseMuted = false;
    break;

  case 2: // Mode 2: bass + lead
    osc1Muted = false;
    osc2Muted = true;
    osc3Muted = false;
    oscNoiseMuted = true;
    break;

  case 3: // Mode 3: lead only
    osc1Muted = true;
    osc2Muted = true;
    osc3Muted = false;
    oscNoiseMuted = true;
    break;

  case 4: // Mode 4: synth (mid) + bass
    osc1Muted = false;
    osc2Muted = false;
    osc3Muted = true;
    oscNoiseMuted = true;
    break;

  case 5: // Mode 5: just middle synth
    osc1Muted = true;
    osc2Muted = false;
    osc3Muted = true;
    oscNoiseMuted = true;
    break;

  case 6: // Mode 6: just white noise
    osc1Muted = true;
    osc2Muted = true;
    osc3Muted = true;
    oscNoiseMuted = false;
    break;
  }
}

// randomizes the oscillator envelopes each time this is called
void randomizeEnvelopes() {
  // --- osc1 — bass ---
  env1.setADLevels(255,            // attack level stays max
                   random(40, 150) // random decay level
  );
  env1.setTimes(random(3, 40),  // attack time
                random(10, 80), // decay time
                random(10, 60), // sustain time
                random(5, 40)   // release time
  );

  // --- osc2 — mid synth ---
  env2.setADLevels(255, random(60, 180));
  env2.setTimes(random(10, 60), random(20, 90), random(10, 80), random(10, 60));

  // --- osc3 — lead (improved: sometimes long attack, sometimes long sustain)
  int behavior = random(0, 100); // decide envelope shape

  int attackTime, decayTime, sustainTime, releaseTime;
  int decayLevel = random(20, 100); // slightly softer for expressiveness

  if (behavior < 40) {
    // --- SHORT attack, SHORT sustain (plucky lead) ---
    attackTime = random(1, 12);
    decayTime = random(5, 25);
    sustainTime = random(5, 25);
    releaseTime = random(5, 25);
  } else if (behavior < 70) {
    // --- LONG attack, SHORT sustain (pad / swell lead) ---
    attackTime = random(20, 80);
    decayTime = random(10, 40);
    sustainTime = random(5, 30);
    releaseTime = random(10, 40);
  } else {
    // --- SHORT attack, LONG sustain (held melodic lead) ---
    attackTime = random(1, 15);
    decayTime = random(5, 30);
    sustainTime = random(40, 120); // extended melodic sustain
    releaseTime = random(20, 60);
  }

  // Store globally for dynamic updates
  env3Atk = attackTime;
  env3Dec = decayTime;
  env3Sus = sustainTime;
  env3Rel = releaseTime;

  env3.setADLevels(255, decayLevel);

  env3.setTimes(env3Atk, env3Dec, env3Sus, env3Rel);
}

// Adjusts envelopes and LFSR for different drum sounds
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

// assigns drum pattern based on randomly generated number
void updateSequencer(uint8_t randDrumSequence) {
  if (randDrumSequence > 16)
    randDrumSequence = 0; // fallback to 0
  activeSequence = drumSequences[randDrumSequence];
  sequenceLength = drumSequenceLengths[randDrumSequence];
  drumStep = 0;
}

// triggers next drum sound
void triggerNextDrum() {
  playDrum(activeSequence[drumStep]);
  drumStep++;
  if (drumStep >= sequenceLength)
    drumStep = 0;
}

void loop() {
  audioHook(); // handles Mozzi audio generation behind the scenes
}

// Called automatically at rate specified by CONTROL_RATE macro, most of your
// code should live in here
void updateControl() {
  meap.readInputs();
  updateOscillatorVolumes(); // Update volumes & envelopes continuously

  // --- Tempo / BPM ---
  // Simple linear mapping: 0-4095 -> 40-180 BPM
  float potScaleTempo = meap.pot_vals[0] / 4095.0; // normalize 0–1
  bpm = 40.0 + (potScaleTempo * 140.0); // min is 40 BPM, max is 180 BPM
  bpm = constrain(bpm, 40.0f, 180.0f);

  sixteenth_length = (60.0 / bpm) / (halfTimeToggle ? 2.0f : 4.0f);

  // --- Global transpose ---
  float potScaleTranspose = meap.pot_vals[1] / 4095.0; // 0–1
  transposer = randTranspose + (int)((potScaleTranspose - 0.5f) * 2.0f * 12);
  transposer = constrain(transposer, -12, 12);

  // --- Handle mode change ---
  static bool lastMode = useMinorMode;
  if (lastMode != useMinorMode) {
    lastMode = useMinorMode;
    chordNotes = useMinorMode ? chordNotes_minor : chordNotes_major;
    nextChord = useMinorMode ? nextChord_minor : nextChord_major;
    chordProbabilities =
        useMinorMode ? chordProbabilities_minor : chordProbabilities_major;
  }

  // --- Metronome tick ---
  static int bassCounter = 0;
  if (metro.ready()) {
    metro.start(sixteenth_length * 1000);

    // --- Bass ---
    bassCounter++;
    if (bassCounter % 2 == 0) { // half-speed trigger
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
      bassNote -= 24; // down 2 octaves
      playRoot = !playRoot;
      osc1.setFreq(mtof(bassNote + transposer));
      env1.noteOn();

      // MIDI OUT - BASS (Channel 1)
      if (lastBassMidi != -1)
        MIDI.sendNoteOn(lastBassMidi, 0, 1); // Note Off
      if (!osc1Muted) {
        lastBassMidi = bassNote + transposer;
        MIDI.sendNoteOn(lastBassMidi, 127, 1);
        // Serial.print("MIDI Ch1 (Bass): ");
        // Serial.println(lastBassMidi);
      } else {
        lastBassMidi = -1;
      }
    }

    // --- Arpeggio / Banjo ---
    int arpNote;
    switch (arpMode) {
    case 1:
      arpNote = getBanjoNote(currentChord);
      break;
    case 2:
      arpNote = getArpNoteUp(currentChord);
      break;
    case 3:
      arpNote = getArpNoteDown(currentChord);
      break;
    case 4:
      arpNote = getArpNotePingPong(currentChord);
      break;
    case 5:
      arpNote = getArpNoteReversePingPong(currentChord);
      break;
    default:
      arpNote = getBanjoNote(currentChord);
      break;
    }
    arpNote += 12; // up 1 octave
    osc2.setFreq(mtof(arpNote + transposer));
    env2.noteOn();

    // MIDI OUT - ARP (Channel 2)
    if (lastArpMidi != -1)
      MIDI.sendNoteOn(lastArpMidi, 0, 2); // Note Off
    if (!osc2Muted) {
      lastArpMidi = arpNote + transposer;
      MIDI.sendNoteOn(lastArpMidi, 127, 2);
      // Serial.print("MIDI Ch2 (Arp): ");
      // Serial.println(lastArpMidi);
    } else {
      lastArpMidi = -1;
    }

    // --- Melody ---
    if (melodyWaitSteps <= 0) {
      // 1. Determine new duration from Musical Rhythm Phrase
      if (phraseStep < phraseLength) {
        melodyWaitSteps = rhythmPhrase[phraseStep];
      } else {
        melodyWaitSteps = 2; // Fallback
      }

      // Calculate dynamic sustain based on note length
      // Total duration in ms = steps * (seconds per 16th) * 1000
      int totalDurationMs = melodyWaitSteps * sixteenth_length * 1000;
      int dynamicSustain =
          totalDurationMs - env3Atk - env3Dec - 20; // 20ms buffer
      if (dynamicSustain < 10)
        dynamicSustain = 10; // Minimum sustain

      melodyWaitSteps--; // count this step immediately

      int melodyNote = getMelodyNote(currentChord);
      melodyNote += 12;
      osc3.setFreq(mtof(melodyNote + transposer));

      // Update envelope with new sustain time, keeping random
      // Attack/Decay/Release
      env3.setTimes(env3Atk, env3Dec, dynamicSustain, env3Rel);
      env3.noteOn();

      // MIDI OUT - MELODY (Channel 3)
      if (lastMelodyMidi != -1)
        MIDI.sendNoteOn(lastMelodyMidi, 0, 3); // Note Off
      if (!osc3Muted && !silentPhrase) {
        lastMelodyMidi = melodyNote + transposer;
        MIDI.sendNoteOn(lastMelodyMidi, 127, 3);
        // Serial.print("MIDI Ch3 (Melody): ");
        // Serial.println(lastMelodyMidi);
      } else {
        lastMelodyMidi = -1;
      }
    } else {
      melodyWaitSteps--;
    }

    // --- Drums ---
    triggerNextDrum();

    beatCounter++;
    if (beatCounter % beatsPerChord == 0) {
      currentChord = nextTargetChord;
      nextTargetChord = getNextChord();

      const char **chordLetters =
          useMinorMode ? chordLetters_minor : chordLetters_major;
      Serial.print("New chord: ");
      Serial.println(chordLetters[currentChord]);
      // Serial.print("Next Target: ");
      // Serial.println(chordLetters[nextTargetChord]);

      if (chordMode == 0) {
        // Calculate index of the chord that just started playing
        // (progInd is already 2 steps ahead due to getNextChord calls)
        int playingIndex = (progressionIndex - 2 + 8) % 8;
        printProgression(currentProgression, playingIndex);
      }
    }
  }
}

// Called automatically at rate specified by AUDIO_RATE macro, for calculating
// samples sent to DAC, too much code in here can disrupt your output
AudioOutput_t updateAudio() {
  // --- Update envelopes ---
  env1.update();
  env2.update();
  env3.update();
  noiseEnv.update();

  // --- Generate oscillator samples ---
  int16_t osc1Sample = osc1.next() * osc1Volume * env1.next();
  int16_t osc2Sample = osc2.next() * osc2Volume * env2.next();
  int16_t osc3Sample = (!osc3Muted && !silentPhrase)
                           ? osc3.next() * osc3Volume * env3.next()
                           : 0;
  // --- Generate noise sample ---
  int16_t noiseSample = (gameboyNoise() * noiseEnv.next());

  int32_t mixed =
      osc1Sample + osc2Sample + osc3Sample + (noiseSample * noiseOscVolume);

  return StereoOutput::fromNBit(16, (mixed * meap.volume_val) >> 12,
                                (mixed * meap.volume_val) >> 12);
}

// Runs whenever a touch pad is pressed or released
void updateTouch(int number, bool pressed) {
  if (pressed) { // Any pad pressed
  } else {       // Any pad released
  }
  switch (number) {
  case 0:
    if (pressed) { // Pad 0 pressed
      Serial.println("t0 pressed ");
    } else { // Pad 0 released
      Serial.println("t0 released");
    }
    break;
  case 1:
    if (pressed) { // Pad 1 pressed
      Serial.println("t1 pressed");
    } else { // Pad 1 released
      Serial.println("t1 released");
    }
    break;
  case 2:
    if (pressed) { // Pad 2 pressed
      Serial.println("t2 pressed");
    } else { // Pad 2 released
      Serial.println("t2 released");
    }
    break;
  case 3:
    if (pressed) { // Pad 3 pressed
      Serial.println("t3 pressed");
    } else { // Pad 3 released
      Serial.println("t3 released");
    }
    break;
  case 4:
    if (pressed) { // Pad 4 pressed
      Serial.println("t4 pressed");
    } else { // Pad 4 released
      Serial.println("t4 released");
    }
    break;
  case 5:
    if (pressed) { // Pad 5 pressed
      Serial.println("t5 pressed");
    } else { // Pad 5 released
      Serial.println("t5 released");
    }
    break;
  case 6:
    if (pressed) { // Pad 6 pressed
      Serial.println("t6 pressed");
    } else { // Pad 6 released
      Serial.println("t6 released");
    }
    break;
  case 7:
    if (pressed) { // Pad 7 pressed
      Serial.println("t7 pressed");
    } else { // Pad 7 released
      Serial.println("t7 released");
    }
    break;
  }
}

// Runs whenever a DIP switch is toggled
void updateDip(int number, bool up) {
  if (up) { // Any DIP toggled up
  } else {  // Any DIP toggled down
  }
  switch (number) {
  case 0:
    if (up) { // DIP 0 up
      randTempo =
          40.0 + ((float)rand() / RAND_MAX) * 140.0; // random 40–180 BPM
      randTranspose = -12 + (int)(((float)rand() / RAND_MAX) * 25); // -12 to 12
      useMinorMode =
          (rand() % 2 == 0);      // Randomly set useMinorMode to true or false
      oscMode = 1 + rand() % 6;   // generates 1–6
      arpMode = 1 + rand() % 5;   // generates 1-5
      bassMode = 1 + rand() % 3;  // generates 1-3
      drumMode = 1 + rand() % 16; // generates 1-16
      randProg = rand() % 50;     // generates 0-49
      newProg = true;
      updateSequencer(drumMode);
      // updates the mode
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
        Serial.println("Bass Mode 1: Bass Note (Root note of the chord without "
                       "any octave changes)");
        break;

      case 2:
        Serial.println("Bass Mode 2: Bass Note Octaves (Root note of the chord "
                       "with octave changes)");
        break;

      case 3:
        Serial.println("Bass Mode 3: Bass Note Root/Fifth (Bass note of the "
                       "chord with alternating 5th changes)");
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

      // Print current drum sequence
      Serial.print("Drum Mode: ");
      Serial.println(drumMode);
      Serial.print("Active Sequence: ");
      for (int i = 0; i < sequenceLength; i++) {
        Serial.print(drumTypeNames[activeSequence[i]]);
        Serial.print(" ");
      }
      Serial.println();

      switch (oscMode) {
      case 1:
        Serial.println("ALL OSCILLATORS ON (BASS + MID + LEAD + NOISE)");
        break;

      case 2:
        Serial.println("BASS + LEAD ONLY (NO MID, NO NOISE)");
        break;

      case 3:
        Serial.println("LEAD ONLY");
        break;

      case 4:
        Serial.println("BASS + MID SYNTH ONLY");
        break;

      case 5:
        Serial.println("MID SYNTH ONLY");
        break;

      case 6:
        Serial.println("WHITE NOISE ONLY (DRUM MACHINE)");
        break;

      default:
        Serial.println("UNKNOWN MODE");
        break;
      }

      Serial.println("============================");

    } else { // DIP 0
    }
    break;
  case 1:
    if (up) { // DIP 1 up
      useMinorMode = true;
      Serial.println("Mode switched to Minor");
    } else { // DIP 1 down
      useMinorMode = false;
      Serial.println("Mode switched to Major");
    }
    break;
  case 2:
    if (up) { // DIP 2 up
      chordMode = 0;
      Serial.println("Chord Mode: Predefined");
    } else { // DIP 2 down
      chordMode = 1;
      Serial.println("Chord Mode: Procedual");
    }
    break;
  case 3:
    if (up) { // DIP 3 up
      halfTimeToggle = true;
      Serial.println("toggled Half Time");
    } else { // DIP 3 down
      halfTimeToggle = false;
      Serial.println("toggled Normal Time");
    }
    break;
  case 4: // Bass mute toggle
    if (up) {
      osc1Muted = true;
      Serial.println("Bass muted (d4 up)");
    } else {
      osc1Muted = false;
      Serial.println("Bass unmuted (d4 down)");
    }
    break;

  case 5: // Banjo/Mid mute toggle
    if (up) {
      osc2Muted = true;
      Serial.println("Middle Osc muted (d5 up)");
    } else {
      osc2Muted = false;
      Serial.println("Middle Osc unmuted (d5 down)");
    }
    break;

  case 6: // Melody mute toggle
    if (up) {
      osc3Muted = true;
      Serial.println("Melody muted (d6 up)");
    } else {
      osc3Muted = false;
      Serial.println("Melody unmuted (d6 down)");
    }
    break;

  case 7: // Noise mute toggle
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