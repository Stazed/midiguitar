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


#include "guitar.h"
#include <FL/fl_ask.H>
#include <math.h>
#include <FL/names.h> // for debugging events ex: fprintf(stderr, "EVENT: %s(%d)\n", fl_eventnames[e], e);

Fret::Fret(int x, int y, int w, int h, const char *label) :
    Fl_Button(x, y, w, h, label)
{

}

Guitar::Guitar(uint a_type, uint a_CC, std::string name, uint a_channel) :
    Fl_Double_Window(1020, 280, "Midi Guitar Player"),
    m_windowLabel("Midi Guitar "),
    m_client_name(name),
    m_have_string_toggle(false),
    m_last_fret(false),
    m_bcontrol(true),
    m_guitar_type(a_type),
    m_guitar_string_param(a_CC),
    m_transpose(0),
    m_last_used_fret(NO_FRET),
    m_last_focus_fret(NO_FRET),
    m_midi_out_channel(a_channel),
    m_midi_in_channel(0),
    m_note_on_velocity(NOTE_ON_VELOCITY)
#ifdef RTMIDI_SUPPORT
    ,m_midiIn(0)
    ,m_midiOut(0)
#endif
#ifdef JACK_SUPPORT
    ,m_jack_midi_in_port(NULL)
    ,m_jack_midi_out_port(NULL)
#endif
        
