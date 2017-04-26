#include "guitar.h"

Guitar* MidiGuitar = 0;

uint Guitar_String_Param = 0;
int octave = 0;
uint guitar_type = 0;
uint note_array[6][25];

int main(int argc, char **argv)
{
    Guitar_String_Param = 16; // TODO argv
    octave = 0; // TODO argv
    guitar_type = 1; // TODO argv  types 0 = R/h normal, 1 = R/h mirror, 2 = L/h normal, 3 = L/h mirror

    MidiGuitar = new Guitar;
    MidiGuitar->show();

    Fl::add_timeout(1.0,Guitar::TimeoutStatic,MidiGuitar);

    return Fl::run();
}
