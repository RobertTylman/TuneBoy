// =====================================================
// 🎵 CHORD AND SCALE DEFINITIONS
// =====================================================

// =====================================================
// 🎸 MAJOR MODE (G Major)
// -----------------------------------------------------
// chord indices:
//   0=G, 1=C, 2=D, 3=D7, 4=A7, 5=B7, 6=Em, 7=Am, 8=Bm
// =====================================================

// --- Chord letters for major mode (G major) ---
const char *chordLetters_major[] = {"G",  "C",   "D",   "D7",    "A7", "B7",
                                    "Em", "Am",  "Bm",  "Cmaj7", "A",  "F#7",
                                    "F",  "Bm7", "Em7", "Am7"};

// --- Chord letters for minor mode (G minor) ---
const char *chordLetters_minor[] = {"Gm",  "Cm",  "Dm",  "D7",  "A7",
                                    "B7",  "Ebm", "Abm", "Bbm", "Fmaj7",
                                    "Eø7", "Bø7", "D°7", "Gm7"};

int chordNotes_major[][4] = {
    {55, 59, 62, -1}, // 0: G     (G  B  D)
    {60, 64, 67, -1}, // 1: C     (C  E  G)
    {62, 66, 69, -1}, // 2: D     (D  F# A)
    {62, 66, 69, 72}, // 3: D7    (D  F# A  C)
    {57, 61, 64, 67}, // 4: A7    (A  C# E  G)
    {59, 63, 66, 69}, // 5: B7    (B  D# F# A)
    {64, 67, 71, -1}, // 6: Em    (E  G  B)
    {57, 60, 64, -1}, // 7: Am    (A  C  E)
    {59, 62, 66, -1}, // 8: Bm    (B  D  F#)
    {60, 64, 67, 71}, // 9: Cmaj7 (C E G B)
    {57, 61, 64, -1}, // 10: A    (A C# E) — V/V
    {66, 70, 73, 77}, // 11: F#7  (F# A# C# E) — V/ii
    {53, 57, 60, 65}, // 12: F    (F A C) — ♭VII modal mix
    {59, 62, 66, 71}, // 13: Bm   (B D F# A) — iii7 flavor
    {64, 67, 71, 74}, // 14: Em7  (E G B D) — vi7
    {57, 60, 64, 67}  // 15: Am7  (A C E G) — ii7
};

// --- Chord transition options (Major key center: G) ---
int nextChord_major[][6] = {
    // From: G
    {1, 2, 6, 12, 14, -1}, // C, D, Em, F(♭VII), Em7
    // From: C
    {0, 2, 7, 9, 12, -1}, // G, D, Am, Cmaj7, F
    // From: D
    {0, 1, 3, 10, -1, -1}, // G, C, D7, A (V/V)
    // From: D7
    {0, 6, 14, -1, -1, -1}, // G, Em, Em7
    // A7 (V/V)
    {2, 10, 0, -1, -1, -1}, // D, A, G
    // B7 (V/vi)
    {6, 14, 0, -1, -1, -1}, // Em, Em7, G
    // Em
    {1, 2, 14, 12, 0, -1}, // C, D, Em7, F, G
    // Am
    {2, 0, 15, 11, -1, -1}, // D, G, Am7, F#7
    // Bm
    {6, 1, 13, 10, -1, -1}, // Em, C, Bm7, A
    // Cmaj7
    {0, 6, 12, -1, -1, -1}, // G, Em, F
    // A major
    {2, 0, -1, -1, -1, -1}, // D, G
    // F#7
    {7, 15, -1, -1, -1, -1}, // Am, Am7
    // F (♭VII)
    {0, 1, 6, -1, -1, -1}, // G, C, Em
    // Bm7
    {6, 14, 1, -1, -1, -1}, // Em, Em7, C
    // Em7
    {1, 2, 0, -1, -1, -1}, // C, D, G
    // Am7
    {2, 0, 6, 14, -1, -1} // D, G, Em, Em7
};

