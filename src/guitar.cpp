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

Guitar::Guitar(uint a_type, uint a_CC, std::string name, uint a_channel, bool midi_numbers) :
    Fl_Double_Window(1020, c_global_min_window_h, "Midi Guitar Player"),
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
    m_note_on_velocity(NOTE_ON_VELOCITY),
    m_midi_CC_number(0),
    m_window_size_h(c_global_min_window_h),
    m_midi_numbers(midi_numbers)
#ifdef RTMIDI_SUPPORT
    , m_midiIn(0)
    , m_midiOut(0)
#endif
#ifdef JACK_SUPPORT
    , m_jack_midi_in_port(NULL)
    , m_jack_midi_out_port(NULL)
#endif

{
    {
        m_reset_button = new Fl_Button(50, 15, 70, 45, "Reset");
        m_reset_button->tooltip("Press button to clear all CC values and previous note calculation.\n"
                "Also, note OFF will be sent to all fret locations.");
        m_reset_button->color(FL_DARK_RED);
        m_reset_button->selection_color((Fl_Color) 135);
        m_reset_button->labelcolor(FL_WHITE);
        m_reset_button->labelsize(c_global_min_label_size);
        m_reset_button->callback((Fl_Callback*) reset_callback, this);
    } // Fl_Button* o

    {
        m_midi_in_group = new Fl_Group(145, 13, 250, 51, "Midi In");
        m_midi_in_group->labelsize(c_global_min_group_label_size);
        m_midi_in_group->box(FL_BORDER_BOX);
        {
            m_control_button = new Fl_Button(150, 15, 70, 45, "Control\n On/Off");
            m_control_button->type(1);
            m_control_button->tooltip("Press to stop calculation of nearest fret.\n"
                    "If pressed all possible note locations will be triggered.");
            m_control_button->color(FL_GREEN);
            m_control_button->selection_color(FL_FOREGROUND_COLOR);
            m_control_button->labelsize(c_global_min_label_size);
            m_control_button->callback((Fl_Callback*) control_callback, this);
        } // Fl_Button* m_control_button
        {
            m_transpose_spinner = new Fl_Spinner(250, 30, 40, 25, "Transpose");
            m_transpose_spinner->minimum(-24);
            m_transpose_spinner->maximum(24);
            m_transpose_spinner->tooltip("Selected value will adjust incoming midi note up or down.");
            m_transpose_spinner->value(m_transpose);
            m_transpose_spinner->align(Fl_Align(FL_ALIGN_TOP));
            m_transpose_spinner->labelsize(c_global_min_label_size);
            m_transpose_spinner->callback((Fl_Callback*) transpose_callback, this);
        } // Fl_Spinner* m_transpose_spinner
        {
            m_channel_in_spinner = new Fl_Spinner(330, 30, 40, 25, "Channel");
            m_channel_in_spinner->minimum(0);
            m_channel_in_spinner->maximum(16);
            m_channel_in_spinner->tooltip("Enter the midi channel to receive input.\n"
                    "Zero '0' means all channels.");
            m_channel_in_spinner->value(m_midi_in_channel);
            m_channel_in_spinner->align(Fl_Align(FL_ALIGN_TOP));
            m_channel_in_spinner->labelsize(c_global_min_label_size);
            m_channel_in_spinner->callback((Fl_Callback*) in_channel_callback, this);
        } // Fl_Spinner* m_channel_in_spinner
        m_midi_in_group->end(); // Must remember to do this or everything after group declaration is included!
        Fl_Group::current()->resizable(m_midi_in_group);
    }

    {
        m_midi_out_group = new Fl_Group(420, 13, 580, 51, "Midi Out");
        m_midi_out_group->labelsize(c_global_min_group_label_size);
        m_midi_out_group->box(FL_BORDER_BOX);
        {
            m_program_change_spinner = new Fl_Spinner(435, 30, 50, 25, "Program");
            m_program_change_spinner->minimum(0);
            m_program_change_spinner->maximum(127);
            m_program_change_spinner->tooltip("Set program change");
            m_program_change_spinner->value(0);
            m_program_change_spinner->align(Fl_Align(FL_ALIGN_TOP));
            m_program_change_spinner->labelsize(c_global_min_label_size);
            m_program_change_spinner->textsize(c_global_min_spin_text_size);
            m_program_change_spinner->callback((Fl_Callback*) program_callback, this);
        } // Fl_Spinner* m_program_change_spinner
        {
            m_channel_out_spinner = new Fl_Spinner(520, 30, 40, 25, "Channel");
            m_channel_out_spinner->minimum(1);
            m_channel_out_spinner->maximum(16);
            m_channel_out_spinner->tooltip("Enter/select the midi channel to send output");
            m_channel_out_spinner->value(m_midi_out_channel + 1);
            m_channel_out_spinner->align(Fl_Align(FL_ALIGN_TOP));
            m_channel_out_spinner->labelsize(c_global_min_label_size);
            m_channel_out_spinner->textsize(c_global_min_spin_text_size);
            m_channel_out_spinner->callback((Fl_Callback*) out_channel_callback, this);
        } // Fl_Spinner* m_channel_out_spinner
        {
            m_velocity_slider = new Fl_Slider(600, 35, 145, 17, "Velocity");
            m_velocity_slider->align(Fl_Align(FL_ALIGN_TOP));
            m_velocity_slider->type(FL_HOR_NICE_SLIDER);
            m_velocity_slider->minimum(0);
            m_velocity_slider->maximum(127);
            m_velocity_slider->value((double) NOTE_ON_VELOCITY);
            m_velocity_slider->labelsize(c_global_min_label_size);
            m_velocity_slider->callback((Fl_Callback*) velocity_callback, this);
        } // Fl_Slider* o
        {
            m_ctrl_change_slider = new Fl_Value_Slider(785, 35, 145, 17, "CC Value");
            m_ctrl_change_slider->align(Fl_Align(FL_ALIGN_TOP));
            m_ctrl_change_slider->type(FL_HOR_NICE_SLIDER);
            m_ctrl_change_slider->minimum(0);
            m_ctrl_change_slider->maximum(127);
            m_ctrl_change_slider->tooltip("Adjust Control Change Value");
            m_ctrl_change_slider->value((double) 0);
            m_ctrl_change_slider->precision(0);
            m_ctrl_change_slider->step(1);
            m_ctrl_change_slider->labelsize(c_global_min_label_size);
            m_ctrl_change_slider->textsize(c_global_min_spin_text_size);
            m_ctrl_change_slider->callback((Fl_Callback*) ctrl_change_callback, this);
        }
        {
            m_CC_change_spinner = new Fl_Spinner(940, 30, 50, 25, "CC Num");
            m_CC_change_spinner->minimum(0);
            m_CC_change_spinner->maximum(127);
            m_CC_change_spinner->tooltip("Set Control Change Number");
            m_CC_change_spinner->value(0);
            m_CC_change_spinner->align(Fl_Align(FL_ALIGN_TOP));
            m_CC_change_spinner->labelsize(c_global_min_label_size);
            m_CC_change_spinner->textsize(c_global_min_spin_text_size);
            m_CC_change_spinner->callback((Fl_Callback*) CC_number_callback, this);
        } // Fl_Spinner* m_program_change_spinner
        m_midi_out_group->end(); // Must remember to do this or everything after group declaration is included!
        Fl_Group::current()->resizable(m_midi_out_group);
    }

    /* Fret numbering */
    memset(&m_fret_numbers, 0, sizeof (m_fret_numbers));
    int n = 0;

    {
        m_fret_numbers[n] = new Fl_Text_Display(65, 90, 15, 15); // first OPEN string position
        m_fret_numbers[n]->box(FL_NO_BOX);
        m_fret_numbers[n]->labelsize(c_global_min_number_label_size);
        m_fret_numbers[n]->copy_label(SSTR(n).c_str()); // position 0
        n++;
    }
    for (int x = 0; x < 24; x++) // the numbered fret positions
    {
        float distance1 = Guitar::fret_distance(x);
        float distance2 = Guitar::fret_distance(x + 1);
        float X = distance1 + ((distance2 - distance1) / 2);

        m_fret_numbers[n] = new Fl_Text_Display((X * 60.4) + 95, 90, 15, 15);
        m_fret_numbers[n]->box(FL_NO_BOX);
        m_fret_numbers[n]->labelsize(c_global_min_number_label_size);
        m_fret_numbers[n]->copy_label(SSTR(n).c_str()); // n = 1 to 24
        n++;
    }

    /* Guitar string toggle buttons */
    memset(&m_gtr_string, 0, sizeof (m_gtr_string));
    char note_string[] = "EBGDAE";
    int y = 98;
    for (int i = 0; i < 6; i++)
    {
        int j = i;
        if (m_guitar_type == RH_MIRROR_GUITAR || m_guitar_type == LH_MIRROR_GUITAR)
        {
            j = 5 - j;
        }
        m_gtr_string[j] = new Fl_Button(30, y, 15, 15);
        m_gtr_string[j]->color(FL_BLACK);
        char temp[2] = {};
        temp[0] = note_string[j];
        m_gtr_string[j]->copy_label(temp);
        m_gtr_string[j]->selection_color(FL_GREEN);
        m_gtr_string[j]->align(Fl_Align(FL_ALIGN_LEFT));
        m_gtr_string[j]->labelsize(c_global_min_label_size);
        m_gtr_string[j]->callback((Fl_Callback*) string_callback, this);
        y += c_global_fret_height;
    }

    /* String number labels*/
    memset(&m_string_numbers, 0, sizeof (m_string_numbers));
    y = 100;
    for (int i = 0; i < 6; i++)
    {
        m_string_numbers[i] = new Fl_Text_Display(17, y, 15, 15);
        m_string_numbers[i]->box(FL_NO_BOX);
        m_string_numbers[i]->labelsize(c_global_min_number_label_size);

        int label = i + 1;
        if (m_guitar_type == RH_MIRROR_GUITAR || m_guitar_type == LH_MIRROR_GUITAR)
        {
            label = 7 - label;
        }

        m_string_numbers[i]->copy_label(SSTR(label).c_str()); // i = 1 to 6
        m_string_numbers[i]->selection_color(FL_BLACK);
        m_string_numbers[i]->align(Fl_Align(FL_ALIGN_LEFT));
        y += c_global_fret_height;
    }

    n = 0; // reset and reuse

    /* Guitar Fret Board */
    memset(&m_fret, 0, sizeof (m_fret));
    Fl_Group* guitar_frets = new Fl_Group(47, c_global_fret_height + 72, 965, 126);
    guitar_frets->box(FL_BORDER_BOX);
    {
        for (int y = 0; y < 6; y++)
        {
            m_fret[n] = new Fret(50, (y + 1) * c_global_fret_height + (y >= 6 ? 12 * 40 : 75), 45, c_global_fret_height, "Open");
            m_fret[n]->color(FL_YELLOW);
            m_fret[n]->selection_color(FL_GREEN);
            m_fret[n]->when(FL_WHEN_CHANGED);
            m_fret[n]->align(FL_ALIGN_CLIP);
            m_fret[n]->labelsize(c_global_min_fret_label_size);
            m_fret[n]->callback((Fl_Callback*) fret_callback, this);
            guitar_frets->add(m_fret[n]);
            n++;
            for (int x = 0; x < 24; x++) // The actual frets
            {
                float distance1 = Guitar::fret_distance(x);
                float distance2 = Guitar::fret_distance(x + 1);

                float fret_W = distance2 - distance1;
                m_fret[n] = new Fret((distance1 * c_global_pixel_scale) + 95,
                        (y + 1) * c_global_fret_height + (y >= 6 ? 12 * 40 : 75),
                        fret_W*c_global_pixel_scale, c_global_fret_height);
                m_fret[n]->color((Fl_Color) 18);
                m_fret[n]->selection_color(FL_GREEN);
                m_fret[n]->when(FL_WHEN_CHANGED);
                m_fret[n]->align(FL_ALIGN_CLIP);
                m_fret[n]->labelsize(c_global_min_fret_label_size);
                m_fret[n]->callback((Fl_Callback*) fret_callback, this);
                guitar_frets->add(m_fret[n]);
                n++;
            }
        }
    }
    guitar_frets->end(); // Must remember to do this or everything after group declaration is included!

    memset(&m_marker, 0, sizeof (m_marker));
    marker(260, 250, 0);
    marker(372, 250, 1);
    marker(475, 250, 2);
    marker(571, 250, 3);
    marker(688, 250, 4);
    marker(688, 270, 5);
    marker(788, 250, 6);
    marker(844, 250, 7);
    marker(895, 250, 8);
    marker(941, 250, 9);

    this->end();
    this->size_range(510, 140, 0, 0, 0, 0, 1); // sets minimum & the 1 = scalable
    this->resizable(this);

    /* End Window */
    m_windowLabel += PACKAGE_VERSION;
    label(m_windowLabel.c_str());


    /* Load the Midi note numeric value into note array according to guitar type */
    memset(&m_note_array, 0, sizeof (m_note_array));
    if (m_guitar_type == RH_MIRROR_GUITAR || m_guitar_type == LH_MIRROR_GUITAR)
    {
        for (int i = 5; i >= 0; i--)
        {
            for (int j = 0; j < 25; j++)
            {
                m_note_array[i][j] = guitarMidiNote[i][j];
            }
        }
    }
    else
    {
        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 25; j++)
            {
                m_note_array[i][j] = guitarMidiNote[i][j];
            }
        }
    }

    for (int i = 0; i < 6; i++)
        storeFretLocation[i] = NO_FRET; // initialize the array

