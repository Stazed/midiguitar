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


#ifndef GUITAR_H
#define GUITAR_H


#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Value_Slider.H>
#include <sstream>
#include "globals.h"

#ifndef CMAKE_BUILD
#include "config.h"
#endif

#ifdef ALSA_SUPPORT
#include <alsa/asoundlib.h>
#endif


#ifdef RTMIDI_SUPPORT
#include "RtMidi.h"
#endif  // RTMIDI_SUPPORT


#ifdef JACK_SUPPORT
#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>
#endif


/* This class is not needed - just a wrapper for Fl_Button*/
class Fret : public Fl_Button
{
public:
    Fret(int x, int y, int w, int h, const char *label = 0);
};

class Guitar : public Fl_Double_Window
{
private:
    
    Fl_Button       *m_gtr_string[7];
    Fret            *m_fret[151];
    Fl_Text_Display *m_fret_numbers[25];
    Fl_Text_Display *m_string_numbers[7];
    Fl_Button       *m_reset_button;
    Fl_Text_Display *m_marker[10];

    /* Midi in group */
    Fl_Group        *m_midi_in_group;
    Fl_Button       *m_control_button;
    Fl_Spinner      *m_transpose_spinner;
    Fl_Spinner      *m_channel_in_spinner;

    /* Midi out group */
    Fl_Group        *m_midi_out_group;
    Fl_Spinner      *m_program_change_spinner;
    Fl_Spinner      *m_channel_out_spinner;
    Fl_Slider       *m_velocity_slider;
    Fl_Value_Slider *m_ctrl_change_slider;
    Fl_Spinner      *m_CC_change_spinner;
    
    void cb_reset_callback(void* Gptr);
    static void reset_callback(Fl_Button*, void*);
    
    /* Midi In*/
    void cb_transpose_callback(Fl_Spinner*);
    static void transpose_callback(Fl_Spinner*, void*);

    void cb_control_callback(Fl_Button*);
    static void control_callback(Fl_Button*, void*);

    void cb_in_channel_callback(Fl_Spinner*);
    static void in_channel_callback(Fl_Spinner*, void*);
    
    /* Midi Out*/
    void cb_program_callback(Fl_Spinner*);
    static void program_callback(Fl_Spinner*, void*);

    void cb_out_channel_callback(Fl_Spinner*);
    static void out_channel_callback(Fl_Spinner*, void*);
    
    void cb_velocity_callback(Fl_Slider*);
    static void velocity_callback(Fl_Slider*, void*);
    
    void cb_ctrl_change_callback(Fl_Value_Slider*);
    static void ctrl_change_callback(Fl_Value_Slider*, void*);
    
    void cb_CC_number_callback(Fl_Spinner*);
    static void CC_number_callback(Fl_Spinner*, void*);
    
    void cb_fret_callback(Fret*);
    static void fret_callback(Fret*, void*);
    
    void cb_string_callback(Fl_Button*);
    static void string_callback(Fl_Button*, void*);
    
#ifdef RTMIDI_SUPPORT
    RtMidiIn    *m_midiIn;
    RtMidiOut   *m_midiOut;
    std::vector<unsigned char> m_message;
    
    bool init_rt_midi();
    bool init_rt_midi_in();
    bool init_rt_midi_out();
    void RtplayMidiGuitar(std::vector< unsigned char > *message, unsigned int nBytes);  // Midi In
    static void RtMidiCallback(double deltatime, std::vector< unsigned char > *message, void *userData);
    void RtSendMidiNote(uint note, bool OnorOff);    // bool OnorOff true = ON, false = Off
    void RtSendProgramChange(uint a_change);
    void RtSendCtrlChange(uint a_CC);
#endif    

#ifdef ALSA_SUPPORT
    int in_port, out_port;
    snd_seq_t* GetHandle(void) {return mHandle;}    /* not used */

    snd_seq_t* mHandle;             // handle and client for system notification events
    snd_seq_event_t m_ev;           // for sending from fret mouse press & program change
    bool init_Alsa();               // set up alsa
    void alsaGetMidiMessages();     // used in Timeout() for polling alsa Midi In
    void alsaSendMidiNote(uint a_note, bool On_or_Off);
    void alsaSendProgramChange(uint a_change);
    void alsaSendCtrlChange(uint a_CC);
#endif
    
#ifdef JACK_SUPPORT
    jack_port_t         *m_jack_midi_out_port;
    jack_port_t         *m_jack_midi_in_port;
    jack_client_t       *m_jack_client;
    jack_ringbuffer_t   *m_buffSize;
    jack_ringbuffer_t   *m_buffMessage;
    
    bool init_jack();
    static int process(jack_nframes_t nframes, void *arg);
    static void jack_shutdown(void *arg);
    void JackPlayMidiGuitar(jack_midi_event_t *midievent);
    void JackSendMidiNote(uint note, bool On_or_Off);
    void JackSendProgramChange(uint a_change);
    void JackSendCtrlChange(uint a_CC);
    void JackSendMessage(jack_midi_data_t *a_message, size_t size);
#endif

    std::string     m_windowLabel;
    std::string     m_client_name;

    uint m_note_array[6][25];
    int storeFretLocation[7];

    bool m_have_string_toggle;
    bool m_last_fret;
    bool m_bcontrol;
    uint m_guitar_type;
    uint m_guitar_string_param;
    int m_transpose;
    int m_last_used_fret;
    int m_last_focus_fret;
    char m_midi_out_channel;
    char m_midi_in_channel;
    char m_note_on_velocity;
    char m_midi_CC_number;
    int m_window_size_h;
    bool m_midi_numbers;

public:
    Guitar(uint a_type, uint a_CC, std::string name, uint a_channel, bool midi_numbers);
    virtual ~Guitar();

    float fret_distance(int num_fret);
    void marker(int x, int y, int num);
    
    static void TimeoutStatic(void* ptr)
    {
        ((Guitar*) ptr)->Timeout();
    }

    void Timeout(void);
    void triggerFretNotes();
    void adjust_label_sizes();

    void stringToggle(int gString);

    void fretToggle(uint note, bool on_off);
    void toggle_fret(int location, bool on_off);

    void reset_all_controls(void);

    int get_fret_center(uint x_or_y, uint h_or_w);
    int calculate_closest_fret();

    Fret *get_adjacent_fret(int last_fret);
    Fret *get_drag_fret();
};


#endif // GUITAR_H