// --- Transition probabilities (Major mode) ---
int chordProbabilities_major[][6] = {
    {30, 60, 80, 90, 100, 0}, // G
    {40, 70, 85, 95, 100, 0}, // C
    {60, 80, 95, 100, 0, 0},  // D
    {70, 100, 0, 0, 0, 0},    // D7
    {60, 90, 100, 0, 0, 0},   // A7
    {50, 80, 100, 0, 0, 0},   // B7
    {40, 70, 85, 95, 100, 0}, // Em
    {40, 80, 95, 100, 0, 0},  // Am
    {40, 80, 95, 100, 0, 0},  // Bm
    {50, 80, 100, 0, 0, 0},   // Cmaj7
    {70, 100, 0, 0, 0, 0},    // A
    {80, 100, 0, 0, 0, 0},    // F#7
    {40, 80, 100, 0, 0, 0},   // F
    {50, 85, 100, 0, 0, 0},   // Bm7
    {40, 80, 100, 0, 0, 0},   // Em7
    {40, 70, 90, 100, 0, 0}   // Am7
};

// 🎵 MAJOR MODE (G Major / Mixolydian colors)
int chordScales_major[][8] = {
    {55, 57, 59, 60, 62, 64, 65, 67}, // G mixolydian: G A B C D E F# G
    {60, 62, 64, 65, 67, 69, 71, 72}, // C major:      C D E F G A B C
    {62, 64, 66, 67, 69, 71, 73, 74}, // D mixolydian: D E F# G A B C D
    {62, 64, 66, 67, 69, 71, 72, 74}, // D7:           D E F# G A B C D
    {57, 59, 61, 62, 64, 66, 67, 69}, // A7:           A B C# D E F# G A
    {59, 61, 63, 64, 66, 68, 69, 71}, // B7:           B C# D# E F# G# A B
    {64, 66, 67, 69, 71, 72, 74, 76}, // Em:           E F# G A B C D E
    {57, 59, 60, 62, 64, 65, 67, 69}, // Am:           A B C D E F G A
    {59, 61, 62, 64, 66, 67, 69, 71}, // Bm:           B C# D E F# G A B
    {60, 62, 64, 65, 67, 69, 71, 72}, // Cmaj7:        C D E F G A B C
    {57, 59, 61, 62, 64, 66, 67, 69}, // A:            A B C# D E F# G# A
    {66, 68, 70, 73, 75, 77, 78, 80}, // F#7:          F# A# C# E F# A# C# E
    {53, 55, 57, 60, 62, 64, 65, 67}, // F:            F A C D E G A C
    {59, 61, 62, 64, 66, 67, 69, 71}, // Bm7:          B D F# A B D F# A
    {64, 66, 67, 69, 71, 72, 74, 76}, // Em7:          E G B D E G B D
    {57, 59, 60, 62, 64, 65, 67, 69}  // Am7:          A C E G A C E G
};

