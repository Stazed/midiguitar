#include "guitar.h"
#include <FL/fl_ask.H>
#include <math.h>


Guitar::Guitar(uint a_type, uint a_CC, std::string name, uint a_channel):
    Fl_Double_Window(1020, 280,"Midi Guitar Player"),
    m_guitar_type(a_type),
    m_guitar_string_param(a_CC),
    m_client_name(name),
    m_midi_channel(a_channel)
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
    for(int x=0; x<24; x++) // the numbered fret positions & round buttons(now invisible)
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
        Fl_Button* b = new Fl_Button(45,(y+1)*c_global_fret_height+(y>=6?12*40:75),45,c_global_fret_height,"Open");
        b->color(FL_YELLOW);
        b->color2(FL_RED);
        b->when(FL_WHEN_CHANGED);
        b->callback((Fl_Callback*) fret_callback,this);
        fret[n]=b;
        n++;
        for (int x=0; x<24; x++) // The actual frets
        {
            float distance1 = Guitar::fret_distance(x);
            float distance2 = Guitar::fret_distance(x+1);

            float fret_W = distance2 - distance1;
            Fl_Button* b = new Fl_Button((distance1*c_global_pixel_scale)+90,(y+1)*c_global_fret_height+(y>=6?12*40:75),fret_W*c_global_pixel_scale,c_global_fret_height);
            b->color((Fl_Color)18);
            b->color2(FL_RED);
            b->when(FL_WHEN_CHANGED);
            b->callback((Fl_Callback*) fret_callback,this);
            fret[n]=b;
            n++;
        }
    }

    int y = 98;

    char note_string[] = "EBGDAE";
    char note_Reverse[] = "EADGBE";

    if(m_guitar_type == 1 || m_guitar_type == 3)
    {
        for(int i = 0; i < 6; i++)
            note_string[i] = note_Reverse[i];

        for(int i=5; i>=0; i--)
        {
            for(int j=0; j<25; j++)
            {
                m_note_array[i][j] = guitarMidiNote[i][j];
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
        y += c_global_fret_height;
    }
    
    y=100;
    for(int i = 0; i < 6; i++)
    {
        Fl_Button* o = new Fl_Button(12, y, 15, 15);
        o->box(FL_NO_BOX);
        o->labelsize(9);
        
        int label = i + 1;
        if(m_guitar_type == 1 || m_guitar_type == 3)
        {
            label = 7 - label;
        }
        
        o->copy_label(SSTR( label ).c_str()); // i = 1 to 6
        o->selection_color(FL_BLACK);
        o->align(Fl_Align(FL_ALIGN_LEFT));
        y += c_global_fret_height;
    }
    
    Guitar::marker(255,250);
    Guitar::marker(367,250);
    Guitar::marker(470,250);
    Guitar::marker(564,250);
    Guitar::marker(683,250);
    Guitar::marker(683,270);
    Guitar::marker(783,250);
    Guitar::marker(839,250);
    Guitar::marker(890,250);
    Guitar::marker(936,250);

    this->size_range(1020,280,0,0,0,0,1); // sets minimum & the 1 = scalable
    this->resizable(this);
    
    
    mHandle = 0;
    char portname[64];

    if (snd_seq_open(&mHandle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0)
    {
        fl_alert("Error opening ALSA sequencer.\n");
        exit(-1);
    }

    snd_seq_set_client_name(mHandle,m_client_name.c_str());
    
    sprintf(portname, "midi in");
    if ((in_port = snd_seq_create_simple_port(mHandle, portname,
                   SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
                   SND_SEQ_PORT_TYPE_APPLICATION)) < 0)
    {
        fl_alert("Error creating sequencer port.\n");
        exit(-1);
    }

    sprintf(portname, "midi out");
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
    
    m_have_string_toggle = false;
    m_bReset = false;
    m_bcontrol = true;
    m_last_fret = false;
    m_last_used_fret = -1;
    
    for(int i = 0; i< 6; i++)
    {   
        stringToggle(i);
        storeFretLocation[i] = -1;
    }
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
                if(ev->data.control.value >=1 && ev->data.control.value <=6) // must do or it gets sent to random key pointer
                {
                    m_have_string_toggle = true;
                    m_last_fret = false;
                    if(m_guitar_type == 0)
                        stringToggle(ev->data.control.value -1);  // we use 0 to 5, but user is 1 to 6
                    else
                        stringToggle(5-(ev->data.control.value -1));
                }
            }

            if (ev->type == SND_SEQ_EVENT_NOTEON)
                fretToggle(ev->data.note.note,true);

            if ((ev->type == SND_SEQ_EVENT_NOTEOFF))
                fretToggle(ev->data.note.note,false);
            
            snd_seq_ev_set_subs(ev);
            snd_seq_ev_set_direct(ev);
            snd_seq_ev_set_source(ev, out_port);
            snd_seq_event_output_direct(mHandle, ev);

            snd_seq_drain_output(mHandle);

            snd_seq_free_event(ev);
            
        }
    }
    while (ev);

    Fl::add_timeout(0.01,Guitar::TimeoutStatic,this);
}

void Guitar::stringToggle(int gString)
{
    //printf("gstring %d\n",gString);
    if(gtString[gString]->value() == 1)
        gtString[gString]->value(0);
    else
        gtString[gString]->value(1);
}

