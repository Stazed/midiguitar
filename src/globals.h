//----------------------------------------------------------------------------
//
//  This file is part of midiguitar.
//
//  midiguitar is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  midiguitar is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with midiguitar; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//-----------------------------------------------------------------------------

#ifndef GLOBALS_H
#define GLOBALS_H

#ifndef CMAKE_BUILD
#include "config.h"
#endif

#ifdef JACK_SUPPORT
#define JACK_RINGBUFFER_SIZE 16384 // Default size for ringbuffer
#endif

const unsigned char  EVENT_STATUS_BIT       = 0x80;
const unsigned char  EVENT_NOTE_OFF         = 0x80;     // decimal 128
const unsigned char  EVENT_NOTE_ON          = 0x90;     // decimal 144
const unsigned char  EVENT_CONTROL_CHANGE   = 0xB0;
const unsigned char  EVENT_PROGRAM_CHANGE   = 0xC0;
const unsigned char  EVENT_CLEAR_CHAN_MASK  = 0xF0;
const unsigned char  EVENT_CHANNEL          = 0x0F;

const unsigned char  NOTE_ON_VELOCITY       = 100;  // default
const unsigned char  NOTE_OFF_VELOCITY      = 64;
const int            NO_FRET                = -1;

const uint c_global_min_window_h            = 280;
const uint c_global_pixel_scale             = 61;
const uint c_global_fret_height             = 20;

const uint c_global_label_resize_increment  = 20;
const uint c_global_min_group_label_size    = 10;
const uint c_global_min_label_size          = 14;
const uint c_global_min_spin_text_size      = 14;
const uint c_global_min_slide_text_size     = 11;
const uint c_global_min_number_label_size   = 9;
const uint c_global_min_fret_label_size     = 8;
const uint c_global_min_marker_size         = 60;


#define SSTR( x ) dynamic_cast< std::ostringstream && >( \
        ( std::ostringstream() << std::dec << x ) ).str()


enum{
    RH_STANDARD_GUITAR,
    RH_MIRROR_GUITAR,
    LH_STANDARD_GUITAR,
    LH_MIRROR_GUITAR
};

/*  example of SSTR
int i = 42;
std::cout << SSTR( "i is: " << i );
std::string s = SSTR( i );
puts( SSTR( i ).c_str() );

 */
/*
  0    1      2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21    22    23    24

E 76|F 77 |# 78 |G 79 |# 80 |A 81 |# 82 |B 83 |C 84 |# 85 |D 86 |# 87 |E 88 |F 89 |# 90 |G 91 |# 92 |A 93 |# 94 |B 95 |C 96 |# 97 |D 98 |# 99 |E 100|
B 71|C 72 |# 73 |D 74 |# 75 |E 76 |F 77 |# 78 |G 79 |# 80 |A 81 |# 82 |B 83 |C 84 |# 85 |D 86 |# 87 |E 88 |F 89 |# 90 |G 91 |# 92 |A 93 |# 94 |B 95 |
G 67|# 68 |A 69 |# 70 |B 71 |C 72 |# 73 |D 74 |# 75 |E 76 |F 77 |# 78 |G 79 |# 80 |A 81 |# 82 |B 83 |C 84 |# 85 |D 86 |# 87 |E 88 |F 89 |# 90 |G 91 |
D 62|# 63 |E 64 |F 65 |# 66 |G 67 |# 68 |A 69 |# 70 |B 71 |C 72 |# 73 |D 74 |# 75 |E 76 |F 77 |# 78 |G 79 |# 80 |A 81 |# 82 |B 83 |C 84 |# 85 |D 86 |
A 57|# 58 |B 59 |C 60 |# 61 |D 62 |# 63 |E 64 |F 65 |# 66 |G 67 |# 68 |A 69 |# 70 |B 71 |C 72 |# 73 |D 74 |# 75 |E 76 |F 77 |# 78 |G 79 |# 80 |A 81 |
E 52|F 53 |# 54 |G 55 |# 56 |A 57 |# 58 |B 59 |C 60 |# 61 |D 62 |# 63 |E 64 |F 65 |# 66 |G 67 |# 68 |A 69 |# 70 |B 71 |C 72 |# 73 |D 74 |# 75 |E 76 |
 */

const char c_key_table_text[][5] ={
    "E", "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#", "E", // E high
    "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", // B
    "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", // G
    "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", // D
    "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", // A
    "E", "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#", "E" // E low
};

const char c_key_table_number[][5] ={
    "64", "65", "66", "67", "68", "69", "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "80", "81", "82", "83", "84", "85", "86", "87", "88", // E high
    "59", "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "80", "81", "82", "83", // B
    "55", "56", "57", "58", "59", "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", // G
    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "70", "71", "72", "73", "74", // D
    "45", "46", "47", "48", "49", "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", // A
    "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "60", "61", "62", "63", "64" // E low
};

const uint guitarMidiNote[6][25] ={
    {64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88}, // E high
    {59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83}, // B
    {55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79}, // G
    {50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74}, // D
    {45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69}, // A
    {40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64}, // E low
};

#if 0
const char c_key_reverse_table_text[][5] ={
    "E 40", "F 41", "# 42", "G 43", "# 44", "A 45", "# 46", "B 47", "C 48", "# 49", "D 50", "# 51", "E 52", "F 53", "# 54", "G 55", "# 56", "A 57", "# 58", "B 59", "C 60", "# 61", "D 62", "# 63", "E 64", // E low
    "A 45", "# 46", "B 47", "C 48", "# 49", "D 50", "# 51", "E 52", "F 53", "# 54", "G 55", "# 56", "A 57", "# 58", "B 59", "C 60", "# 61", "D 62", "# 63", "E 64", "F 65", "# 66", "G 67", "# 68", "A 69", // A
    "D 50", "# 51", "E 52", "F 53", "# 54", "G 55", "# 56", "A 57", "# 58", "B 59", "C 60", "# 61", "D 62", "# 63", "E 64", "F 65", "# 66", "G 67", "# 68", "A 69", "# 70", "B 71", "C 72", "# 73", "D 74", // D
    "G 55", "# 56", "A 57", "# 58", "B 59", "C 60", "# 61", "D 62", "# 63", "E 64", "F 65", "# 66", "G 67", "# 68", "A 69", "# 70", "B 71", "C 72", "# 73", "D 74", "# 75", "E 76", "F 77", "# 78", "G 79", // G
    "B 59", "C 60", "# 61", "D 62", "# 63", "E 64", "F 65", "# 66", "G 67", "# 68", "A 69", "# 70", "B 71", "C 72", "# 73", "D 74", "# 75", "E 76", "F 77", "# 78", "G 79", "# 80", "A 81", "# 82", "B 83", // B
    "E 64", "F 65", "# 66", "G 67", "# 68", "A 69", "# 70", "B 71", "C 72", "# 73", "D 74", "# 75", "E 76", "F 77", "# 78", "G 79", "# 80", "A 81", "# 82", "B 83", "C 84", "# 85", "D 86", "# 87", "E 88" // E high
};

const uint guitarReverseNote[6][25] ={
    {40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64}, // E low
    {45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69}, // A
    {50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74}, // D
    {55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79}, // G
    {59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83}, // B
    {64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88}, // E high
};
#endif

#endif  // GLOBALS_H
