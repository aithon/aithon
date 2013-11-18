#ifndef _MUSIC_H
#define _MUSIC_H

typedef struct
{
	uint16_t frequency;
	uint16_t duration;
} MusicNote;

typedef enum
{
   C0 = 16,
   CS0 = 17, DF0 = 17,
   D0 = 18,
   DS0 = 19, EF0 = 19,
   E0 = 21,
   F0 = 22,
   FS0 = 23, GF0 = 23,
   G0 = 24,
   GS0 = 26, AF0 = 26,
   A0 = 28, // lowest note of piano
   AS0 = 29, BF0 = 29,
   B0 = 31,

   C1 = 33,
   CS1 = 35, DF1 = 35,
   D1 = 37,
   DS1 = 39, EF1 = 39,
   E1 = 41, // lowest note of bass
   F1 = 44,
   FS1 = 46, GF1 = 46,
   G1 = 49,
   GS1 = 52, AF1 = 52,
   A1 = 55,
   AS1 = 58, BF1 = 58,
   B1 = 62,

   C2 = 65,
   CS2 = 69, DF2 = 69,
   D2 = 73,
   DS2 = 78, EF2 = 78,
   E2 = 82, // lowest note of guitar
   F2 = 87,
   FS2 = 92, GF2 = 92,
   G2 = 98,
   GS2 = 104, AF2 = 104,
   A2 = 110,
   As2 = 117, BF2 = 117,
   B2 = 123,

   C3 = 131,
   CS3 = 139, DF3 = 139,
   D3 = 147,
   DS3 = 156, EF3 = 156,
   E3 = 165,
   F3 = 175,
   FS3 = 185, GF3 = 185,
   G3 = 196, // lowest note of violin
   GS3 = 208, AF3 = 208,
   A3 = 220,
   AS3 = 233, BF3 = 233,
   B3 = 247,

   C4 = 262, // midle C
   CS4 = 277, DF4 = 277,
   D4 = 294,
   DS4 = 311, EF4 = 311,
   E4 = 330,
   F4 = 349,
   FS4 = 370, GF4 = 370,
   G4 = 392,
   GS4 = 415, AF4 = 415,
   A4 = 440, // tuning reference note
   AS4 = 466, BF4 = 466,
   B4 = 494,

   C5 = 523,
   CS5 = 554, DF5 = 554,
   D5 = 587,
   DS5 = 622, EF5 = 622,
   E5 = 659,
   F5 = 698,
   FS5 = 740, GF5 = 740,
   G5 = 784,
   GS5 = 831, AF5 = 831,
   A5 = 880,
   AS5 = 932, BF5 = 932,
   B5 = 988,

   C6 = 1047,
   CS6 = 1109, DF6 = 1109,
   D6 = 1175,
   DS6 = 1245, EF6 = 1245,
   E6 = 1319,
   F6 = 1397,
   FS6 = 1480, GF6 = 1480,
   G6 = 1568,
   GS6 = 1661, AF6 = 1661,
   A6 = 1760,
   AS6 = 1865, BF6 = 1865,
   B6 = 1976,
   
   C7 = 2093,
   CS7 = 2217, DF7 = 2217,
   D7 = 2349,
   DS7 = 2489, EF7 = 2489,
   E7 = 2637,
   F7 = 2794,
   FS7 = 2960, GF7 = 2960,
   G7 = 3136,
   GS7 = 3322, AF7 = 3322,
   A7 = 3520,
   AS7 = 3729, BF7 = 3729,
   B7 = 3951,
   
   C8 = 4186,
   CS8 = 4435, DF8 = 4435,
   D8 = 4699,
   DS8 = 4978, EF8 = 4978,
   E8 = 5274,
   F8 = 5588,
   FS8 = 5920, GF8 = 5920,
   G8 = 6272,
   GS8 = 6645, AF8 = 6645,
   A8 = 7040,
   AS8 = 7459, BF8 = 7459,
   B8 = 7902,
   
   C9 = 8372,
   CS9 = 8870, DF9 = 8870,
   D9 = 9397,
   DS9 = 9956, EF9 = 9956,
   E9 = 10548,
   F9 = 11175,
   FS9 = 11840, GF9 = 11840,
   G9 = 12544,
   GS9 = 13290, AF9 = 13290,
   A9 = 14080,
   AS9 = 14917, BF9 = 14917,
   B9 = 15804,
   
   VOID = 0 // use for pauses
} MusicPitch;

// "Song of Time" from The Legend of Zelda: Orcarina of Time
#define SONG_OF_TIME 			(MusicNote[]) {{A5, 500}, {D5, 1000}, {F5, 500}, {A5, 500}, {D5, 1000}, \
										 {F5, 500}, {A5, 250}, {C6, 250}, {B5, 500}, {G5, 500}, \
										 {F5, 250}, {G5, 250}, {A5, 500}, {D5, 500}, {C5, 250}, \
										 {E5, 250}, {D5, 1500}}

// "Achievement!" from The Legend of Zelda: Orcarina of Time
#define ACHIEVEMENT   			(MusicNote[]) {{G5, 136}, {FS5, 136}, {EF5, 136}, {A4, 136}, \
										 {AF4, 136},  {E5, 136}, {AF5, 136}, {C6, 136}}

// "1 Up" from Super Mario Bros.
#define ONE_UP          	   (MusicNote[]) {{E4, 167}, {G4, 167}, {E5, 167}, {C5, 167}, \
										 {D5, 167}, {G5, 167}}

// Notes for "Game Over" from The Legend of Zelda: Orcarina of Time

#define GAME_OVER         	   (MusicNote[]) {{E4, 500}, {FS4, 500}, {B4, 500}, \
										 {E5, 500}, {FS5, 500}, {B5, 500}}

// Notes for "The Imperial March" from "The Empire Strikes Back"
#define IMPERIAL_MARCH        (MusicNote[]) {{E3, 500}, {E3, 500}, {E3, 500}, {C3, 375}, {G3, 125}, \
                               {E3, 500}, {C3, 375}, {G3, 125}, {E3, 1000},\
                               {B3, 500}, {B3, 500}, {B3, 500}, {C4, 375}, {G3, 125},\
                                \
                               {EF3, 500}, {C3, 375}, {G3, 125}, {E3, 1000},\
                               {E4, 500}, {E3, 375}, {E3, 125}, {E4, 500}, {EF4, 375}, {D4, 125}, \
                               {DF4, 125}, {C4, 125}, {DF4, 250}, {VOID, 250}, {F3, 250}, {BF3, 500}, {A3, 375}, {GS3, 125}, \
                                \
                               {G3, 125}, {FS3, 125}, {G3, 250}, {VOID, 250}, {C3, 250}, {EF3, 500}, {C3, 375}, {EF3, 125}, \
                               {G3, 500}, {E3, 375}, {G3, 125}, {B3, 1000}, \
                               {E4, 500}, {E3, 375}, {E3, 125}, {E4, 500}, {EF4, 375}, {D4, 125}, \
                                \
                               {DF4, 125}, {C4, 125}, {DF4, 250}, {VOID, 250}, {F3, 250}, {BF3, 500}, {A3, 375}, {AF3, 125}, \
                               {G3, 125}, {FS3, 125}, {G3, 250}, {VOID, 250}, {C3, 250}, {EF3, 500}, {C3, 375}, {G3, 125}, \
                               {G3, 500}, {E3, 375}, {G3, 125}, {B3, 1000}}

#define BUZZER_DEMO           (MusicNote[]) {{G5, 167}, {D5, 167}, {C5, 167}, {E5, 167}, {G4, 167}, {E4, 167}}

#endif