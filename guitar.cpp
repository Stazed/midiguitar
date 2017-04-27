#include "guitar.h"
#include <FL/fl_ask.H>
#include <math.h>


Guitar::Guitar():Fl_Double_Window(1020, 300,"Midi Guitar Player")
{

    {
        Fl_Spinner* o = new Fl_Spinner(250, 30, 40, 25, "Octave");
        o->minimum(-3);
        o->maximum(3);
        o->value(m_octave);
        o->align(Fl_Align(FL_ALIGN_TOP));
        o->callback((Fl_Callback*)spin_callback,this);
    } // Fl_Spinner* o
    {
        Fl_Button* o = new Fl_Button(60, 15, 70, 45, "Reset");
        o->color((Fl_Color)2);
        o->selection_color((Fl_Color)135);
        o->callback((Fl_Callback*) reset_callback,this);
    } // Fl_Button* o
    {
        Fl_Button* o = new Fl_Button(150, 15, 70, 45, "Control\n On/Off");
        o->type(1);
        o->color(FL_GREEN);
        o->selection_color(FL_FOREGROUND_COLOR);
        o->callback((Fl_Callback*) control_callback,this);
    } //

    int n = 0;

    {
        Fl_Button* b = new Fl_Button(60,80,15,15);    // first OPEN string position 
        b->box(FL_NO_BOX);
        b->labelsize(9);
        b->copy_label(SSTR( n ).c_str()); // position 0
        b->color(FL_GREEN);
        b->color2(FL_BLACK);
        //b->align(Fl_Align(FL_ALIGN_TOP));
        n++;
    }
    for(int x=0; x<24; x++) // the numbered fret positions & round buttons
    {
        float distance1 = Guitar::fret_distance(x);
        float distance2 = Guitar::fret_distance(x+1);
        float X = distance1 +((distance2 - distance1)/2);

        Fl_Button* b = new Fl_Button((X*60.4)+90,80,15,15);
        b->box(FL_NO_BOX);
        b->labelsize(9);
        b->copy_label(SSTR( n ).c_str()); // n = 1 to 24
        b->color(FL_GREEN);
        b->color2(FL_BLACK);
        //b->align(Fl_Align(FL_ALIGN_TOP));
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
        for (int x=0; x<24; x++) // The actual frets
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

    if(global_guitar_type == 1 || global_guitar_type == 3)
    {
        for(int i = 0; i < 6; i++)
            note_string[i] = note_Reverse[i];

        for(int i=0; i<6; i++)
        {
            for(int j=0; j<25; j++)
            {
                m_note_array[i][j] = guitarReverseNote[i][j];
            }
        }
    }
    else
    {
        for(int i=0; i<6; i++)
        {
            for(int j=0; j<25; j++)
            {
                m_note_array[i][j] = guitarMidiNote[i][j];
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

    this->size_range(1020,300,0,0,0,0,1); // sets minimum & the 1 = scalable
    this->resizable(this);
    
    
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
    
    m_have_string_toggle = true; // FIXME
    m_bReset = false;
    m_bcontrol = true;
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
    for(int i=0; i < 6; i++)
        gtString[i]->value(1);
    for(int i=0; i < 127; i++)
        fretToggle(i,false);

    m_have_string_toggle = false;
    m_bReset = false;
}

void Guitar::Timeout(void)
{
    if(m_bReset)
        Guitar::reset_all_controls();

    snd_seq_event_t *ev;
    do
    {
        snd_seq_event_input(mHandle, &ev);

        if(ev)
        {
            snd_seq_free_event(ev);

            if(ev->type == SND_SEQ_EVENT_CONTROLLER && ev->data.control.param == m_guitar_string_param && m_bcontrol == true)
            {
                if(ev->data.control.value >=0 && ev->data.control.value <=5) // must do or it gets sent to random key pointer
                {
                    m_have_string_toggle = true;
                    stringToggle(ev->data.control.value);
                }
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

void Guitar::fretToggle(uint note,bool on_off)
{
     // guitar grid is 0 - 150

     // need to save grid location [i][j] to static [I][J] and check if == occurrence
     // if occurrence == 0 then just do trigger
     // if occurrence == match then trigger below
     // if no match then continue searching until match or end of array
     // if end of array(i == 5, j == 24)  then trigger the last note in static
     // don't trigger last note unless occur_count is > 1


     for(int i=0; i<6; i++) // strings
     {
         for(int j=0; j<25; j++)  // frets
         {
             static int I,J;
             if((note + (m_octave * 12)) == m_note_array[i][j]) // did the note match the grid?
             {
                // save the location
                I = i;
                J = j;
                
                // do we have a starting string location
                if(!m_have_string_toggle) // no so send all notes found - default
                {
                    toggle_fret((I*25) +J,on_off);
                }
                else // user supplied CC starting location
                {
                    if(gtString[I]->value() == 0)   // if the string is on and...
                    {
                        // get the  x/y center and save it
                        // on next note compare all occurrences of the new note x/y to the previous saved note
                        // select the note with the lowest x-x + y-y value & play it & save it
                        // goto the top
                        
                        toggle_fret((I*25) +J,on_off);
                       
                        stringToggle(I);              // shut off string after we get it
                        m_have_string_toggle = false; // shut off flag for string toggle
                    }
                }
             }
         }
     }
 }

void Guitar::toggle_fret(int location, bool on_off)
{
    fret[location]->value(on_off); // convert from 2d struct array to 1d button array
    
    if(on_off)
        fret[location]->copy_label("On");
    else
         fret[location]->copy_label("");
    
    printf("center x=%d: center y=%d\n",get_fret_center_x(fret[location]->x(),fret[location]->h()),
        get_fret_center_y(fret[location]->y(),fret[location]->w()));
     //fret[(I*25) +J]->copy_label(SSTR( n ).c_str()); // position 0
}

void Guitar::cb_spin_callback(Fl_Spinner* o)
{
    m_octave = o->value();
}

void Guitar::spin_callback(Fl_Spinner* o, void* data)
{
    ((Guitar*)data)->cb_spin_callback(o);
}

void Guitar::cb_reset_callback(Fl_Button* o)
{
    m_bReset = true;
}

void Guitar::reset_callback(Fl_Button* o, void* data)
{
    ((Guitar*)data)->cb_reset_callback(o);
}

void Guitar::cb_control_callback(Fl_Button *b)
{
    if(b->value() == 1)
    {
        m_bcontrol = false;
    }
    else
    {
        m_bcontrol = true;
    }
}

void Guitar::control_callback(Fl_Button *b, void* data)
{
    ((Guitar*)data)->cb_control_callback(b);
}

int Guitar::get_fret_center_x(uint x, uint h)
{
    return x + (h *.5);
}

int Guitar::get_fret_center_y(uint y, uint w)
{
    return y + (w * .5);
}