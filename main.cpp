#include <getopt.h>
#include "guitar.h"


/* struct for command parsing */
static struct
    option long_options[] =
{
    {"help",     0, 0, 'h'},
    {"string_CC_value", required_argument, 0, 's'},
    {"guitar_view_type", required_argument, 0, 't'},
    {"client_name", required_argument, 0, 'n'},
    {0, 0, 0, 0}
};


int main(int argc, char **argv)
{
    std::string client_name = "MIDI Guitar Player";
    uint guitar_string_param = 16;
    uint guitar_type = 0;
    
    /* parse parameters */
    int c;

    while (true)
    {
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "hs:t:n:", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {

        case '?':
        case 'h':

            printf( "   -h, --help: show this message\n" );
            printf( "   -s, --string_CC_value: for defining which fret should\n");
            printf( "         be played based on predefined string location (1 to 6).\n");
            printf( "         The CC parameter must be in the range from 0 to 127.\n");
            printf( "         The default CC value is 16.\n");
            printf( "   -t, --guitar_view_type: types 0 = R/h normal, 1 = R/h mirror\n");
//                    ", 2 = L/h normal, 3 = L/h mirror\n" );
            printf( "   -n, --client_name <name>: Set alsa client name: Default = MIDI Guitar Player\n");
            printf( "\n\n\n" );

            return EXIT_SUCCESS;
            break;

        case 't':
            if (atoi( optarg ) > 0)
            {
                guitar_type = (atoi( optarg ));
            }

        case 's':
            if(atoi( optarg ) >= 0 && atoi( optarg ) < 128)
            {
                guitar_string_param = (atoi( optarg ));
            }
            break;
            
        case 'n':
            client_name = std::string( optarg );
            break;
            
        }
    }

    
    Guitar* MidiGuitar = 0;

    MidiGuitar = new Guitar(guitar_type, guitar_string_param, client_name);
    
    MidiGuitar->show();

    Fl::add_timeout(1.0,Guitar::TimeoutStatic,MidiGuitar);

    return Fl::run();
}