int chordProgressions_major[][8] = {
    {1, 6, 2, 5, 1, 6, 2, 5}, // I-vi-ii-V-I-vi-ii-V
    {1, 3, 6, 2, 1, 3, 6, 2}, // I-iii-vi-ii-I-iii-vi-ii
    {6, 2, 5, 1, 6, 2, 5, 1}, // vi-ii-V-I-vi-ii-V-I
    {1, 6, 4, 5, 1, 6, 4, 5}, // I-vi-IV-V-I-vi-IV-V
    {6, 4, 1, 3, 6, 4, 1, 3}, // vi-IV-I-iii-vi-IV-I-iii
    {5, 1, 6, 2, 5, 1, 6, 2}, // V-I-vi-ii-V-I-vi-ii
    {3, 6, 2, 1, 3, 6, 2, 1}, // iii-vi-ii-I-iii-vi-ii-I
    {1, 5, 6, 2, 1, 5, 6, 2}, // I-V-vi-ii-I-V-vi-ii
    {6, 3, 1, 5, 6, 3, 1, 5}, // vi-iii-I-V-vi-iii-I-V
    {2, 6, 1, 3, 2, 6, 1, 3}, // ii-vi-I-iii-ii-vi-I-iii
    {1, 3, 5, 6, 1, 3, 5, 6}, // I-iii-V-vi-I-iii-V-vi
    {6, 2, 3, 1, 6, 2, 3, 1}, // vi-ii-iii-I-vi-ii-iii-I
    {3, 5, 1, 6, 3, 5, 1, 6}, // iii-V-I-vi-iii-V-I-vi
    {5, 6, 2, 1, 5, 6, 2, 1}, // V-vi-ii-I-V-vi-ii-I
    {1, 2, 3, 6, 1, 2, 3, 6}, // I-ii-iii-vi-I-ii-iii-vi
    {6, 1, 5, 3, 6, 1, 5, 3}, // vi-I-V-iii-vi-I-V-iii
    {3, 1, 6, 5, 3, 1, 6, 5}, // iii-I-vi-V-iii-I-vi-V
    {2, 1, 6, 3, 2, 1, 6, 3}, // ii-I-vi-iii-ii-I-vi-iii
    {1, 6, 3, 5, 1, 6, 3, 5}, // I-vi-iii-V-I-vi-iii-V
    {6, 5, 1, 2, 6, 5, 1, 2}, // vi-V-I-ii-vi-V-I-ii
    {3, 6, 5, 1, 3, 6, 5, 1}, // iii-vi-V-I-iii-vi-V-I
    {1, 2, 6, 5, 1, 2, 6, 5}, // I-ii-vi-V-I-ii-vi-V

    // --- 22 new progressions ---
    {1, 4, 2, 5, 6, 3, 1, 5}, // I-IV-ii-V-vi-iii-I-V
    {6, 2, 4, 1, 3, 5, 6, 2}, // vi-ii-IV-I-iii-V-vi-ii
    {1, 3, 5, 2, 4, 6, 1, 3}, // I-iii-V-ii-IV-vi-I-iii
    {2, 5, 1, 6, 3, 4, 2, 5}, // ii-V-I-vi-iii-IV-ii-V
    {1, 4, 6, 2, 5, 3, 1, 4}, // I-IV-vi-ii-V-iii-I-IV
    {3, 6, 2, 5, 1, 4, 3, 6}, // iii-vi-ii-V-I-IV-iii-vi
    {1, 5, 2, 6, 3, 4, 1, 5}, // I-V-ii-vi-iii-IV-I-V
    {6, 3, 1, 5, 2, 4, 6, 3}, // vi-iii-I-V-ii-IV-vi-iii
    {1, 2, 4, 5, 3, 6, 1, 2}, // I-ii-IV-V-iii-vi-I-ii
    {4, 1, 5, 2, 6, 3, 4, 1}, // IV-I-V-ii-vi-iii-IV-I
    {1, 3, 6, 2, 5, 4, 1, 3}, // I-iii-vi-ii-V-IV-I-iii
    {5, 2, 4, 1, 6, 3, 5, 2}, // V-ii-IV-I-vi-iii-V-ii
    {1, 4, 3, 6, 2, 5, 1, 4}, // I-IV-iii-vi-ii-V-I-IV
    {6, 2, 5, 1, 4, 3, 6, 2}, // vi-ii-V-I-IV-iii-vi-ii
    {1, 5, 6, 3, 2, 4, 1, 5}, // I-V-vi-iii-ii-IV-I-V
    {2, 6, 1, 3, 5, 4, 2, 6}, // ii-vi-I-iii-V-IV-ii-vi
    {1, 3, 2, 5, 4, 6, 1, 3}, // I-iii-ii-V-IV-vi-I-iii
    {4, 1, 6, 3, 2, 5, 4, 1}, // IV-I-vi-iii-ii-V-IV-I
    {1, 2, 5, 3, 6, 4, 1, 2}, // I-ii-V-iii-vi-IV-I-ii
    {3, 6, 1, 4, 2, 5, 3, 6}, // iii-vi-I-IV-ii-V-iii-vi
    {1, 4, 2, 6, 5, 3, 1, 4}, // I-IV-ii-vi-V-iii-I-IV
    {6, 3, 5, 1, 2, 4, 6, 3}, // vi-iii-V-I-ii-IV-vi-iii
    
    // --- 2-Chord Alternating Progressions ---
    {0, 3, 0, 3, 0, 3, 0, 3}, // I - IV
    {0, 4, 0, 4, 0, 4, 0, 4}, // I - V
    {0, 5, 0, 5, 0, 5, 0, 5}, // I - vi
    {1, 4, 1, 4, 1, 4, 1, 4}, // ii - V
    {2, 3, 2, 3, 2, 3, 2, 3}, // iii - IV
    {3, 4, 3, 4, 3, 4, 3, 4}  // IV - V
};