{
    {
        Fl_Button* o = new Fl_Button(50, 15, 70, 45, "Reset");
        o->tooltip("Press button to clear all CC values and previous note calculation.\n"
                   "Also, note OFF will be sent to all fret locations.");
        o->color(FL_DARK_RED);
        o->selection_color((Fl_Color) 135);
        o->labelcolor(FL_WHITE);
        o->callback((Fl_Callback*) reset_callback, this);
    } // Fl_Button* o
    
    {Fl_Group* midi_in_group = new Fl_Group(145, 13, 250, 51, "Midi In");
        midi_in_group->labelsize(10);
        midi_in_group->box(FL_BORDER_BOX);
        {
            Fl_Button* o = new Fl_Button(150, 15, 70, 45, "Control\n On/Off");
            o->type(1);
            o->tooltip("Press to stop calculation of nearest fret.\n"
                       "If pressed all possible note locations will be triggered.");
            o->color(FL_GREEN);
            o->selection_color(FL_FOREGROUND_COLOR);
            o->callback((Fl_Callback*) control_callback, this);
        } // Fl_Button* o
        {
            Fl_Spinner* o = new Fl_Spinner(250, 30, 40, 25, "Transpose");
            o->minimum(-24);
            o->maximum(24);
            o->tooltip("Selected value will adjust incoming midi note up or down.");
            o->value(m_transpose);
            o->align(Fl_Align(FL_ALIGN_TOP));
            o->callback((Fl_Callback*) transpose_callback, this);
        } // Fl_Spinner* o
        {
            Fl_Spinner* s = new Fl_Spinner(330, 30, 40, 25, "Channel");
            s->minimum(0);
            s->maximum(16);
            s->tooltip("Enter the midi channel to receive input.\n"
                       "Zero '0' means all channels.");
            s->value(m_midi_in_channel);
            s->align(Fl_Align(FL_ALIGN_TOP));
            s->callback((Fl_Callback*) in_channel_callback, this);
        } // Fl_Spinner* o
        midi_in_group->end();   // Must remember to do this or everything after group declaration is included!
        Fl_Group::current()->resizable(midi_in_group);
    }
    
    {Fl_Group* midi_out_group = new Fl_Group(420, 13, 365, 51, "Midi Out");
        midi_out_group->labelsize(10);
        midi_out_group->box(FL_BORDER_BOX);
        {
            Fl_Spinner* o = new Fl_Spinner(435, 30, 50, 25, "Program");
            o->minimum(0);
            o->maximum(127);
            o->tooltip("Set program change");
            o->value(0);
            o->align(Fl_Align(FL_ALIGN_TOP));
            o->callback((Fl_Callback*) program_callback, this);
        } // Fl_Spinner* o
        {
            Fl_Spinner* s = new Fl_Spinner(520, 30, 40, 25, "Channel");
            s->minimum(1);
            s->maximum(16);
            s->tooltip("Enter/select the midi channel to send output");
            s->value(m_midi_out_channel+1);
            s->align(Fl_Align(FL_ALIGN_TOP));
            s->callback((Fl_Callback*) out_channel_callback, this);
        } // Fl_Spinner* o
        { 
            Fl_Slider* o = new Fl_Slider(600, 35, 145, 17, "Velocity");
            o->align(Fl_Align(FL_ALIGN_TOP));
            o->type(FL_HORIZONTAL);
            o->minimum(0);
            o->maximum(127);
            o->value((double)NOTE_ON_VELOCITY);
            o->callback((Fl_Callback*) velocity_callback, this);
        } // Fl_Slider* o
        midi_out_group->end();   // Must remember to do this or everything after group declaration is included!
        Fl_Group::current()->resizable(midi_out_group);
    }
    
    /* Fret numbering */
    int n = 0;

    {
        Fl_Text_Display* b = new Fl_Text_Display(65, 90, 15, 15); // first OPEN string position
        b->box(FL_NO_BOX);
        b->labelsize(9);
        b->copy_label(SSTR(n).c_str()); // position 0
        n++;
    }
    for (int x = 0; x < 24; x++) // the numbered fret positions
    {
        float distance1 = Guitar::fret_distance(x);
        float distance2 = Guitar::fret_distance(x + 1);
        float X = distance1 + ((distance2 - distance1) / 2);

        Fl_Text_Display* b = new Fl_Text_Display((X * 60.4) + 95, 90, 15, 15);
        b->box(FL_NO_BOX);
        b->labelsize(9);
        b->copy_label(SSTR(n).c_str()); // n = 1 to 24
        n++;
    }


    if (m_guitar_type == RH_MIRROR_GUITAR || m_guitar_type == LH_MIRROR_GUITAR)
    {
        for (int i = 5; i >= 0; i--)
        {
            for (int j = 0; j < 25; j++)
            {
                m_note_array[i][j] = guitarMidiNote[i][j];
            }
        }
    } else
    {
        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 25; j++)
            {
                m_note_array[i][j] = guitarMidiNote[i][j];
            }
        }
    }
    
    /* Guitar string toggle buttons */
    char note_string[] = "EBGDAE";
    int y = 98;
    for (int i = 0; i < 6; i++)
    {
        int j = i;
        if (m_guitar_type == RH_MIRROR_GUITAR || m_guitar_type == LH_MIRROR_GUITAR)
        {
            j = 5 - j;
        }
        gtString[j] = new Fl_Button(30, y, 15, 15);
        gtString[j]->color(FL_BLACK);
        char temp[2] = {};
        temp[0] = note_string[j];
        gtString[j]->copy_label(temp);
        gtString[j]->selection_color(FL_GREEN);
        gtString[j]->align(Fl_Align(FL_ALIGN_LEFT));
        gtString[j]->callback((Fl_Callback*) string_callback, this);
        y += c_global_fret_height;
    }
    
   /* String number labels*/
    y = 100;
    for (int i = 0; i < 6; i++)
    {
        Fl_Text_Display* o = new Fl_Text_Display(17, y, 15, 15);
        o->box(FL_NO_BOX);
        o->labelsize(9);

        int label = i + 1;
        if (m_guitar_type == RH_MIRROR_GUITAR || m_guitar_type == LH_MIRROR_GUITAR)
        {
            label = 7 - label;
        }

        o->copy_label(SSTR(label).c_str()); // i = 1 to 6
        o->selection_color(FL_BLACK);
        o->align(Fl_Align(FL_ALIGN_LEFT));
        y += c_global_fret_height;
    }
  
    n = 0; // reset and reuse

    /* Guitar Fret Board */
    Fl_Group* guitar_frets = new Fl_Group(47, c_global_fret_height + 72, 965, 126);
    guitar_frets->box(FL_BORDER_BOX);
    {
        for (int y = 0; y < 6; y++)
        {
            fret[n] = new Fret(50, (y + 1) * c_global_fret_height + (y >= 6 ? 12 * 40 : 75), 45, c_global_fret_height, "Open");
            fret[n]->color(FL_YELLOW);
            fret[n]->selection_color(FL_RED);
            fret[n]->when(FL_WHEN_CHANGED);
            fret[n]->align(FL_ALIGN_CLIP);
            fret[n]->callback((Fl_Callback*) fret_callback, this);
            guitar_frets->add(fret[n]);
            n++;
            for (int x = 0; x < 24; x++) // The actual frets
            {
                float distance1 = Guitar::fret_distance(x);
                float distance2 = Guitar::fret_distance(x + 1);

                float fret_W = distance2 - distance1;
                fret[n] = new Fret((distance1 * c_global_pixel_scale) + 95,
                                   (y + 1) * c_global_fret_height + (y >= 6 ? 12 * 40 : 75),
                                   fret_W*c_global_pixel_scale, c_global_fret_height);
                fret[n]->color((Fl_Color) 18);
                fret[n]->selection_color(FL_RED);
                fret[n]->when(FL_WHEN_CHANGED);
                fret[n]->align(FL_ALIGN_CLIP);
                fret[n]->labelsize(10);
                fret[n]->callback((Fl_Callback*) fret_callback, this);
                guitar_frets->add(fret[n]);
                n++;
            }
        }
    }
    guitar_frets->end();    // Must remember to do this or everything after group declaration is included!

    marker(260, 250);
    marker(372, 250);
    marker(475, 250);
    marker(571, 250);
    marker(688, 250);
    marker(688, 270);
    marker(788, 250);
    marker(844, 250);
    marker(895, 250);
    marker(941, 250);
    
    this->size_range(1020, 280, 0, 0, 0, 0, 1); // sets minimum & the 1 = scalable
    this->resizable(this);
    
    m_windowLabel += PACKAGE_VERSION;
    label(m_windowLabel.c_str());
    
    for (int i = 0; i < 6; i++)
        storeFretLocation[i] = NO_FRET; // initialize the array

