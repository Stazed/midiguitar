/*
  sliderW.cpp

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

 */

#include "sliderW.h"

SliderW::SliderW(int x, int y, int w, int h, const char *label) : Fl_Value_Slider(x, y, w, h, label)
{
}

void SliderW::draw()
{
    int sxx = x(), syy = y(), sww = w(), shh = h();
    int bxx = x(), byy = y(), bww = w(), bhh = h();
    
    /* This whole point of this override class is to allow the value display box
     * to resize. FLTK hard coded the width to 35 & height to 25. The original code:
     *  bww = 35;       bhh = 25;
     *  sxx += 35;      syy += 25;
     *  sww -= 35;      shh -= 25;
     * This is changed here to use a ratio of the total size ie. .25 for
     * width and .18 for height.
     * The vertical slider has not been tested because we are not using one here.
     */
    if (horizontal())
    {
        bww = w() * .25;
        sxx += bww;
        sww -= bww;
    }
    else
    {
        bhh = h() * .18;
        syy += bhh;
        shh -= bhh;
    }
    
    /* Box around the slider */
    if (damage() & FL_DAMAGE_ALL) draw_box(box(), sxx, syy, sww, shh, color());
    
    /* The slider itself */
    Fl_Slider::draw(sxx + Fl::box_dx(box()),
                    syy + Fl::box_dy(box()),
                    sww - Fl::box_dw(box()),
                    shh - Fl::box_dh(box()));
    
    /* Box around the value display*/
    draw_box(box(), bxx, byy, bww, bhh, color());
    
    
    char buf[128];
    format(buf);
    fl_font(textfont(), textsize());
    fl_color(active_r() ? textcolor() : fl_inactive(textcolor()));
    
    /* Text inside the value display */
    fl_draw(buf, bxx, byy, bww, bhh, FL_ALIGN_CLIP);
}