// =====================================================
// 🎻 MINOR MODE (G Minor — Parallel Minor of G Major)
// -----------------------------------------------------
// chord indices:
//   0=Gm, 1=Cm, 2=Dm, 3=D7, 4=A7, 5=B7, 6=Ebm, 7=Abm, 8=Bbm
// =====================================================

int chordNotes_minor[][4] = {
    {55, 58, 62, -1}, // 0: Gm   (i)
    {60, 63, 67, -1}, // 1: Cm   (iv)
    {62, 65, 69, -1}, // 2: Dm   (v)
    {62, 66, 69, 72}, // 3: D7   (V7)
    {57, 61, 64, 67}, // 4: A7   (V7/V)
    {59, 63, 66, 69}, // 5: B7   (V7/III)
    {63, 66, 70, -1}, // 6: Ebm  (bIII borrowed)
    {56, 59, 63, -1}, // 7: Abm  (bVI borrowed)
    {58, 61, 65, -1}, // 8: Bbm  (bVII modal mixture)
    {53, 57, 60, 65}, // 9: Fmaj7 (bVIImaj7 → i)
    {52, 55, 59, 62}, // 10: Eø7 (iiø / iiø–V7–i)
    {59, 62, 65, 69}, // 11: Bø7 (iiø/V leading into A7)
    {50, 53, 57, 60}, // 12: D°7 (leading tone diminished)
    {55, 59, 62, 65}  // 13: Gm7 (i7 color)
};

// --- Chord transition options (Minor key center: Gm) ---
int nextChord_minor[][6] = {
    {1, 2, 3, 8, 9, 13},    // 0: Gm → iv, v, V7, bVII, bVIImaj7, i7
    {0, 2, 5, 8, 9, -1},    // 1: Cm → i, v, B7, bVII, Fmaj7
    {0, 3, 1, 9, 13, -1},   // 2: Dm → i, V7, iv, Fmaj7, i7
    {0, 1, 2, 10, 13, -1},  // 3: D7 → i, iv, v, iiø7, i7
    {0, 3, 1, 11, -1, -1},  // 4: A7 → i, V7, iv, iiø/V
    {2, 4, 0, -1, -1, -1},  // 5: B7 → v → V7 → i
    {0, 1, 7, 8, -1, -1},   // 6: Ebm → i, iv, bVI, bVII
    {0, 1, 8, 6, -1, -1},   // 7: Abm → i, iv, bVII, bIII
    {0, 3, 9, 13, -1, -1},  // 8: Bbm → i, V7, Fmaj7, i7
    {0, 3, 13, 1, -1, -1},  // 9: Fmaj7 → i, V7, i7, iv
    {3, 0, 13, -1, -1, -1}, // 10: Eø7 → V7 → i
    {4, 3, 0, -1, -1, -1},  // 11: Bø7 → A7 → V7 → i
    {0, 3, -1, -1, -1, -1}, // 12: D°7 → i, V7
    {0, 3, 1, 2, 9, 8}      // 13: Gm7 → i, V7, iv, v, bVIImaj7, bVII
};

// --- Transition probabilities (Minor mode) ---
int chordProbabilities_minor[][6] = {
    {20, 45, 70, 85, 95, 100}, // Gm
    {40, 70, 85, 95, 100, 0},  // Cm
    {55, 80, 90, 100, 0, 0},   // Dm
    {35, 65, 80, 95, 100, 0},  // D7
    {50, 75, 95, 100, 0, 0},   // A7
    {40, 70, 100, 0, 0, 0},    // B7
    {35, 60, 85, 100, 0, 0},   // Ebm
    {40, 70, 100, 0, 0, 0},    // Abm
    {50, 75, 90, 100, 0, 0},   // Bbm
    {40, 70, 90, 100, 0, 0},   // Fmaj7
    {55, 100, 0, 0, 0, 0},     // Eø7 → V
    {60, 100, 0, 0, 0, 0},     // Bø7 → A7
    {50, 100, 0, 0, 0, 0},     // D°7 → i
    {40, 70, 85, 95, 100, 0}   // Gm7
};