#ifdef ALSA_SUPPORT
    if(!init_Alsa())
    {
        exit(-1);
    }
#endif  // ALSA_SUPPORT
    
#ifdef RTMIDI_SUPPORT
    if(!init_rt_midi())
    {
        exit(-1);
    }
#endif
    
#ifdef JACK_SUPPORT
    init_jack();
#endif
    //ctor
}

Guitar::~Guitar()
{
#ifdef ALSA_SUPPORT
    snd_seq_close(mHandle);
#endif
    
#ifdef RTMIDI_SUPPORT
    delete m_midiIn;
    delete m_midiOut;
#endif
    
#ifdef JACK_SUPPORT
    jack_ringbuffer_free( m_buffSize );
    jack_ringbuffer_free( m_buffMessage );
    if ( m_jack_client ) {
        jack_client_close (m_jack_client);}
#endif
    //dtor
}

#ifdef RTMIDI_SUPPORT
bool Guitar::init_rt_midi()
{
    if(init_rt_midi_out())
    {
        m_midiOut->openVirtualPort("Output");
    }else
    {
        fl_alert("Error creating RtMidi out port.\n");
        return false;
    } 
    
    if(init_rt_midi_in())
    {
        m_midiIn->openVirtualPort("Input");
        m_midiIn->setCallback(RtMidiCallback, (void*)this);
    }else
    {
        fl_alert("Error creating RtMidi in port.\n");
        return false;
    }
    
    return true;
}

void Guitar::RtplayMidiGuitar(std::vector< unsigned char > *message, unsigned int nBytes)
{
    /* We only care about note on/off and CC which are 3 bytes */
    if(nBytes != 3)
        return;
    
    /* events */
    unsigned char status = 0, channel = 0, parameter = 0, value = 0; 
    
    status = message->at(0);
    channel = (message->at(0)  & EVENT_CHANNEL);
    parameter = message->at(1);     // for notes = key, for CC = which one
    value = message->at(2);         // for notes = velocity, for CC = value sent
    
    /* For Notes & CC we compare with EVENT_CLEAR_CHAN_MASK because we do not 
     want to iterate through 16 different items do to the channel bit. So we
     mask it off to compare to the single generic note or CC.
     User is 1 to 16, we are 0 to 15 (= -1). Channel 0 (user) means all channels*/
    
    if(channel == m_midi_in_channel - 1 || m_midi_in_channel == 0)
    {
        if (((status & EVENT_CLEAR_CHAN_MASK) == EVENT_CONTROL_CHANGE) &&
            (parameter == m_guitar_string_param) &&
            (m_bcontrol == true))
        {
            if (value >= 1 && value <= 6)   // must do or it gets sent to random key pointer
            {
                stringToggle(value - 1);    // we use 0 to 5, but user is 1 to 6
            }
        }
              
        if ((status & EVENT_CLEAR_CHAN_MASK) == EVENT_NOTE_ON)
            fretToggle(parameter, true);

        if ((status & EVENT_CLEAR_CHAN_MASK) == EVENT_NOTE_OFF)
            fretToggle(parameter, false);
    }
}

void Guitar::RtMidiCallback(double deltatime, std::vector< unsigned char > *message, void *userData)
{
    Guitar *MidiGit = (Guitar*)userData;

    unsigned int nBytes = message->size();
    if(nBytes)
    {
        MidiGit->RtplayMidiGuitar(message, nBytes);
    
        MidiGit->m_midiOut->sendMessage(message ); // Pass thru to out port
    }
}

bool Guitar::init_rt_midi_in()
{
    std::string clientName = m_client_name + " In";
    m_midiIn = new RtMidiIn(RtMidi::UNSPECIFIED, clientName);

    // Check available ports.
    unsigned int nPorts = m_midiIn->getPortCount();

    if (nPorts == 0)
    {
        return false;
    }
    return true;
}

bool Guitar::init_rt_midi_out()
{  
    // RtMidiOut constructor
    std::string clientName = m_client_name + " Out";
    try
    {
        m_midiOut = new RtMidiOut(RtMidi::UNSPECIFIED, clientName);
    }
    catch ( RtMidiError &error )
    {
        error.printMessage();
        return false;
    }
    return true;
}

