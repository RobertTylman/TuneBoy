# TuneBoy

**TuneBoy** is a generative music sequencer and synthesizer designed for the Meap board (ESP32). It combines procedural melody generation, smart chord progressions, and a rhythmic drum sequencer to create evolving 4-voice polyphonic music in real-time.

## Features

-   **4-Voice Polyphony**:
    -   **Bass**: Anchors the harmony with root notes or alternating root-fifths.
    -   **Mid Synth**: Provides harmonic texture.
    -   **Lead Melody**: Procedurally generating using call-and-response phrasing.
    -   **Noise (Drums)**: Features a Gameboy-style LFSR noise generator for percussion.

-   **Smart Key & Chord System**:
    -   Supports both **Major** and **Minor** modes (G Major / G Minor).
    -   Implements complex chord transitions with weighted probabilities for musical coherence.
    -   Includes borrowed chords and modal mixtures for harmonic interest.

-   **Procedural Generation**:
    -   **Melodies**: NOT random notes, but structured phrases that evolve over time.
    -   **Arpeggios**: Multiple patterns (Up, Down, Ping-Pong, Random).
    -   **Drums**: Pattern-based sequencer with Euclidean-style rhythms.

-   **Real-time Controls**:
    -   **Tempo**: Adjustable from 40 to 180 BPM via potentiometer.
    -   **Instrument Modes**: Toggle between full band, minimal synth, or solo modes.
    -   **Performance Features**: Mute individual tracks, change chord progressions on the fly.

## The Game Boy Sound Chip (DMG-01) Emulation

TuneBoy is designed to emulate the constraints and aesthetic of the original Nintendo Game Boy (DMG-01) sound hardware (the SHARP LR35902). The original hardware famously featured 4 monophonic channels, which TuneBoy replicates:

1.  **Channel 1 (Pulse/Square)**: Traditionally used for melody. In TuneBoy, this is the **Lead Melody** voice.
2.  **Channel 2 (Pulse/Square)**: Traditionally used for harmony. In TuneBoy, this is the **Mid Synth**.
3.  **Channel 3 (Wave)**: A variable wavetable channel often used for bass. In TuneBoy, this is the **Bass** voice.
4.  **Channel 4 (Noise)**: A pseudo-random noise generator used for percussion and effects. In TuneBoy, this is the **Drum** channel.

By strictly adhering to this 4-voice limit, TuneBoy recreates the "chip tune" sound where voices must often drop out to make room for others, forcing creative economy in arrangement.

## Under the Hood: LFSR & Noise

To authentically recreate the Game Boy's percussion, TuneBoy uses a **Linear Feedback Shift Register (LFSR)** instead of playing back recorded drum samples. 

In digital logic, an LFSR generates pseudo-random numbers by taking a binary value, shifting it, and using the "exclusive OR" (XOR) of specific bits to create a feedback bit. 
-   **Long Mode (15-bit)**: Produces a "white noise" hiss, perfect for **Snares** and **Hi-Hats**.
-   **Short Mode (7-bit)**: Creates a metallic, repeating buzzing loop, used for **Cymbals** or **Robotic Bass** tones.

This technique is mathematically identical to how the Game Boy (and NES) hardware generated sound effects.

## Hardware Requirements

-   **Meap Board**: An ESP32-based creative coding platform.
-   **Audio Output**: Speaker or Headphones connected to the Meap board.
-   **Controls**: Potentiometers and buttons (standard on Meap) for user interaction.

## Software Dependencies

This project relies on the following libraries:

-   **[Meap Library](https://github.com/masonandrewmann/MEAP)**: Handles input/output and integrates with the Mozzi audio engine. [(Documentation)](https://masonmann.online/electronics/meap/software/)
-   **[Mozzi](https://github.com/sensorium/Mozzi)**: A sound synthesis library for Arduino/ESP32 (bundled with Meap).

## Installation

1.  **Install Arduino IDE**: Ensure you have the latest version of the Arduino IDE installed.
2.  **Install Meap Library**: 
    -   Download the [Meap Library](https://github.com/masonandrewmann/MEAP) as a `.zip` file from GitHub.
    -   In Arduino IDE, go to **Sketch -> Include Library -> Add .ZIP Library...** and select the downloaded file.
    -   *Note: The Meap library bundles the Mozzi synthesis engine, so no separate install is needed.*
3.  **Open Project**: Open `Assignment_5.ino` in the Arduino IDE.
4.  **Select Board**: 
    -   Go to **Tools -> Board -> ESP32 Arduino**.
    -   Select **ESP32 Dev Module** (or your specific compatible board).
5.  **Upload**: Connect your Meap board via USB and click the Upload button.

## Usage

Once uploaded, the TuneBoy will immediately start generating music.

-   **Tempo Control**: Turn **Potentiometer 0** (leftmost) to adjust the speed between 40 BPM (slow) and 180 BPM (fast).
-   **Mode Switching**: Use the on-board buttons (layout may vary by specific Meap version) to cycle through instrument combinations (e.g., Bass+Drums, Full Band, Solo).
-   **Mute**: Functionality is mapped to specific inputs to drop voices in and out for live arrangement.

## File Structure

-   `Assignment_5.ino`: The main application code containing the synthesizer engine, sequencer logic, and procedural generation algorithms.
-   `ChordData.h`: A large database of chord definitions, scales, and transition probabilities for music theory logic.
