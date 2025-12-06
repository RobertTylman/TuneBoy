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

## Hardware Requirements

-   **Meap Board**: An ESP32-based creative coding platform.
-   **Audio Output**: Speaker or Headphones connected to the Meap board.
-   **Controls**: Potentiometers and buttons (standard on Meap) for user interaction.

## Software Dependencies

This project relies on the following libraries:

-   **[Meap Library](https://github.com/mrbid/Meap)**: Handles input/output and integrates with the Mozzi audio engine.
-   **Mozzi**: A sound synthesis library for Arduino/ESP32 (bundled with Meap).

## Installation

1.  **Install Arduino IDE**: Ensure you have the latest version of the Arduino IDE installed.
2.  **Install Meap Library**: Follow the installation instructions for the Meap library.
3.  **Open Project**: Open `Assignment_5.ino` in the Arduino IDE.
4.  **Select Board**: Choose "ESP32 Dev Module" (or your specific Meap configuration).
5.  **Upload**: Connect your Meap board via USB and click the Upload button.

## Usage

Once uploaded, the TuneBoy will immediately start generating music.

-   **Tempo Control**: Turn **Potentiometer 0** (leftmost) to adjust the speed between 40 BPM (slow) and 180 BPM (fast).
-   **Mode Switching**: Use the on-board buttons (layout may vary by specific Meap version) to cycle through instrument combinations (e.g., Bass+Drums, Full Band, Solo).
-   **Mute**: Functionality is mapped to specific inputs to drop voices in and out for live arrangement.

## File Structure

-   `Assignment_5.ino`: The main application code containing the synthesizer engine, sequencer logic, and procedural generation algorithms.
-   `ChordData.h`: A large database of chord definitions, scales, and transition probabilities for music theory logic.