void Guitar::fretToggle(uint note,bool on_off)
{
     // guitar grid is 0 - 150
    
    bool found_fret = false;
    
     for(int i=0; i<6; i++) // strings
     {
         for(int j=0; j<25; j++)  // frets
         {
             static int I,J;
             if((note + (m_octave * 12)) == m_note_array[i][j]) // did the note match the grid?
             {
                found_fret = on_off;
                // save the location
                I = i;
                J = j;
                
                // do we want to calculate the fret location - or is it note on
                if(!m_bcontrol || !on_off) // no so send all notes found - default
                {
                    //printf("Fret [%d]\n",(I*25) +J);
                    toggle_fret((I*25) +J,on_off);
                    
                    if(I >= 6 && J >= 24) // when we are done
                        return;
                }
                else // user supplied CC starting location string or use first found
                {
                    if(m_have_string_toggle) // user supplied CC for string
                    {
                        if(gtString[I]->value() == 0)   // if the string is on and...
                        {
                            toggle_fret((I*25) +J,on_off);

                            stringToggle(I);              // shut off string after we get it
                            m_have_string_toggle = false; // shut off flag for string toggle
                            m_last_used_fret = (I*25) +J; // save the fret location
                            m_last_fret = true; // set this so we know to use it next time
                            return; // found it so leave
                        }
                    }
                    else if(m_last_fret) // we have previous fret so use it for calculation
                    {
                        // after all locations are found then falls through to end which calls calculate_closest_fret()
                        storeFretLocation[I] = (I*25) +J;
                       // printf("init store Fret %d\n",storeFretLocation[I]);
                    }
                    else // we don't have a CC or last fret so this would be the first found
                         // so use it by default
                    {
                        toggle_fret((I*25) +J,on_off);
                        m_last_used_fret = (I*25) +J; // save the fret location
                        m_last_fret = true; // set this so we know to use it
                        return;
                    }
                }
             }
         }
     }
    // this gets triggered only when we use the storeFretLocation[] 
    if(found_fret && m_last_fret)
        toggle_fret(calculate_closest_fret(),on_off);
 }

void Guitar::toggle_fret(int location, bool on_off)
{
    int string = location / 25;
    int nfret = location % 25;
    
    if(m_guitar_type == 1)
        string = (5 - string) * 25;
    else
        string *= 25;
    
    fret[string + nfret]->value(on_off);
    
    if(on_off) // true is note ON, so display note text
    {
        fret[string + nfret]->copy_label(c_key_table_text[location]);
    }
    else // clear note text
    {
        std::string label = "";
        if(nfret == 0)
            label = "Open";
        
        fret[string + nfret]->copy_label(label.c_str());
    }
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
        m_last_fret = false;
        m_last_used_fret = -1;
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

void Guitar::cb_fret_callback(Fl_Button* b)
{
    for(int i=0; i<150; i++)
    {
        if(b == fret[i])
        {
            int string = i / 25;
            int nfret = i % 25;
            int text_array = (string * 25) + nfret;
    
            if(m_guitar_type == 1)
            {
                string = (5 - string);
                text_array = (string * 25) + nfret;
            }
            snd_seq_ev_clear(&m_ev);
            
            if(fret[i]->value() == 1)
            {
                fret[i]->copy_label(c_key_table_text[text_array]);
                snd_seq_ev_set_noteon(&m_ev,m_midi_channel,m_note_array[string][nfret],127);
            }
            else
            {
                std::string label = "";
                if(nfret == 0)
                    label = "Open";
        
                fret[i]->copy_label(label.c_str());
                snd_seq_ev_set_noteoff(&m_ev,m_midi_channel,m_note_array[string][nfret],0);
            }
            
            //printf("string = %d: fret = %d: note %u\n",string,nfret, m_note_array[string][nfret]);

            snd_seq_ev_set_source(&m_ev, out_port);
            snd_seq_ev_set_subs(&m_ev);
            snd_seq_ev_set_direct(&m_ev);
            snd_seq_event_output_direct(mHandle, &m_ev);
            snd_seq_drain_output(mHandle);
           
        }
    }
    snd_seq_ev_clear(&m_ev);
}

void Guitar::fret_callback(Fl_Button* b, void* data)
{
    ((Guitar*)data)->cb_fret_callback(b);
}

int Guitar::get_fret_center(uint x_or_y, uint h_or_w)
{
    return x_or_y + (h_or_w *.5);
}

int Guitar::calculate_closest_fret()
{
    int last_X = get_fret_center(fret[m_last_used_fret]->x(),fret[m_last_used_fret]->h());
    int last_Y = get_fret_center(fret[m_last_used_fret]->y(),fret[m_last_used_fret]->w());
    
   // printf("last_x %d: last_y %d\n",last_X,last_Y);
    
    int closest_fret = -1;
    int last_diff = -1;
    
    //printf("last_fret %d\n",m_last_used_fret);
    for(int i = 0; i < 6; i++)
    {
       // printf("storeFret %d: last_diff %d\n",storeFretLocation[i],last_diff);
        if(storeFretLocation[i] == -1)
            continue;

        if(closest_fret == -1)
            closest_fret = storeFretLocation[i];
        
        int X = get_fret_center(fret[storeFretLocation[i]]->x(),fret[storeFretLocation[i]]->h());
        int Y = get_fret_center(fret[storeFretLocation[i]]->y(),fret[storeFretLocation[i]]->w());
        
       // printf("X %d: Y %d\n",X,Y);

        int current_diff = abs(last_X - X) + abs(last_Y - Y);

        if(last_diff == -1)
        {
            last_diff = current_diff;
            closest_fret = storeFretLocation[i];
            continue;
        }
        
        last_diff = (last_diff < current_diff?last_diff:current_diff);
        closest_fret = (last_diff < current_diff?closest_fret:storeFretLocation[i]);
    }
    
    for(int i = 0; i< 6; i++)
        storeFretLocation[i] = -1; // clear the array
    
    m_last_fret = true;
    m_last_used_fret = closest_fret;
    
   // printf("closest_fret %d\n",closest_fret);
    
    return closest_fret;
    
}