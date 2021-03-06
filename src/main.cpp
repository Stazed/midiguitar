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


#include <getopt.h>
#include "guitar.h"


/* struct for command parsing */
static struct
option long_options[] ={
    {"help", 0, 0, 'h'},
    {"string_CC_value", required_argument, 0, 's'},
    {"guitar_view_type", required_argument, 0, 't'},
    {"client_name", required_argument, 0, 'n'},
    {"midi_channel", required_argument, 0, 'c'},
    {"Fltk scheme", required_argument, 0, 'S'},
    {"display_midi_numbers", 0, 0, 'm'},
    {0, 0, 0, 0}
};

int main(int argc, char **argv)
{
    std::string client_name = "MIDI_Guitar";
    uint guitar_string_param = 16;
    uint guitar_type = RH_STANDARD_GUITAR;
    uint midi_channel = 0;
    bool display_midi_numbers = false;
    uint scheme = 0;

    /* parse parameters */
    int c;

    while (true)
    {
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "hs:t:n:c:S:m", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {

            case '?':
            case 'h':

                printf("   -h, --help: show this message\n");
                printf("   -s, --string_CC_value: for defining which fret should\n");
                printf("         be played based on predefined string location (1 to 6).\n");
                printf("         The CC parameter must be in the range from 0 to 127.\n");
                printf("         The default CC value is 16.\n");
                printf("   -t, --guitar_view_type: types 0 = R/h normal, 1 = R/h mirror\n");
                //                    ", 2 = L/h normal, 3 = L/h mirror\n" );
                printf("   -n, --client_name <name>: set program client name: Default = 'MIDI_Guitar'\n");
                printf("   -c, --midi_channel: set midi channel for sending notes from fret mouse press (1 to 16)\n");
                printf("   -S, --Fltk_scheme: set fltk scheme type 1 = none, 2 = gtk+, 3 = plastic, 4 = gleam\n");
                printf("   -m, --midi_numbers: display midi numbers on fret instead of note letter\n");
                printf("\n\n\n");

                return EXIT_SUCCESS;
                break;

            case 't':
                if (atoi(optarg) >= RH_STANDARD_GUITAR && atoi(optarg) <= RH_MIRROR_GUITAR)
                {
                    guitar_type = (atoi(optarg));
                }
                else
                {
                    printf("Invalid Guitar type %d indicated! Using standard type 0\n", atoi(optarg));
                }
                break;

            case 's':
                if (atoi(optarg) >= 0 && atoi(optarg) < 128)
                {
                    guitar_string_param = (atoi(optarg));
                }
                break;

            case 'n':
                client_name = std::string(optarg);
                break;

            case 'c':
                if (atoi(optarg) >= 1 && atoi(optarg) <= 16)
                {
                    midi_channel = (atoi(optarg)) - 1; // -1 for user offset
                }
                break;
            case 'm':
                display_midi_numbers = true;
                break;
            case 'S':
                if (atoi(optarg) >= 1 && atoi(optarg) <= 4)
                {
                    scheme = (atoi(optarg));
                }
                break;
        }
    }


    Guitar* MidiGuitar = 0;

    MidiGuitar = new Guitar(guitar_type, guitar_string_param, client_name, midi_channel, display_midi_numbers);
    
    /* Apply user selected scheme */
    switch (scheme)
    {
    case 0:
        Fl::scheme("");
        break;
    case 1:
        Fl::scheme("none");
        break;
    case 2:
        Fl::scheme("gtk+");
        break;
    case 3:
        Fl::scheme("plastic");
        break;
    case 4:
        Fl::scheme("gleam");
        break;
    }

    MidiGuitar->show();

    Fl::add_timeout(0.1, Guitar::TimeoutStatic, MidiGuitar);

    int ret = Fl::run();

    delete MidiGuitar;

    return ret;
}
