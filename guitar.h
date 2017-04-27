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



uint const guitarMidiNote[6][25]=
{
    {64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88}, // E high
    {59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83}, // B
    {55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79}, // G
    {50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74}, // D
    {45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69}, // A
    {40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64}, // E low
};


uint const guitarReverseNote[6][25]=
{
    {40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64}, // E low
    {45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69}, // A
    {50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74}, // D
    {55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79}, // G
    {59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83}, // B
    {64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88}, // E high
};

extern uint global_guitar_type;

class Guitar:public Fl_Double_Window
{
private:

    Fl_Button *gtString[7];
    Fl_Button *fret[151];

    struct pollfd *mPollFds;
    int mPollMax, in_port, out_port;

    void cb_spin_callback(Fl_Spinner*);
    static void spin_callback(Fl_Spinner*, void*);

    void cb_reset_callback(Fl_Button*);
    static void reset_callback(Fl_Button*, void*);
    
    void cb_control_callback(Fl_Button*);
    static void control_callback(Fl_Button*, void*);

    snd_seq_t* mHandle; // handle and client for system notification events
    
    bool m_have_string_toggle;
    uint m_note_array[6][25];
    bool m_bReset;
    bool m_bcontrol;
    uint m_guitar_string_param;
    int  m_octave;

public:
    Guitar();
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
    
    int get_fret_center_x(uint x, uint h);
    int get_fret_center_y(uint y, uint w);
    

};


#endif // GUITAR_H