int chordScales_minor[][8] = {
    {55, 57, 58, 60, 62, 63, 65, -1}, // 0: Gm scale — G A Bb C D Eb F
    {60, 62, 63, 65, 67, 68, 70, -1}, // 1: Cm — C D Eb F G Ab Bb
    {62, 64, 65, 67, 69, 70, 72, -1}, // 2: Dm — D E F G A Bb C
    {62, 64, 66, 67, 69, 71, 72, -1}, // 3: D7 — D E F# G A C C#
    {57, 59, 61, 62, 64, 65, 67, -1}, // 4: A7 — A B C# D E F G
    {59, 61, 63, 65, 66, 68, 69, -1}, // 5: B7 — B C# D# E F# G# A
    {63, 65, 66, 68, 70, 71, 73, -1}, // 6: Ebm — Eb F Gb Ab Bb Cb Db
    {56, 58, 59, 61, 63, 64, 66, -1}, // 7: Abm — Ab Bb Cb Db Eb Fb Gb
    {58, 60, 61, 63, 65, 66, 68, -1}, // 8: Bbm — Bb C Db Eb F Gb Ab
    {53, 55, 57, 60, 62, 64, 65, 67}, // 9: Fmaj7 — F G A C D E F G
    {52, 55, 58, 59, 62, 64, 67, -1}, // 10: Eø7 — E G Bb B D F A
    {59, 61, 62, 64, 66, 68, 69, -1}, // 11: Bø7 — B D F G A C E
    {50, 53, 56, 59, 62, 65, 68, -1}, // 12: D°7 — D F Ab B C# E G
    {55, 57, 58, 60, 62, 65, 69, -1}  // 13: Gm7 — G A Bb C D F A
};