void Guitar::RtSendMidiNote(uint note, bool OnorOff)      // bool OnorOff true = ON, false = Off
{
    unsigned char velocity = m_note_on_velocity;
    m_message.clear();
    
    if(OnorOff)
    {
        m_message.push_back(EVENT_NOTE_ON + m_midi_out_channel);       // status note ON
    }else
    {
        m_message.push_back(EVENT_NOTE_OFF + m_midi_out_channel);       // status note Off
        velocity = NOTE_OFF_VELOCITY;
    }
    
    m_message.push_back(note);
    m_message.push_back(velocity);
    
    m_midiOut->sendMessage(&m_message);
}

void Guitar::RtSendProgramChange(uint a_change)
{
    m_message.clear();
    m_message.push_back(EVENT_PROGRAM_CHANGE + m_midi_out_channel);
    m_message.push_back(a_change);
    
    m_midiOut->sendMessage(&m_message);
}

#endif // RTMIDI_SUPPORT

#ifdef ALSA_SUPPORT
bool Guitar::init_Alsa()
{
    mHandle = 0;
    char portname[64];

    if (snd_seq_open(&mHandle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0)
    {
        fl_alert("Error opening ALSA sequencer.\n");
        return false;
    }

    snd_seq_set_client_name(mHandle, m_client_name.c_str());

    sprintf(portname, "midi_in");
    if ((in_port = snd_seq_create_simple_port(mHandle, portname,
                                              SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                              SND_SEQ_PORT_TYPE_APPLICATION)) < 0)
    {
        fl_alert("Error creating sequencer port.\n");
        return false;
    }

    sprintf(portname, "midi_out");
    if ((out_port = snd_seq_create_simple_port(mHandle, portname,
                                               SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
                                               SND_SEQ_PORT_TYPE_APPLICATION)) < 0)
    {
        fl_alert("Error creating sequencer port.\n");
        return false;
    }

    mPollMax = snd_seq_poll_descriptors_count(mHandle, POLLIN);
    mPollFds = (struct pollfd *) calloc(mPollMax, sizeof (struct pollfd));

    snd_seq_nonblock(mHandle, 1);
    return true;
}

void Guitar::alsaGetMidiMessages()
{
    /* For MIDI incoming messages */
    snd_seq_event_t *ev;
    do
    {
        snd_seq_event_input(mHandle, &ev);

        if (ev)
        {
            snd_seq_free_event(ev);

            if (ev->data.note.channel == m_midi_in_channel - 1 || m_midi_in_channel == 0 ||
                ev->data.control.channel == m_midi_in_channel - 1)
            {
                if (ev->type == SND_SEQ_EVENT_CONTROLLER &&
                    ev->data.control.param == m_guitar_string_param &&
                    m_bcontrol == true)
                {
                    if (ev->data.control.value >= 1 && ev->data.control.value <= 6) // must do or it gets sent to random key pointer
                    {
                        m_have_string_toggle = true;
                        m_last_fret = false;
                        stringToggle(ev->data.control.value - 1); // we use 0 to 5, but user is 1 to 6
                    }
                }

                if (ev->type == SND_SEQ_EVENT_NOTEON)
                    fretToggle(ev->data.note.note, true);

                if ((ev->type == SND_SEQ_EVENT_NOTEOFF))
                    fretToggle(ev->data.note.note, false);

                snd_seq_ev_set_subs(ev);
                snd_seq_ev_set_direct(ev);
                snd_seq_ev_set_source(ev, out_port);
                snd_seq_event_output_direct(mHandle, ev);

                snd_seq_drain_output(mHandle);
            }
            snd_seq_free_event(ev);

        }
    } while (ev);
}

void Guitar::alsaSendMidiNote(uint a_note, bool On_or_Off)
{
    snd_seq_ev_clear(&m_ev);
    if(On_or_Off)
        snd_seq_ev_set_noteon(&m_ev, m_midi_out_channel, a_note, m_note_on_velocity);
    else
        snd_seq_ev_set_noteoff(&m_ev, m_midi_out_channel, a_note, NOTE_OFF_VELOCITY);
    
    snd_seq_ev_set_source(&m_ev, out_port);
    snd_seq_ev_set_subs(&m_ev);
    snd_seq_ev_set_direct(&m_ev);
    snd_seq_event_output_direct(mHandle, &m_ev);
    snd_seq_drain_output(mHandle);
    
    snd_seq_ev_clear(&m_ev);
}

void Guitar::alsaSendProgramChange(uint a_change)
{
    snd_seq_ev_clear(&m_ev);
    
    snd_seq_ev_set_pgmchange(&m_ev,m_midi_out_channel,a_change);
    
    /* Send the program change to out port */
    snd_seq_ev_set_source(&m_ev, out_port);
    snd_seq_ev_set_subs(&m_ev);
    snd_seq_ev_set_direct(&m_ev);
    snd_seq_event_output_direct(mHandle, &m_ev);
    snd_seq_drain_output(mHandle);
    
    snd_seq_ev_clear(&m_ev);
}
#endif // ALSA_SUPPORT

#ifdef JACK_SUPPORT
    
bool Guitar::init_jack()
{
    // Initialize output ringbuffers  
    m_buffSize = jack_ringbuffer_create( JACK_RINGBUFFER_SIZE );
    m_buffMessage = jack_ringbuffer_create( JACK_RINGBUFFER_SIZE );
    
    if ((m_jack_client = jack_client_open (m_client_name.c_str(), JackNoStartServer, NULL)) == 0)
    {
        fl_alert("Jack server is not running");
        return false;
    }

    jack_set_process_callback (m_jack_client, process, this);

    jack_on_shutdown (m_jack_client, jack_shutdown, this);

    m_jack_midi_in_port = jack_port_register (m_jack_client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    m_jack_midi_out_port = jack_port_register (m_jack_client, "midi_out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);

    if (jack_activate (m_jack_client))
    {
        fl_alert("Cannot activate Jack client");
        return false;
    }
    return true;
}

int Guitar::process(jack_nframes_t nframes, void *arg)
{
    int i, count;
    Guitar *Gtr = ((Guitar*)arg);
    
    /* For midi incoming */
    jack_midi_event_t midievent;
    
    // Is port created?
    if ( Gtr->m_jack_midi_in_port == NULL ) return 0;   // we ignore out port as well!
    
    float *data = (float *)jack_port_get_buffer(Gtr->m_jack_midi_in_port, nframes);
    count = jack_midi_get_event_count(data);
    
    for (i = 0; i < count; i++)
    {
        jack_midi_event_get(&midievent, data, i);
        Gtr->JackPlayMidiGuitar(&midievent);
    }

    /* For midi outgoing */
    jack_midi_data_t *midiData;
    int space;
    
    // Is port created?
    if ( Gtr->m_jack_midi_out_port == NULL ) return 0;

    void *buffer = jack_port_get_buffer( Gtr->m_jack_midi_out_port, nframes );
    jack_midi_clear_buffer( buffer );

    while ( jack_ringbuffer_read_space( Gtr->m_buffSize ) > 0 )
    {
        jack_ringbuffer_read( Gtr->m_buffSize, (char *) &space, (size_t) sizeof(space) );
        midiData = jack_midi_event_reserve( buffer, 0, space );

        jack_ringbuffer_read( Gtr->m_buffMessage, (char *) midiData, (size_t) space );
    }
 
    return 0;
}


void Guitar::jack_shutdown(void *arg)
{
    printf("Jack shut down!\n");
}

void Guitar::JackPlayMidiGuitar(jack_midi_event_t *midievent)
{
    /* We only care about note on/off and CC which are 3 bytes */
    if(midievent->size != 3)
        return;
    
    /* events */
    unsigned char status = 0, channel = 0, parameter = 0, value = 0; 
    
    status = midievent->buffer[0];
    channel = (midievent->buffer[0]  & EVENT_CHANNEL);
    parameter = midievent->buffer[1];     // for notes = key, for CC = which one
    value = midievent->buffer[2];         // for notes = velocity, for CC = value sent
    
    /* For Notes & CC we compare with EVENT_CLEAR_CHAN_MASK because we do not 
     want to iterate through 16 different items do to the channel bit. So we
     mask it off to compare to the single generic note or CC.
     User is 1 to 16, we are 0 to 15 (= -1). Channel 0 (user) means all channels*/
    
    if(channel == m_midi_in_channel - 1 || m_midi_in_channel == 0)
    {
        if (((status & EVENT_CLEAR_CHAN_MASK) == EVENT_CONTROL_CHANGE) &&
            (parameter == m_guitar_string_param) &&
            (m_bcontrol == true))
        {
            if (value >= 1 && value <= 6)   // must do or it gets sent to random key pointer
            {
                stringToggle(value - 1);    // we use 0 to 5, but user is 1 to 6
            }
        }
              
        if ((status & EVENT_CLEAR_CHAN_MASK) == EVENT_NOTE_ON)
            fretToggle(parameter, true);

        if ((status & EVENT_CLEAR_CHAN_MASK) == EVENT_NOTE_OFF)
            fretToggle(parameter, false);
    }
}

void Guitar::JackSendMidiNote(uint note, bool On_or_Off)
{
    size_t size = 3;
    unsigned char velocity = m_note_on_velocity;
    int nBytes = static_cast<int>(size);

    if(On_or_Off)
    {
        m_jack_midi_data[0] = EVENT_NOTE_ON + m_midi_out_channel;
    }
    else
    {
        m_jack_midi_data[0] = EVENT_NOTE_OFF + m_midi_out_channel;
        velocity = NOTE_OFF_VELOCITY;
    }
    m_jack_midi_data[1] = note;
    m_jack_midi_data[2] = velocity;
    
    /* printf("midi_data[0] = %d: [1] = %d: [2] = %d\n", m_jack_midi_data[0], m_jack_midi_data[1],m_jack_midi_data[2]);*/
    
    // Write full message to buffer
    jack_ringbuffer_write( m_buffMessage, ( const char * ) m_jack_midi_data,
                           nBytes );
    jack_ringbuffer_write( m_buffSize, ( char * ) &nBytes, sizeof( nBytes ) );
}

void Guitar::JackSendProgramChange(uint a_change)
{
    size_t size = 2;
    int nBytes = static_cast<int>(size);
    
    m_jack_midi_data[0] = EVENT_PROGRAM_CHANGE + m_midi_out_channel;
    m_jack_midi_data[1] = a_change;
    
    // Write full message to buffer
    jack_ringbuffer_write( m_buffMessage, ( const char * ) m_jack_midi_data,
                           nBytes );
    jack_ringbuffer_write( m_buffSize, ( char * ) &nBytes, sizeof( nBytes ) );

}
#endif  // JACK_SUPPORT

float Guitar::fret_distance(int num_fret)
{
    return 20 - (20 / (pow(2.0, (num_fret / 12.0))));
}

void Guitar::marker(int x, int y)
{
    Fl_Text_Display* o = new Fl_Text_Display(x, y, 0, 0, ".");
    o->box(FL_UP_FRAME);
    o->labelfont(9);
    o->labelsize(60);
}

void Guitar::reset_all_controls()
{
    for (int i = 0; i < 6; i++)
        gtString[i]->value(0);
    for (int i = 0; i < 127; i++)
        fretToggle(i, false);

    m_have_string_toggle = false;
    
    for (int i = 0; i < 127; i++)
    {
#ifdef RTMIDI_SUPPORT
        RtSendMidiNote(i, false);
#endif
#ifdef ALSA_SUPPORT
        alsaSendMidiNote(i, false);
#endif
#ifdef JACK_SUPPORT
        JackSendMidiNote(i, false);
#endif
    }
}

void Guitar::Timeout(void)
{
    /* Fret mouse click or drag */
    triggerFretNotes();
    
#ifdef ALSA_SUPPORT
    /* Alsa midi incoming messages */
    alsaGetMidiMessages();
#endif

    Fl::repeat_timeout(0.005, Guitar::TimeoutStatic, this);
}

void Guitar::triggerFretNotes()
{
    /* For MIDI sending messages */
    if (Fl::event_button1() && Fl::event() == FL_DRAG)
    {
        if (m_last_focus_fret == NO_FRET)
        {
            Fret * f = get_drag_fret();
            if (f != NULL)
            {
                f->value(1);
                cb_fret_callback(f);
            }
        } else
        {
            if (Fl::event_inside(fret[m_last_focus_fret]))
            {
                return; // if still inside previous fret then don't change
            } else
            {
                fret[m_last_focus_fret]->value(0); // moved outside of fret so shut it off
                cb_fret_callback(fret[m_last_focus_fret]);
            }

            Fret *f = get_adjacent_fret(m_last_focus_fret); // find the new fret we moved to if any
            if (f != NULL) // we found a fret inside so use it
            {
                f->value(1);
                cb_fret_callback(f);
            } else
                m_last_focus_fret = NO_FRET; // did not find a fret (moved off fretboard) so clear last
        }
    }

    if (m_last_focus_fret != NO_FRET && !Fl::event_button1()) // if released left mouse button
    {
        fret[m_last_focus_fret]->value(0); // then shut off the last fret we played
        cb_fret_callback(fret[m_last_focus_fret]);
        m_last_focus_fret = NO_FRET;
    }                                   
}

void Guitar::stringToggle(int gString)
{
    cb_string_callback(gtString[gString]);
}

void Guitar::fretToggle(uint note, bool on_off)
{
    /* guitar grid is 0 - 150 */

    bool found_fret = false;

    for (int i = 0; i < 6; i++) // strings
    {
        for (int j = 0; j < 25; j++) // frets
        {
            static int I, J;
            if ((note + m_transpose) == m_note_array[i][j]) // did the note match the grid?
            {
                found_fret = on_off;
                // save the location
                I = i;
                J = j;

                /* do we want to calculate the fret location - or is it note on */
                if (!m_bcontrol || !on_off) // no so send all notes found - default
                {
                    //printf("Fret [%d]\n",(I*25) +J);
                    toggle_fret((I * 25) + J, on_off);

                    if (I >= 6 && J >= 24) // when we are done
                        return;
                } else // user supplied CC starting location string or use first found
                {
                    if (m_have_string_toggle) // user supplied CC for string
                    {
                        if (gtString[I]->value() == 1) // if the string is on and...
                        {
                            toggle_fret((I * 25) + J, on_off);
                            stringToggle(I); // shut off string after we get it
                            
                            m_have_string_toggle = false; // shut off flag for string toggle
                            m_last_used_fret = (I * 25) + J; // save the fret location
                            m_last_fret = true; // set this so we know to use it next time
                            return; // found it so leave
                        }
                    } else if (m_last_fret) // we have previous fret so use it for calculation
                    {
                        /* after all locations are found then falls through to end which calls calculate_closest_fret() */
                        storeFretLocation[I] = (I * 25) + J;
                        // printf("init store Fret %d\n",storeFretLocation[I]);
                    } else // we don't have a CC or last fret so this would be the first found
                    { // so use it by default
                        toggle_fret((I * 25) + J, on_off);
                        m_last_used_fret = (I * 25) + J; // save the fret location
                        m_last_fret = true; // set this so we know to use it
                        return;
                    }
                }
            }
        }
    }
    /* this gets triggered only when we use the storeFretLocation[] */
    if (found_fret && m_last_fret)
        toggle_fret(calculate_closest_fret(), on_off);
}

void Guitar::toggle_fret(int location, bool on_off)
{
    int string = location / 25;
    int nfret = location % 25;

    if (m_guitar_type == RH_MIRROR_GUITAR || m_guitar_type == LH_MIRROR_GUITAR)
        string = (5 - string) * 25;
    else
        string *= 25;

    fret[string + nfret]->value(on_off);

    if (on_off) // true is note ON, so display note text
    {
        fret[string + nfret]->copy_label(c_key_table_text[location]);
    } else // clear note text
    {
        std::string label = "";
        if (nfret == 0)
            label = "Open";

        fret[string + nfret]->copy_label(label.c_str());
    }
}

void Guitar::cb_transpose_callback(Fl_Spinner* o)
{
    m_transpose = o->value();
}

void Guitar::transpose_callback(Fl_Spinner* o, void* data)
{
    ((Guitar*) data)->cb_transpose_callback(o);
}

void Guitar::cb_reset_callback(void* Gptr )
{
    ((Guitar*)Gptr)->reset_all_controls();
}

void Guitar::reset_callback(Fl_Button* o, void* data)
{
    ((Guitar*) data)->cb_reset_callback(data);
}

void Guitar::cb_control_callback(Fl_Button *b)
{
    if (b->value() == 1)
    {
        m_bcontrol = false;
        m_last_fret = false;
        m_last_used_fret = NO_FRET;
    } else
    {
        m_bcontrol = true;
    }
}

void Guitar::control_callback(Fl_Button *b, void* data)
{
    ((Guitar*) data)->cb_control_callback(b);
}

/* Used for mouse press on fret location - sends midi note on/off based on fret */
void Guitar::cb_fret_callback(Fret* b)
{
    for (int i = 0; i < 150; i++)
    {
        if (b == fret[i])
        {
            m_last_focus_fret = i;
            int string = i / 25;
            int nfret = i % 25;
            int text_array = (string * 25) + nfret;

            if (m_guitar_type == RH_MIRROR_GUITAR || m_guitar_type == LH_MIRROR_GUITAR)
            {
                string = (5 - string);
                text_array = (string * 25) + nfret;
            }

            if (fret[i]->value() == 1) // if ON note - display text & set midi note on
            {
                fret[i]->copy_label(c_key_table_text[text_array]);
#ifdef ALSA_SUPPORT
                alsaSendMidiNote(m_note_array[string][nfret], true);
#endif    
#ifdef RTMIDI_SUPPORT
                RtSendMidiNote(m_note_array[string][nfret], true);
#endif
#ifdef JACK_SUPPORT
                JackSendMidiNote(m_note_array[string][nfret], true);
#endif                
            } else // note off - clear text & set midi note off
            {
                std::string label = "";
                if (nfret == 0)
                    label = "Open";

                fret[i]->copy_label(label.c_str());
#ifdef ALSA_SUPPORT
                alsaSendMidiNote(m_note_array[string][nfret], false);
#endif      
#ifdef RTMIDI_SUPPORT
                RtSendMidiNote(m_note_array[string][nfret], false);
#endif
                #ifdef JACK_SUPPORT
                JackSendMidiNote(m_note_array[string][nfret], false);
#endif
            }
            //printf("string = %d: fret = %d: note %u\n",string,nfret, m_note_array[string][nfret]);
            break;
        }
    }
}

Fret *Guitar::get_drag_fret()
{
    for (int i = 0; i < 150; i++)
    {
        if (Fl::event_inside(fret[i]))
        {
            m_last_focus_fret = i;
            return fret[i];
        }
    }
    return NULL;
}

/* Used for mouse press on fret location */
void Guitar::fret_callback(Fret* b, void* data)
{
    if (Fl::event() != FL_DRAG) // user pressed the fret
    {
        if(Fl::event_button1()) // only on button press, not release
            ((Guitar*) data)->cb_fret_callback(b);
    }
    else // dragging on the fret
        b->value(1); // need this since timeout code will take care of callback()
}

void Guitar::cb_in_channel_callback(Fl_Spinner* o)
{
    m_midi_in_channel = o->value();
}

void Guitar::in_channel_callback(Fl_Spinner* o, void* data)
{
    ((Guitar*) data)->cb_in_channel_callback(o);
}

void Guitar::cb_program_callback(Fl_Spinner* o)
{
#ifdef RTMIDI_SUPPORT
    RtSendProgramChange((uint) o->value());
#endif
#ifdef ALSA_SUPPORT
    alsaSendProgramChange((uint) o->value());
#endif
#ifdef JACK_SUPPORT
    JackSendProgramChange((uint) o->value());
#endif
}

void Guitar::program_callback(Fl_Spinner* o, void* data)
{
    ((Guitar*) data)->cb_program_callback(o);
}

void Guitar::cb_out_channel_callback(Fl_Spinner* o)
{
    m_midi_out_channel = int(o->value()) - 1;
}

void Guitar::out_channel_callback(Fl_Spinner* o, void* data)
{
    ((Guitar*) data)->cb_out_channel_callback(o);
}
    
void Guitar::cb_velocity_callback(Fl_Slider* o)
{
    m_note_on_velocity = (char) o->value();
}

void Guitar::velocity_callback(Fl_Slider* o, void* data)
{
    ((Guitar*) data)->cb_velocity_callback(o);
}


void Guitar::cb_string_callback(Fl_Button* o)
{
    m_have_string_toggle = true;
    m_last_fret = false;
    if (o->value() == 1)
    {
        o->value(0);
    }
    else
        o->value(1);
}

void Guitar::string_callback(Fl_Button* o, void* data)
{
    ((Guitar*) data)->cb_string_callback(o);
}


int Guitar::get_fret_center(uint x_or_y, uint h_or_w)
{
    return x_or_y + (h_or_w * .5);
}

int Guitar::calculate_closest_fret()
{
    int last_X = get_fret_center(fret[m_last_used_fret]->x(), fret[m_last_used_fret]->h());
    int last_Y = get_fret_center(fret[m_last_used_fret]->y(), fret[m_last_used_fret]->w());

    // printf("last_x %d: last_y %d\n",last_X,last_Y);

    int closest_fret = NO_FRET;
    int last_diff = NO_FRET;

    //printf("last_fret %d\n",m_last_used_fret);
    for (int i = 0; i < 6; i++)
    {
        // printf("storeFret %d: last_diff %d\n",storeFretLocation[i],last_diff);
        if (storeFretLocation[i] == NO_FRET)
            continue;

        if (closest_fret == NO_FRET)
            closest_fret = storeFretLocation[i];

        int X = get_fret_center(fret[storeFretLocation[i]]->x(), fret[storeFretLocation[i]]->h());
        int Y = get_fret_center(fret[storeFretLocation[i]]->y(), fret[storeFretLocation[i]]->w());

        // printf("X %d: Y %d\n",X,Y);

        int current_diff = abs(last_X - X) + abs(last_Y - Y);

        if (last_diff == NO_FRET)
        {
            last_diff = current_diff;
            closest_fret = storeFretLocation[i];
            continue;
        }

        last_diff = (last_diff < current_diff ? last_diff : current_diff);
        closest_fret = (last_diff < current_diff ? closest_fret : storeFretLocation[i]);
    }

    for (int i = 0; i < 6; i++)
        storeFretLocation[i] = NO_FRET; // clear the array

    m_last_fret = true;
    m_last_used_fret = closest_fret;

    // printf("closest_fret %d\n",closest_fret);

    return closest_fret;
}

/*           -26 | -25 | -24
 *            -1 |  X  |  +1
 *           +24 | +25 | +26
 */

Fret *Guitar::get_adjacent_fret(int last_fret)
{
    if (last_fret + 1 < 150)
    {
        if (Fl::event_inside(fret[last_fret + 1]))
        {
            return fret[last_fret + 1];
        }
    }
    if (last_fret - 1 > 0)
    {
        if (Fl::event_inside(fret[last_fret - 1]))
        {
            return fret[last_fret - 1];
        }
    }

    if (last_fret - 26 > 0)
    {
        if (Fl::event_inside(fret[last_fret - 26]))
        {
            return fret[last_fret - 26];
        }
    }

    if (last_fret + 26 < 150)
    {
        if (Fl::event_inside(fret[last_fret + 26]))
        {
            return fret[last_fret + 26];
        }
    }

    if (last_fret - 25 > 0)
    {
        if (Fl::event_inside(fret[last_fret - 25]))
        {
            return fret[last_fret - 25];
        }
    }

    if (last_fret + 25 < 150)
    {
        if (Fl::event_inside(fret[last_fret + 25]))
        {
            return fret[last_fret + 25];
        }
    }

    if (last_fret - 24 > 0)
    {
        if (Fl::event_inside(fret[last_fret - 24]))
        {
            return fret[last_fret - 24];
        }
    }

    if (last_fret + 24 < 150)
    {
        if (Fl::event_inside(fret[last_fret + 24]))
        {
            return fret[last_fret + 24];
        }
    }

    return NULL;
}