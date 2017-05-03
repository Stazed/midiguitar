#ifndef GUITAR_H
#define GUITAR_H


#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Text_Display.H>
#include <alsa/asoundlib.h>
#include <sstream>

#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

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

const char c_key_table_text[][5]=
{
    "E 64","F 65","# 66","G 67","# 68","A 69","# 70","B 71","C 72","# 73","D 74","# 75","E 76","F 77","# 78","G 79","# 80","A 81","# 82","B 83","C 84","# 85","D 86","# 87","E 88", // E high
    "B 59","C 60","# 61","D 62","# 63","E 64","F 65","# 66","G 67","# 68","A 69","# 70","B 71","C 72","# 73","D 74","# 75","E 76","F 77","# 78","G 79","# 80","A 81","# 82","B 83", // B
    "G 55","# 56","A 57","# 58","B 59","C 60","# 61","D 62","# 63","E 64","F 65","# 66","G 67","# 68","A 69","# 70","B 71","C 72","# 73","D 74","# 75","E 76","F 77","# 78","G 79", // G
    "D 50","# 51","E 52","F 53","# 54","G 55","# 56","A 57","# 58","B 59","C 60","# 61","D 62","# 63","E 64","F 65","# 66","G 67","# 68","A 69","# 70","B 71","C 72","# 73","D 74", // D
    "A 45","# 46","B 47","C 48","# 49","D 50","# 51","E 52","F 53","# 54","G 55","# 56","A 57","# 58","B 59","C 60","# 61","D 62","# 63","E 64","F 65","# 66","G 67","# 68","A 69", // A
    "E 40","F 41","# 42","G 43","# 44","A 45","# 46","B 47","C 48","# 49","D 50","# 51","E 52","F 53","# 54","G 55","# 56","A 57","# 58","B 59","C 60","# 61","D 62","# 63","E 64"  // E low
};

const uint guitarMidiNote[6][25]=
{
    {64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88}, // E high
    {59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83}, // B
    {55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79}, // G
    {50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74}, // D
    {45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69}, // A
    {40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64}, // E low
};

#if 0
const char c_key_reverse_table_text[][5]=
{
    "E 40","F 41","# 42","G 43","# 44","A 45","# 46","B 47","C 48","# 49","D 50","# 51","E 52","F 53","# 54","G 55","# 56","A 57","# 58","B 59","C 60","# 61","D 62","# 63","E 64",  // E low
    "A 45","# 46","B 47","C 48","# 49","D 50","# 51","E 52","F 53","# 54","G 55","# 56","A 57","# 58","B 59","C 60","# 61","D 62","# 63","E 64","F 65","# 66","G 67","# 68","A 69", // A
    "D 50","# 51","E 52","F 53","# 54","G 55","# 56","A 57","# 58","B 59","C 60","# 61","D 62","# 63","E 64","F 65","# 66","G 67","# 68","A 69","# 70","B 71","C 72","# 73","D 74", // D
    "G 55","# 56","A 57","# 58","B 59","C 60","# 61","D 62","# 63","E 64","F 65","# 66","G 67","# 68","A 69","# 70","B 71","C 72","# 73","D 74","# 75","E 76","F 77","# 78","G 79", // G
    "B 59","C 60","# 61","D 62","# 63","E 64","F 65","# 66","G 67","# 68","A 69","# 70","B 71","C 72","# 73","D 74","# 75","E 76","F 77","# 78","G 79","# 80","A 81","# 82","B 83", // B
    "E 64","F 65","# 66","G 67","# 68","A 69","# 70","B 71","C 72","# 73","D 74","# 75","E 76","F 77","# 78","G 79","# 80","A 81","# 82","B 83","C 84","# 85","D 86","# 87","E 88"  // E high
};

const uint guitarReverseNote[6][25]=
{
    {40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64}, // E low
    {45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69}, // A
    {50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74}, // D
    {55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79}, // G
    {59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83}, // B
    {64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88}, // E high
};
#endif

const uint c_global_pixel_scale = 61;
const uint c_global_fret_height = 20;

class Guitar:public Fl_Double_Window
{
private:

    Fl_Button *gtString[7];
    Fl_Button *fret[151];

    void cb_spin_callback(Fl_Spinner*);
    static void spin_callback(Fl_Spinner*, void*);

    void cb_reset_callback(Fl_Button*);
    static void reset_callback(Fl_Button*, void*);
    
    void cb_control_callback(Fl_Button*);
    static void control_callback(Fl_Button*, void*);
    
    void cb_fret_callback(Fl_Button*);
    static void fret_callback(Fl_Button*, void*);
    
    struct pollfd *mPollFds;
    int mPollMax, in_port, out_port;

    snd_seq_t* mHandle;         // handle and client for system notification events
    snd_seq_ev_note_t m_note;   // for sending from fret mouse press
    snd_seq_event_t m_ev;       // for sending from fret mouse press
    
    std::string m_client_name;
    
    uint m_note_array[6][25];
    int storeFretLocation[7];
    
    bool m_have_string_toggle;
    bool m_last_fret;
    bool m_bReset;
    bool m_bcontrol;
    uint m_guitar_type;
    uint m_guitar_string_param;
    int  m_octave;
    int  m_last_used_fret;
    char  m_midi_channel;

public:
    Guitar(uint a_type, uint a_CC, std::string name, uint a_channel);
    virtual ~Guitar();

    float fret_distance(int num_fret);

    void marker(int x,int y);

    snd_seq_t* GetHandle(void)
    {
        return mHandle;
    }

    static void TimeoutStatic(void* ptr)
    {
        ((Guitar*)ptr)->Timeout();
    }

    void Timeout(void);

    void stringToggle(int gString);

    void fretToggle(uint note,bool on_off);
    void toggle_fret(int location, bool on_off);

    void reset_all_controls(void);
    
    int get_fret_center(uint x_or_y, uint h_or_w);
    int calculate_closest_fret();

};


#endif // GUITAR_H