int chordProgressions_minor[][8] = {
    {0, 3, 0, 13, 0, 3, 0, 13},   // i - v7 - i - i7 - i - v7 - i - i7
    {0, 1, 3, 0, 0, 1, 3, 0},     // i - iv - v7 - i - i - iv - v7 - i
    {0, 4, 3, 0, 0, 4, 3, 0},     // i - VI7 - v7 - i - i - VI7 - v7 - i
    {1, 0, 3, 2, 1, 0, 3, 2},     // iv - i - v7 - ii - iv - i - v7 - ii
    {0, 2, 3, 13, 0, 2, 3, 13},   // i - ii - v7 - i7 - i - ii - v7 - i7
    {3, 0, 4, 0, 3, 0, 4, 0},     // v7 - i - VI7 - i - v7 - i - VI7 - i
    {0, 3, 4, 0, 0, 3, 4, 0},     // i - v7 - VI7 - i - i - v7 - VI7 - i
    {0, 9, 3, 0, 0, 9, 3, 0},     // i - ♭VImaj7 - v7 - i - i - ♭VImaj7 - v7 - i
    {1, 4, 3, 0, 1, 4, 3, 0},     // iv - VI7 - v7 - i - iv - VI7 - v7 - i
    {0, 10, 3, 0, 0, 10, 3, 0},   // i - øii7 - v7 - i - i - øii7 - v7 - i
    {0, 11, 3, 0, 0, 11, 3, 0},   // i - øiii7 - v7 - i - i - øiii7 - v7 - i
    {12, 3, 0, 13, 12, 3, 0, 13}, // vii°7 - v7 - i - i7 - vii°7 - v7 - i - i7
    {4, 3, 0, 2, 4, 3, 0, 2},     // VI7 - v7 - i - ii - VI7 - v7 - i - ii
    {0, 1, 2, 13, 0, 1, 2, 13},   // i - iv - ii - i7 - i - iv - ii - i7
    {2, 3, 0, 1, 2, 3, 0, 1},     // ii - v7 - i - iv - ii - v7 - i - iv
    {1, 12, 3, 0, 1, 12, 3, 0},   // iv - vii°7 - v7 - i - iv - vii°7 - v7 - i
    {0, 2, 1, 3, 0, 2, 1, 3},     // i - ii - iv - v7 - i - ii - iv - v7
    {13, 3, 4, 0, 13, 3, 4, 0},   // i7 - v7 - VI7 - i - i7 - v7 - VI7 - i
    {0, 4, 1, 3, 0, 4, 1, 3},     // i - VI7 - iv - v7 - i - VI7 - iv - v7
    {0, 3, 12, 0, 0, 3, 12, 0},   // i - v7 - i7 - i - i - v7 - i7 - i
    {1, 0, 13, 3, 1, 0, 13, 3},   // iv - i - i7 - v7 - iv - i - i7 - v7
    {0, 13, 3, 0, 0, 13, 3, 0},   // i - i7 - v7 - i - i - i7 - v7 - i

    // --- 22 new minor progressions ---
    {0, 1, 2, 3, 4, 5, 6, 0}, // i - iv - ii - v7 - VI7 - VII7 - vi - i
    {0, 4, 1, 2, 5, 3, 0, 4}, // i - VI7 - iv - ii - VII7 - v7 - i - VI7
    {0, 2, 5, 3, 1, 4, 0, 2}, // i - ii - VII7 - v7 - iv - VI7 - i - ii
    {0, 3, 1, 4, 2, 5, 0, 3}, // i - v7 - iv - VI7 - ii - VII7 - i - v7
    {0, 5, 3, 1, 4, 2, 0, 5}, // i - VII7 - v7 - iv - VI7 - ii - i - VII7
    {0, 2, 4, 1, 5, 3, 0, 2}, // i - ii - VI7 - iv - VII7 - v7 - i - ii
    {0, 3, 5, 1, 2, 4, 0, 3}, // i - v7 - VII7 - iv - ii - VI7 - i - v7
    {0, 1, 4, 2, 3, 5, 0, 1}, // i - iv - VI7 - ii - v7 - VII7 - i - iv
    {0, 4, 2, 5, 1, 3, 0, 4}, // i - VI7 - ii - VII7 - iv - v7 - i - VI7
    {0, 3, 1, 5, 2, 4, 0, 3}, // i - v7 - iv - VII7 - ii - VI7 - i - v7
    {0, 5, 2, 3, 1, 4, 0, 5}, // i - VII7 - ii - v7 - iv - VI7 - i - VII7
    {0, 2, 3, 1, 4, 5, 0, 2}, // i - ii - v7 - iv - VI7 - VII7 - i - ii
    {0, 3, 4, 2, 1, 5, 0, 3}, // i - v7 - VI7 - iv - iv - VII7 - i - v7
    {0, 1, 5, 2, 3, 4, 0, 1}, // i - iv - VII7 - ii - v7 - VI7 - i - iv
    {0, 4, 3, 5, 1, 2, 0, 4}, // i - VI7 - v7 - VII7 - iv - ii - i - VI7
    {0, 5, 4, 1, 2, 3, 0, 5}, // i - VII7 - v7 - iv - ii - v7 - i - VII7
    {0, 3, 2, 4, 1, 5, 0, 3}, // i - v7 - ii - VI7 - iv - VII7 - i - v7
    {0, 1, 3, 2, 4, 5, 0, 1}, // i - iv - v7 - ii - VI7 - VII7 - i - iv
    {0, 4, 2, 5, 1, 3, 0, 4}, // i - VI7 - ii - VII7 - iv - v7 - i - VI7
    {0, 2, 5, 3, 1, 4, 0, 2}, // i - ii - VII7 - v7 - iv - VI7 - i - ii
    {0, 3, 1, 5, 2, 4, 0, 3}, // i - v7 - iv - VII7 - ii - VI7 - i - v7
    {0, 5, 3, 2, 1, 4, 0, 5}, // i - VII7 - v7 - ii - iv - VI7 - i - VII7

    // --- 2-Chord Alternating Progressions ---
    {0, 1, 0, 1, 0, 1, 0, 1}, // i - iv
    {0, 3, 0, 3, 0, 3, 0, 3}, // i - V7
    {0, 2, 0, 2, 0, 2, 0, 2}, // i - v
    {1, 3, 1, 3, 1, 3, 1, 3}, // iv - V7
    {0, 9, 0, 9, 0, 9, 0, 9}, // i - bVIImaj7
    {3, 0, 3, 0, 3, 0, 3, 0}  // V7 - i
};