#ifdef ALSA_SUPPORT
    if (!init_Alsa())
    {
        exit(-1);
    }
#endif  // ALSA_SUPPORT

#ifdef RTMIDI_SUPPORT
    if (!init_rt_midi())
    {
        exit(-1);
    }
#endif

#ifdef JACK_SUPPORT
    if (!init_jack())
    {
        exit(-1);
    }
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
    jack_ringbuffer_free(m_buffSize);
    jack_ringbuffer_free(m_buffMessage);
    if (m_jack_client)
    {
        jack_client_close(m_jack_client);
    }
#endif
    //dtor
}

#ifdef RTMIDI_SUPPORT

bool Guitar::init_rt_midi()
{
    if (init_rt_midi_out())
    {
        m_midiOut->openVirtualPort("Output");
    }
    else
    {
        fl_alert("Error creating RtMidi out port.\n");
        return false;
    }

    if (init_rt_midi_in())
    {
        m_midiIn->openVirtualPort("Input");
        m_midiIn->setCallback(RtMidiCallback, (void*) this);
    }
    else
    {
        fl_alert("Error creating RtMidi in port.\n");
        return false;
    }

    return true;
}

void Guitar::RtplayMidiGuitar(std::vector< unsigned char > *message, unsigned int nBytes)
{
    /* We only care about note on/off and CC which are 3 bytes */
    if (nBytes != 3)
        return;

    /* events */
    unsigned char status = 0, channel = 0, parameter = 0, value = 0;

    status = message->at(0);
    channel = (message->at(0) & EVENT_CHANNEL);
    parameter = message->at(1); // for notes = key, for CC = which one
    value = message->at(2); // for notes = velocity, for CC = value sent

    /* For Notes & CC we compare with EVENT_CLEAR_CHAN_MASK because we do not 
     want to iterate through 16 different items do to the channel bit. So we
     mask it off to compare to the single generic note or CC.
     User is 1 to 16, we are 0 to 15 (= -1). Channel 0 (user) means all channels*/

    if (channel == m_midi_in_channel - 1 || m_midi_in_channel == 0)
    {
        if (((status & EVENT_CLEAR_CHAN_MASK) == EVENT_CONTROL_CHANGE) &&
                (parameter == m_guitar_string_param) &&
                (m_bcontrol == true))
        {
            if (value >= 1 && value <= 6) // must do or it gets sent to random key pointer
            {
                stringToggle(value - 1); // we use 0 to 5, but user is 1 to 6
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
    Guitar *MidiGit = (Guitar*) userData;

    unsigned int nBytes = message->size();
    if (nBytes)
    {
        MidiGit->RtplayMidiGuitar(message, nBytes);

        MidiGit->m_midiOut->sendMessage(message); // Pass thru to out port
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
    catch (RtMidiError &error)
    {
        error.printMessage();
        return false;
    }
    return true;
}

void Guitar::RtSendMidiNote(uint note, bool OnorOff) // bool OnorOff true = ON, false = Off
{
    unsigned char velocity = m_note_on_velocity;
    m_message.clear();

    if (OnorOff)
    {
        m_message.push_back(EVENT_NOTE_ON + m_midi_out_channel); // status note ON
    }
    else
    {
        m_message.push_back(EVENT_NOTE_OFF + m_midi_out_channel); // status note Off
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

void Guitar::RtSendCtrlChange(uint a_CC)
{
    unsigned char value = 0;
    m_message.clear();

    m_message.push_back(EVENT_CONTROL_CHANGE + m_midi_out_channel);
    m_message.push_back(m_midi_CC_number);
    m_message.push_back(a_CC); // value of CC

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
    }
    while (ev);
}

void Guitar::alsaSendMidiNote(uint a_note, bool On_or_Off)
{
    snd_seq_ev_clear(&m_ev);
    if (On_or_Off)
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

    snd_seq_ev_set_pgmchange(&m_ev, m_midi_out_channel, a_change);

    /* Send the program change to out port */
    snd_seq_ev_set_source(&m_ev, out_port);
    snd_seq_ev_set_subs(&m_ev);
    snd_seq_ev_set_direct(&m_ev);
    snd_seq_event_output_direct(mHandle, &m_ev);
    snd_seq_drain_output(mHandle);

    snd_seq_ev_clear(&m_ev);
}

void Guitar::alsaSendCtrlChange(uint a_CC)
{
    snd_seq_ev_clear(&m_ev);
    snd_seq_ev_set_controller(&m_ev, m_midi_out_channel, m_midi_CC_number, a_CC);

    /* Send the control change to out port */
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
    m_buffSize = jack_ringbuffer_create(JACK_RINGBUFFER_SIZE);
    m_buffMessage = jack_ringbuffer_create(JACK_RINGBUFFER_SIZE);

    if ((m_jack_client = jack_client_open(m_client_name.c_str(), JackNoStartServer, NULL)) == 0)
    {
        fl_alert("Jack server is not running");
        return false;
    }

    jack_set_process_callback(m_jack_client, process, this);

    jack_on_shutdown(m_jack_client, jack_shutdown, this);

    m_jack_midi_in_port = jack_port_register(m_jack_client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    m_jack_midi_out_port = jack_port_register(m_jack_client, "midi_out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);

    if (jack_activate(m_jack_client))
    {
        fl_alert("Cannot activate Jack client");
        return false;
    }
    return true;
}

int Guitar::process(jack_nframes_t nframes, void *arg)
{
    int i, count;
    Guitar *Gtr = ((Guitar*) arg);

    /* For midi incoming */
    jack_midi_event_t midievent;

    // Is port created?
    if (Gtr->m_jack_midi_in_port == NULL) return 0; // we ignore out port as well!

    float *data = (float *) jack_port_get_buffer(Gtr->m_jack_midi_in_port, nframes);
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
    if (Gtr->m_jack_midi_out_port == NULL) return 0;

    void *buffer = jack_port_get_buffer(Gtr->m_jack_midi_out_port, nframes);
    jack_midi_clear_buffer(buffer);

    while (jack_ringbuffer_read_space(Gtr->m_buffSize) > 0)
    {
        jack_ringbuffer_read(Gtr->m_buffSize, (char *) &space, (size_t) sizeof (space));
        midiData = jack_midi_event_reserve(buffer, 0, space);

        jack_ringbuffer_read(Gtr->m_buffMessage, (char *) midiData, (size_t) space);
    }

    return 0;
}

void Guitar::jack_shutdown(void *arg)
{
    printf("Jack shut down!\n");
}

void Guitar::JackPlayMidiGuitar(jack_midi_event_t *midievent)
{
    /* Write full message to buffer for pass thru   */
    JackSendMessage(midievent->buffer, midievent->size);

    /* Trigger guitar Fret Board    */
    /* We only care about note on/off and CC which are 3 bytes */
    if (midievent->size != 3)
        return;

    /* events */
    unsigned char status = 0, channel = 0, parameter = 0, value = 0;

    status = midievent->buffer[0];
    channel = (midievent->buffer[0] & EVENT_CHANNEL);
    parameter = midievent->buffer[1]; // for notes = key, for CC = which one
    value = midievent->buffer[2]; // for notes = velocity, for CC = value sent

    /* For Notes & CC we compare with EVENT_CLEAR_CHAN_MASK because we do not 
     want to iterate through 16 different items do to the channel bit. So we
     mask it off to compare to the single generic note or CC.
     User is 1 to 16, we are 0 to 15 (= -1). Channel 0 (user) means all channels*/

    if (channel == m_midi_in_channel - 1 || m_midi_in_channel == 0)
    {
        if (((status & EVENT_CLEAR_CHAN_MASK) == EVENT_CONTROL_CHANGE) &&
                (parameter == m_guitar_string_param) &&
                (m_bcontrol == true))
        {
            if (value >= 1 && value <= 6) // must do or it gets sent to random key pointer
            {
                stringToggle(value - 1); // we use 0 to 5, but user is 1 to 6
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
    const size_t size = 3;
    unsigned char velocity = m_note_on_velocity;
    int nBytes = static_cast<int> (size);
    jack_midi_data_t jack_midi_data[size];

    if (On_or_Off)
    {
        jack_midi_data[0] = EVENT_NOTE_ON + m_midi_out_channel;
    }
    else
    {
        jack_midi_data[0] = EVENT_NOTE_OFF + m_midi_out_channel;
        velocity = NOTE_OFF_VELOCITY;
    }
    jack_midi_data[1] = note;
    jack_midi_data[2] = velocity;

    // Write full message to buffer
    JackSendMessage(jack_midi_data, size);
}

void Guitar::JackSendProgramChange(uint a_change)
{
    const size_t size = 2;
    int nBytes = static_cast<int> (size);
    jack_midi_data_t jack_midi_data[size];

    jack_midi_data[0] = EVENT_PROGRAM_CHANGE + m_midi_out_channel;
    jack_midi_data[1] = a_change;

    // Write full message to buffer
    JackSendMessage(jack_midi_data, size);
}

void Guitar::JackSendCtrlChange(uint a_CC)
{
    const size_t size = 3;
    int nBytes = static_cast<int> (size);
    jack_midi_data_t jack_midi_data[size];

    jack_midi_data[0] = EVENT_CONTROL_CHANGE + m_midi_out_channel;
    jack_midi_data[1] = m_midi_CC_number;
    jack_midi_data[2] = a_CC;

    // Write full message to buffer
    JackSendMessage(jack_midi_data, size);

    /*    unsigned char value = 0;
        m_message.clear();

        m_message.push_back(EVENT_CONTROL_CHANGE + m_midi_out_channel);
        m_message.push_back(m_midi_CC_number);
        m_message.push_back(a_CC);      // value of CC

        m_midiOut->sendMessage(&m_message);*/
}

void Guitar::JackSendMessage(jack_midi_data_t *a_message, size_t size)
{
    int nBytes = static_cast<int> (size);

    jack_ringbuffer_write(m_buffMessage, (const char *) a_message,
            nBytes);
    jack_ringbuffer_write(m_buffSize, (char *) &nBytes, sizeof ( nBytes));
}

#endif  // JACK_SUPPORT

float Guitar::fret_distance(int num_fret)
{
    return 20 - (20 / (pow(2.0, (num_fret / 12.0))));
}

void Guitar::marker(int x, int y, int num)
{
    m_marker[num] = new Fl_Text_Display(x, y, 0, 0, ".");
    m_marker[num]->box(FL_UP_FRAME);
    m_marker[num]->labelfont(9);
    m_marker[num]->labelsize(c_global_min_marker_size);
}

void Guitar::reset_all_controls()
{
    for (int i = 0; i < 6; i++)
        m_gtr_string[i]->value(0);
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
    adjust_label_sizes();

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
        }
        else
        {
            if (Fl::event_inside(m_fret[m_last_focus_fret]))
            {
                return; // if still inside previous fret then don't change
            }
            else
            {
                m_fret[m_last_focus_fret]->value(0); // moved outside of fret so shut it off
                cb_fret_callback(m_fret[m_last_focus_fret]);
            }

            Fret *f = get_adjacent_fret(m_last_focus_fret); // find the new fret we moved to if any
            if (f != NULL) // we found a fret inside so use it
            {
                f->value(1);
                cb_fret_callback(f);
            }
            else
                m_last_focus_fret = NO_FRET; // did not find a fret (moved off fretboard) so clear last
        }
    }

    if (m_last_focus_fret != NO_FRET && !Fl::event_button1()) // if released left mouse button
    {
        m_fret[m_last_focus_fret]->value(0); // then shut off the last fret we played
        cb_fret_callback(m_fret[m_last_focus_fret]);
        m_last_focus_fret = NO_FRET;
    }
}

void Guitar::adjust_label_sizes()
{
    int window_h = this->h();
#if 0
    int label_size;

    /* Adjusting by increment works if you resize slowly so that an increment
     is not bypassed. The advantage is you do not need global class variables
     for the widgets */

    if (window_h > (m_window_size_h + c_global_label_resize_increment) ||
            window_h < (m_window_size_h - c_global_label_resize_increment))
    {
        int adjust = 0;
        if (window_h > (m_window_size_h + c_global_label_resize_increment))
        {
            adjust = 1;
        }

        if (window_h < (m_window_size_h + c_global_label_resize_increment))
        {
            adjust = -1;
        }

        for (int t = 0; t < this->children(); t++)
        {
            Fl_Widget *w = this->child(t);

            if (w->as_group())
            {
                Fl_Group *g = (Fl_Group *) w;
                label_size = g->labelsize();
                label_size += adjust;
                g->labelsize(label_size);

                for (int i = 0; i < g->children(); i++)
                {
                    Fl_Widget *c = g->child(i);
                    label_size = c->labelsize();
                    label_size += adjust;
                    c->labelsize(label_size);
                    if (c == m_transpose_spinner || c == m_channel_in_spinner ||
                            c == m_program_change_spinner || c == m_channel_out_spinner)
                    {
                        Fl_Spinner *s = (Fl_Spinner *) c;
                        label_size = s->textsize();
                        label_size += adjust;
                        s->textsize(label_size);
                    }
                }
            }
            else
            {
                label_size = w->labelsize();
                label_size += adjust;
                w->labelsize(label_size);
            }
        }
        m_window_size_h = window_h;
        Fl::redraw();
    }
#endif // 0

    /* This adjusts by widow resize ratio based on minimum default sizes.
     The advantage is that it works even if increment is skipped and maintains
     the relative size better. The disadvantage is that it requires all adjusted
     widgets to be identified by class global to use the minimum global setting
     to adjust by the ratio */

    if (window_h > (m_window_size_h + c_global_label_resize_increment) ||
            window_h < (m_window_size_h - c_global_label_resize_increment))
    {

        float ratio = ((float) window_h / c_global_min_window_h);

        m_reset_button->labelsize(c_global_min_label_size * ratio);
        m_midi_in_group->labelsize(c_global_min_group_label_size * ratio);
        m_control_button->labelsize(c_global_min_label_size * ratio);
        m_transpose_spinner->labelsize(c_global_min_label_size * ratio);
        m_transpose_spinner->textsize(c_global_min_spin_text_size * ratio);
        m_channel_in_spinner->labelsize(c_global_min_label_size * ratio);
        m_channel_in_spinner->textsize(c_global_min_spin_text_size * ratio);

        m_midi_out_group->labelsize(c_global_min_group_label_size * ratio);
        m_program_change_spinner->labelsize(c_global_min_label_size * ratio);
        m_program_change_spinner->textsize(c_global_min_spin_text_size * ratio);
        m_channel_out_spinner->labelsize(c_global_min_label_size * ratio);
        m_channel_out_spinner->textsize(c_global_min_spin_text_size * ratio);
        m_velocity_slider->labelsize(c_global_min_label_size * ratio);

        m_ctrl_change_slider->labelsize(c_global_min_label_size * ratio);
        m_ctrl_change_slider->textsize(c_global_min_spin_text_size * ratio);
        m_CC_change_spinner->labelsize(c_global_min_label_size * ratio);
        m_CC_change_spinner->textsize(c_global_min_spin_text_size * ratio);

        for (int i = 0; i < 25; ++i)
        {
            m_fret_numbers[i]->labelsize(c_global_min_number_label_size * ratio);
        }

        for (int i = 0; i < 6; i++)
        {
            m_gtr_string[i]->labelsize(c_global_min_label_size * ratio);
            m_string_numbers[i]->labelsize(c_global_min_number_label_size * ratio);
        }

        for (int i = 0; i < 150; ++i)
        {
            m_fret[i]->labelsize(c_global_min_fret_label_size * ratio);
        }

        for (int i = 0; i < 10; i++)
        {
            m_marker[i]->labelsize(c_global_min_marker_size * ratio);
        }

        m_window_size_h = window_h;
        Fl::redraw();
    }
}

void Guitar::stringToggle(int gString)
{
    Fl::lock();
    cb_string_callback(m_gtr_string[gString]);
    Fl::unlock();
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
                }
                else // user supplied CC starting location string or use first found
                {
                    if (m_have_string_toggle) // user supplied CC for string
                    {
                        if (m_gtr_string[I]->value() == 1) // if the string is on and...
                        {
                            toggle_fret((I * 25) + J, on_off);
                            stringToggle(I); // shut off string after we get it

                            m_have_string_toggle = false; // shut off flag for string toggle
                            m_last_used_fret = (I * 25) + J; // save the fret location
                            m_last_fret = true; // set this so we know to use it next time
                            return; // found it so leave
                        }
                    }
                    else if (m_last_fret) // we have previous fret so use it for calculation
                    {
                        /* after all locations are found then falls through to end which calls calculate_closest_fret() */
                        storeFretLocation[I] = (I * 25) + J;
                        // printf("init store Fret %d\n",storeFretLocation[I]);
                    }
                    else // we don't have a CC or last fret so this would be the first found
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
    {
        toggle_fret(calculate_closest_fret(), on_off);
    }
}

void Guitar::toggle_fret(int location, bool on_off)
{
    if (location < 0 || location > 150)
        return;

    Fl::lock();

    int string = location / 25;
    int nfret = location % 25;

    if (m_guitar_type == RH_MIRROR_GUITAR || m_guitar_type == LH_MIRROR_GUITAR)
        string = (5 - string) * 25;
    else
        string *= 25;

    m_fret[string + nfret]->value(on_off);

    if (on_off) // true is note ON, so display note text
    {
        if (m_midi_numbers)
            m_fret[string + nfret]->copy_label(c_key_table_number[location]);
        else
            m_fret[string + nfret]->copy_label(c_key_table_text[location]);
    }
    else // clear note text
    {
        std::string label = "";
        if (nfret == 0)
            label = "Open";

        m_fret[string + nfret]->copy_label(label.c_str());
    }
    Fl::unlock();
}

void Guitar::cb_transpose_callback(Fl_Spinner* o)
{
    m_transpose = o->value();
}

void Guitar::transpose_callback(Fl_Spinner* o, void* data)
{
    ((Guitar*) data)->cb_transpose_callback(o);
}

void Guitar::cb_reset_callback(void* Gptr)
{
    ((Guitar*) Gptr)->reset_all_controls();
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
    }
    else
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
        if (b == m_fret[i])
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

            if (m_fret[i]->value() == 1) // if ON note - display text & set midi note on
            {
                if (m_midi_numbers)
                    m_fret[i]->copy_label(c_key_table_number[text_array]);
                else
                    m_fret[i]->copy_label(c_key_table_text[text_array]);
#ifdef ALSA_SUPPORT
                alsaSendMidiNote(m_note_array[string][nfret], true);
#endif    
#ifdef RTMIDI_SUPPORT
                RtSendMidiNote(m_note_array[string][nfret], true);
#endif
#ifdef JACK_SUPPORT
                JackSendMidiNote(m_note_array[string][nfret], true);
#endif                
            }
            else // note off - clear text & set midi note off
            {
                std::string label = "";
                if (nfret == 0)
                    label = "Open";

                m_fret[i]->copy_label(label.c_str());
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
        if (Fl::event_inside(m_fret[i]))
        {
            m_last_focus_fret = i;
            return m_fret[i];
        }
    }
    return NULL;
}

/* Used for mouse press on fret location */
void Guitar::fret_callback(Fret* b, void* data)
{
    if (Fl::event() != FL_DRAG) // user pressed the fret
    {
        if (Fl::event_button1()) // only on button press, not release
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

void Guitar::cb_ctrl_change_callback(Fl_Value_Slider* o)
{
#ifdef RTMIDI_SUPPORT
    RtSendCtrlChange((uint) o->value());
#endif
#ifdef ALSA_SUPPORT
    alsaSendCtrlChange((uint) o->value());
#endif
#ifdef JACK_SUPPORT
    JackSendCtrlChange((uint) o->value());
#endif
}

void Guitar::ctrl_change_callback(Fl_Value_Slider* o, void* data)
{
    ((Guitar*) data)->cb_ctrl_change_callback(o);
}

void Guitar::cb_CC_number_callback(Fl_Spinner* o)
{
    m_midi_CC_number = o->value();
}

void Guitar::CC_number_callback(Fl_Spinner* o, void* data)
{
    ((Guitar*) data)->cb_CC_number_callback(o);
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
    if (m_last_used_fret == NO_FRET)
        return NO_FRET;

    int last_X = get_fret_center(m_fret[m_last_used_fret]->x(), m_fret[m_last_used_fret]->h());
    int last_Y = get_fret_center(m_fret[m_last_used_fret]->y(), m_fret[m_last_used_fret]->w());

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

        int X = get_fret_center(m_fret[storeFretLocation[i]]->x(), m_fret[storeFretLocation[i]]->h());
        int Y = get_fret_center(m_fret[storeFretLocation[i]]->y(), m_fret[storeFretLocation[i]]->w());

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

    if (closest_fret != NO_FRET)
    {
        m_last_fret = true;
        m_last_used_fret = closest_fret;
    }
    //     printf("closest_fret %d\n",closest_fret);

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
        if (Fl::event_inside(m_fret[last_fret + 1]))
        {
            return m_fret[last_fret + 1];
        }
    }
    if (last_fret - 1 > 0)
    {
        if (Fl::event_inside(m_fret[last_fret - 1]))
        {
            return m_fret[last_fret - 1];
        }
    }

    if (last_fret - 26 > 0)
    {
        if (Fl::event_inside(m_fret[last_fret - 26]))
        {
            return m_fret[last_fret - 26];
        }
    }

    if (last_fret + 26 < 150)
    {
        if (Fl::event_inside(m_fret[last_fret + 26]))
        {
            return m_fret[last_fret + 26];
        }
    }

    if (last_fret - 25 > 0)
    {
        if (Fl::event_inside(m_fret[last_fret - 25]))
        {
            return m_fret[last_fret - 25];
        }
    }

    if (last_fret + 25 < 150)
    {
        if (Fl::event_inside(m_fret[last_fret + 25]))
        {
            return m_fret[last_fret + 25];
        }
    }

    if (last_fret - 24 > 0)
    {
        if (Fl::event_inside(m_fret[last_fret - 24]))
        {
            return m_fret[last_fret - 24];
        }
    }

    if (last_fret + 24 < 150)
    {
        if (Fl::event_inside(m_fret[last_fret + 24]))
        {
            return m_fret[last_fret + 24];
        }
    }

    return NULL;
}