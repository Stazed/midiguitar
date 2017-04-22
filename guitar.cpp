#include "guitar.h"
//#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <math.h>

bool bReset = false;
bool bcontrol = true;
uint occurrence = 0;


Guitar::Guitar():Fl_Double_Window(1020, 300,"Midi Guitar Player")
{

    {
        Fl_Spinner* o = new Fl_Spinner(420, 25, 40, 25, "Octave");
        o->minimum(-3);
        o->maximum(3);
        o->value(octave);
        o->align(Fl_Align(FL_ALIGN_TOP));
        o->callback((Fl_Callback*) spin_callback);
    } // Fl_Spinner* o
    {
        Fl_Button* o = new Fl_Button(500, 15, 70, 45, "Reset");
        o->color((Fl_Color)2);
        o->selection_color((Fl_Color)135);
        o->callback((Fl_Callback*) reset_callback);
    } // Fl_Button* o
    {
        Fl_Button* o = new Fl_Button(700, 15, 70, 45, "Control\n On/Off");
        o->type(1);
        o->color(FL_GREEN);
        o->selection_color(FL_FOREGROUND_COLOR);
        o->callback((Fl_Callback*) control_callback);
    } //

    {
        Fl_Text_Display* o = new Fl_Text_Display(210, 15, 0, 0, "Occurance");
        o->box(FL_UP_FRAME);
        o->labelfont(9);
        o->labelsize(10);
    }
    int x = 140;
    for(int i = 0; i < 6; i++)
    {
        Fl_Button* o = new Fl_Button(x, 40, 15, 15);
        o->box(FL_ROUND_UP_BOX);
        o->color(FL_BLUE);
        o->copy_label(SSTR( i + 1 ).c_str());
        o->selection_color(FL_BLACK);
        o->align(Fl_Align(FL_ALIGN_TOP));
        occur_Array[i]= o;
        x+=25;
    }

    int n = 0;

    {
        Fl_Button* b = new Fl_Button(60,80,15,15);    // first OPEN string position
        b->box(FL_ROUND_UP_BOX);
        b->labelsize(9);
        b->copy_label(SSTR( n ).c_str());
        b->color(FL_GREEN);
        b->color2(FL_BLACK);
        b->align(Fl_Align(FL_ALIGN_TOP));
        fret_Off_On[n]=b;
        n++;
    }
    for(int x=0; x<24; x++) // the actual frets
    {
        float distance1 = Guitar::fret_distance(x);
        float distance2 = Guitar::fret_distance(x+1);
        float X = distance1 +((distance2 - distance1)/2);

        Fl_Button* b = new Fl_Button((X*60.4)+90,80,15,15);
        b->box(FL_ROUND_UP_BOX);
        b->labelsize(9);
        b->copy_label(SSTR( n ).c_str());
        b->color(FL_GREEN);
        b->color2(FL_BLACK);
        b->align(Fl_Align(FL_ALIGN_TOP));
        fret_Off_On[n]=b;
        n++;
    }

    n = 0;  // reset and reuse

    for (int y=0; y<6; y++)
    {
        Fl_Button* b = new Fl_Button(45,(y+1)*25+(y>=6?12*40:75),45,25,"Open");
        b->color(FL_YELLOW);
        b->color2(FL_RED);
        fret[n]=b;
        n++;
        for (int x=0; x<24; x++)
        {
            float distance1 = Guitar::fret_distance(x);
            float distance2 = Guitar::fret_distance(x+1);

            float fret_W = distance2 - distance1;
            Fl_Button* b = new Fl_Button((distance1*61)+90,(y+1)*25+(y>=6?12*40:75),fret_W*61,25);
            b->color((Fl_Color)18);
            b->color2(FL_RED);
            fret[n]=b;
            n++;
        }
    }

    int y = 106;

    char note_string[] = "EBGDAE";
    char note_Reverse[] = "EADGBE";
//    uint note_array[6][25];

    if(guitar_type == 1 || guitar_type == 3)
    {
        for(int i = 0; i < 6; i++)
            note_string[i] = note_Reverse[i];

        for(int i=0; i<6; i++)
        {
            for(int j=0; j<25; j++)
            {
                note_array[i][j] = guitarReverseNote[i][j];
            }
        }
    }
    else
    {
        for(int i=0; i<6; i++)
        {
            for(int j=0; j<25; j++)
            {
                note_array[i][j] = guitarMidiNote[i][j];
            }
        }
    }

    for(int i = 0; i < 6; i++)
    {
        Fl_Button* o = new Fl_Button(25, y, 15, 15);
        o->box(FL_ROUND_UP_BOX);
        o->color(FL_GREEN);
        char temp[2] = {};
        temp[0]=note_string[i];
        o->copy_label(temp);
        o->selection_color(FL_BLACK);
        o->align(Fl_Align(FL_ALIGN_LEFT));
        gtString[i]= o;
        y += 25;
    }
    Guitar::marker(255,285);
    Guitar::marker(367,285);
    Guitar::marker(470,285);
    Guitar::marker(564,285);
    Guitar::marker(683,285);
    Guitar::marker(683,305);
    Guitar::marker(783,285);
    Guitar::marker(839,285);
    Guitar::marker(890,285);
    Guitar::marker(936,285);

    mHandle = 0;
    char portname[64];

    if (snd_seq_open(&mHandle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0)
    {
        fl_alert("Error opening ALSA sequencer.\n");
        exit(-1);
    }

    snd_seq_set_client_name(mHandle,"MIDI Guitar Player");

    sprintf(portname, "MIDI Guitar Player IN");
    if ((in_port = snd_seq_create_simple_port(mHandle, portname,
                   SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
                   SND_SEQ_PORT_TYPE_APPLICATION)) < 0)
    {
        fl_alert("Error creating sequencer port.\n");
        exit(-1);
    }

    sprintf(portname, "MIDI Guitar Player OUT");
    if ((out_port = snd_seq_create_simple_port(mHandle, portname,
                    SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
                    SND_SEQ_PORT_TYPE_APPLICATION)) < 0)
    {
        fl_alert("Error creating sequencer port.\n");
        exit(-1);
    }

    mPollMax = snd_seq_poll_descriptors_count(mHandle,POLLIN);
    mPollFds = (struct pollfd *) calloc(mPollMax, sizeof(struct pollfd));

    snd_seq_nonblock(mHandle, 1);
    //ctor
}

Guitar::~Guitar()
{
    snd_seq_close(mHandle);
    //dtor
}


float Guitar::fret_distance(int num_fret)
{
    return 20 - (20 / (pow(2.0,(num_fret/12.0))));
}

void Guitar::marker(int x,int y)
{
    Fl_Text_Display* o = new Fl_Text_Display(x, y, 0, 0, ".");
    o->box(FL_UP_FRAME);
    o->labelfont(9);
    o->labelsize(60);
}

void Guitar::reset_all_controls()
{
    for(int i=0; i < 25; i++)
        fret_Off_On[i]->value(0);
    for(int i=0; i < 6; i++)
        gtString[i]->value(0);
    for(int i=0; i < 127; i++)
        fretToggle(i,false);

    occur_Number(0);
//    occurrence = 0;

    bReset = false;
}

void Guitar::Timeout(void)
{
    if(bReset)
        Guitar::reset_all_controls();

    snd_seq_event_t *ev;
    do
    {
        snd_seq_event_input(mHandle, &ev);

        if(ev)
        {
            snd_seq_free_event(ev);

            if(ev->type == SND_SEQ_EVENT_CONTROLLER && ev->data.control.param == Guitar_String_Param && bcontrol == true)
            {
                if(ev->data.control.value >=0 && ev->data.control.value <=5) // must do or it gets sent to random key pointer
                    stringToggle(ev->data.control.value);
            }
            if(ev->type == SND_SEQ_EVENT_CONTROLLER && ev->data.control.param == fret_Number_Param  && bcontrol == true)
            {
                if(ev->data.control.value >=0 && ev->data.control.value <=24) // must do or it gets sent to random key pointer
                    fret_On_Off(ev->data.control.value);
            }
            if(ev->type == SND_SEQ_EVENT_CONTROLLER && ev->data.control.param == occur_Param  && bcontrol == true)
            {
                if(ev->data.control.value >=0 && ev->data.control.value <=6) // must do or it gets sent to random key pointer
                    occur_Number(ev->data.control.value);
            }

            snd_seq_ev_set_subs(ev);
            snd_seq_ev_set_direct(ev);

            if (ev->type == SND_SEQ_EVENT_NOTEON)
                fretToggle(ev->data.note.note,true);

            if ((ev->type == SND_SEQ_EVENT_NOTEOFF))
                fretToggle(ev->data.note.note,false);

            snd_seq_ev_set_source(ev, out_port);
            snd_seq_event_output_direct(mHandle, ev);

            snd_seq_drain_output(mHandle);

            snd_seq_free_event(ev);
        }
    }
    while (ev);

    Fl::add_timeout(0.001,Guitar::TimeoutStatic,this);
}

void Guitar::stringToggle(int gString)
{
    if(gtString[gString]->value() == 1)
        gtString[gString]->value(0);
    else
        gtString[gString]->value(1);
}

void Guitar::fret_On_Off(int fret_Number)
{
    if(fret_Off_On[fret_Number]->value()==1)
        fret_Off_On[fret_Number]->value(0);
    else
        fret_Off_On[fret_Number]->value(1);
}

void Guitar::occur_Number(uint number)
{
     // array == 0 to 5 representing number 1 to 6
     // number 0 is all on 0 to 5

     occurrence = number;

     for(uint i = 0; i < 6; i++)
     {
         if(number == 0) // then turn on all notes
         {
             occur_Array[i]->value(0);
         }
         else if(i == (occurrence -1)) // then turn on the one note offset by array 0 = 1
         {
             occur_Array[i]->value(0);
         }
         else    // turn off everything else
             occur_Array[i]->value(1);
     }
 }

void Guitar::fretToggle(uint note,bool on_off)
{
     // guitar grid is 0 - 150

     // need to save grid location [i][j] to static [I][J] and check if == occurrence
     // if occurrence == 0 then just do trigger
     // if occurrence == match then trigger below
     // if no match then continue searching until match or end of array
     // if end of array(i == 5, j == 24)  then trigger the last note in static
     // don't trigger last note unless occur_count is > 1


     static uint occur_count = 1;

     for(int i=0; i<6; i++)
     {
         for(int j=0; j<25; j++)
         {
             static int I,J;
             if((note + (octave * 12)) == note_array[i][j]) // did the note match the grid?
             {
                 I = i;
                 J = j;
                 if(occurrence == 0 || bReset)
                 {
                     if(gtString[I]->value() == 0)   // if the string is on and...
                     {
                         if(fret_Off_On[J]->value() == 0) // if the fret is on then play it
                             fret[(I*25) +J]->value(on_off); // convert from 2d struct array to 1d button array
                     }
                     occur_count = 1;
                 }
                 else if(occur_count == occurrence ) // if match of occurrence
                 {
                     if(gtString[I]->value() == 0)   // if the string is on and...
                     {
                         if(fret_Off_On[J]->value() == 0) // if the fret is on then play it
                             fret[(I*25) +J]->value(on_off); // convert from 2d struct array to 1d button array
                     }
                     occur_count = 1;    // we found it so reset counter and return for next event
                     return;
                 }
                 occur_count++;
             }

             if(i == 5 && j == 24 && occur_count > 1) // only if occur_count >1 because that means NOT out of range
             {
                 if(gtString[I]->value() == 0)   // if the string is on and...
                 {
                     if(fret_Off_On[J]->value() == 0) // if the fret is on then play it
                         fret[(I*25) +J]->value(on_off); // convert from 2d struct array to 1d button array
                 }
             }
         }
     }
     occur_count = 1;
 }

void Guitar::spin_callback(Fl_Spinner* b, void*)
{
    octave = b->value();
}

void Guitar::reset_callback(Fl_Button*, void*)
{
    bReset = true;
}

void Guitar::control_callback(Fl_Button *b)
{
    if(b->value() == 1)
    {
        bcontrol = false;
    }
    else
    {
        bcontrol = true;
    }
}