const int numBanjoPatterns = 8; // total patterns in banjoPatterns

const int banjoPatterns[][6] = {
    {0, 2, 1, 2, 0, 2}, // forward roll (4-note extended)
    {2, 1, 0, 1, 2, 1}, // backward roll (4-note extended)
    {0, 1, 2, 1, 0, 2}, // mixed roll (4-note extended)
    {0, 2, 3, 2, 0, 3}, // octave roll (4-note extended)
    {0, 1, 2, 3, 2, 0}, // forward extended (5-note)
    {2, 1, 0, 1, 2, 0}, // backward extended (5-note)
    {0, 1, 2, 1, 3, 2}, // mixed extended (6-note)
    {0, 3, 1, 2, 3, 0}  // jumping/octave pattern (6-note)
};

// ---------- Drum pattern ----------
DrumType drumSequence[] = {KICK, HIHAT, SNARE, HIHAT};
DrumType drumSequence1[] = {KICK, HIHAT, SNARE, HIHAT};
DrumType drumSequence2[] = {KICK, SNARE, HIHAT};
DrumType drumSequence3[] = {KICK, HIHAT, KICK, HIHAT, SNARE, HIHAT};
DrumType drumSequence4[] = {KICK, CLAP};
DrumType drumSequence5[] = {KICK, HIHAT, SNARE, HIHAT, KICK};
DrumType drumSequence6[] = {KICK, HIHAT, HIHAT, SNARE};
DrumType drumSequence7[] = {KICK, HIHAT, SNARE, HIHAT, CLAP, HIHAT, TOM};
DrumType drumSequence8[] = {KICK, RIM, HIHAT};
DrumType drumSequence9[] = {KICK, HIHAT, KICK, HIHAT, KICK, HIHAT};
DrumType drumSequence10[] = {SNARE, HIHAT, KICK};
DrumType drumSequence11[] = {KICK, SNARE, KICK, SNARE};
DrumType drumSequence12[] = {HIHAT, HIHAT, HIHAT, HIHAT};
DrumType drumSequence13[] = {KICK, HIHAT, SNARE, HIHAT, KICK, TOM, HIHAT, CLAP};
DrumType drumSequence14[] = {KICK};
DrumType drumSequence15[] = {KICK, HIHAT, KICK, HIHAT, SNARE};
DrumType drumSequence16[] = {KICK, SNARE, HIHAT, HIHAT, KICK, RIM};

// ---------- Drum sequences ----------
DrumType *drumSequences[] = {drumSequence,   drumSequence1,  drumSequence2,
                             drumSequence3,  drumSequence4,  drumSequence5,
                             drumSequence6,  drumSequence7,  drumSequence8,
                             drumSequence9,  drumSequence10, drumSequence11,
                             drumSequence12, drumSequence13, drumSequence14,
                             drumSequence15, drumSequence16};

// Corresponding lengths
uint8_t drumSequenceLengths[] = {sizeof(drumSequence) / sizeof(DrumType),
                                 sizeof(drumSequence1) / sizeof(DrumType),
                                 sizeof(drumSequence2) / sizeof(DrumType),
                                 sizeof(drumSequence3) / sizeof(DrumType),
                                 sizeof(drumSequence4) / sizeof(DrumType),
                                 sizeof(drumSequence5) / sizeof(DrumType),
                                 sizeof(drumSequence6) / sizeof(DrumType),
                                 sizeof(drumSequence7) / sizeof(DrumType),
                                 sizeof(drumSequence8) / sizeof(DrumType),
                                 sizeof(drumSequence9) / sizeof(DrumType),
                                 sizeof(drumSequence10) / sizeof(DrumType),
                                 sizeof(drumSequence11) / sizeof(DrumType),
                                 sizeof(drumSequence12) / sizeof(DrumType),
                                 sizeof(drumSequence13) / sizeof(DrumType),
                                 sizeof(drumSequence14) / sizeof(DrumType),
                                 sizeof(drumSequence15) / sizeof(DrumType),
                                 sizeof(drumSequence16) / sizeof(DrumType)};