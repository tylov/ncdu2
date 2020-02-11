/* ncdu - NCurses Disk Usage

  Copyright (c) 2007-2019 Yoran Heling

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "global.h"

#include <ncurses.h>
#include <string.h>


int page, start;


#define KEYS 22
char *keys[KEYS*2] = {
/*|----key----|  |----------------description----------------|*/
           "Up", "Move cursor up",
         "Down", "Move cursor down",
  "Enter/Right", "Open selected directory",
 "Backspc/Left", "Open parent directory",
            "n", "Sort by name (asc./desc.)",
            "s", "Sort by size on disk (asc./desc.)",
            "a", "Sort by apparent size (asc./desc.)",
            "c", "Sort by item count (asc./desc.)",
            "m", "Sort by mod. time (ext info)",
            "u", "Toggle sort user first (ext info)",
            "g", "Toggle sort group first (ext info)",
            "f", "Toggle folders first",
            "1", "Toggle 1024/1000 base size units",
            "2", "Toggle percentage and/or graph",
            "3", "Toggle display of child item counts",
            "4", "Toggle display of extended info",
            "x", "Toggle display of excluded/hidden files",
            "i", "Show information about selected item",
     "Delete/d", "Delete selected file or directory",
            "r", "Recalculate the current directory",
            "b", "Spawn shell in current directory",
            "q", "Quit ncdu2"
};


void help_draw() {
  int i, line;

  browse_draw();

  nccreate(15, 60, "ncdu2 help");
  ncaddstr(13, 42, "Press ");
  uic_set(UIC_KEY);
  addch('q');
  uic_set(UIC_DEFAULT);
  addstr(" to close");

  nctab(30, page == 1, 1, "Keys");
  nctab(39, page == 2, 2, "Format");
  nctab(50, page == 3, 3, "About");

  switch(page) {
    case 1:
      line = 1;
      for(i=start*2; i<start*2+20; i+=2) {
        uic_set(UIC_KEY);
        ncaddstr(++line, 13-strlen(keys[i]), keys[i]);
        uic_set(UIC_DEFAULT);
        ncaddstr(line, 15, keys[i+1]);
      }
      if(start != KEYS-10)
        ncaddstr(12, 25, "-- more --");
      break;
    case 2:
      attron(A_BOLD);
      ncaddstr(2, 3, "X  [size] [graph] [file or directory]");
      attroff(A_BOLD);
      ncaddstr(3, 4, "The X is only present in the following cases:");
      uic_set(UIC_FLAG);
      ncaddch( 5, 4, '!');
      ncaddch( 6, 4, '.');
      ncaddch( 7, 4, '<');
      ncaddch( 8, 4, '>');
      ncaddch( 9, 4, '@');
      ncaddch(10, 4, 'H');
      ncaddch(11, 4, 'e');
      uic_set(UIC_DEFAULT);
      ncaddstr( 5, 7, "An error occurred while reading this directory");
      ncaddstr( 6, 7, "An error occurred while reading a subdirectory");
      ncaddstr( 7, 7, "File or directory is excluded from the statistics");
      ncaddstr( 8, 7, "Directory was on another filesystem");
      ncaddstr( 9, 7, "This is not a file nor a dir (symlink, socket, ...)");
      ncaddstr(10, 7, "Same file was already counted (hard link)");
      ncaddstr(11, 7, "Empty directory");
      break;
    case 3:
      /* Indeed, too much spare time */
      attron(A_REVERSE);
#define x 12
#define y 3
      /* N */
      ncaddstr(y+0, x+0, "      ");
      ncaddstr(y+1, x+0, "  ");
      ncaddstr(y+2, x+0, "  ");
      ncaddstr(y+3, x+0, "  ");
      ncaddstr(y+4, x+0, "  ");
      ncaddstr(y+1, x+4, "  ");
      ncaddstr(y+2, x+4, "  ");
      ncaddstr(y+3, x+4, "  ");
      ncaddstr(y+4, x+4, "  ");
      /* C */
      ncaddstr(y+0, x+8, "     ");
      ncaddstr(y+1, x+8, "  ");
      ncaddstr(y+2, x+8, "  ");
      ncaddstr(y+3, x+8, "  ");
      ncaddstr(y+4, x+8, "     ");
      /* D */
      ncaddstr(y+0, x+19, "  ");
      ncaddstr(y+1, x+19, "  ");
      ncaddstr(y+2, x+15, "      ");
      ncaddstr(y+3, x+15, "  ");
      ncaddstr(y+3, x+19, "  ");
      ncaddstr(y+4, x+15, "      ");
      /* U */
      ncaddstr(y+0, x+23, "  ");
      ncaddstr(y+1, x+23, "  ");
      ncaddstr(y+2, x+23, "  ");
      ncaddstr(y+3, x+23, "  ");
      ncaddstr(y+0, x+27, "  ");
      ncaddstr(y+1, x+27, "  ");
      ncaddstr(y+2, x+27, "  ");
      ncaddstr(y+3, x+27, "  ");
      ncaddstr(y+4, x+23, "      ");
      attroff(A_REVERSE);
      ncaddstr(y+0, x+30, "NCurses");
      ncaddstr(y+1, x+30, "Disk");
      ncaddstr(y+2, x+30, "Usage");
      ncprint( y+4, x+30, "%s", PACKAGE_VERSION);
      ncaddstr( 9,  7, "Written by Yoran Heling <projects@yorhel.nl>");
      ncaddstr(10, 16, "https://dev.yorhel.nl/ncdu/");
      ncaddstr(11,  7, "Modified by Tyge Løvset <tylo@norceresearch.no>");
      break;
  }
}


int help_key(int ch) {
  switch(ch) {
    case '1':
    case '2':
    case '3':
      page = ch-'0';
      start = 0;
      break;
    case KEY_RIGHT:
    case KEY_NPAGE:
      if(++page > 3)
        page = 3;
      start = 0;
      break;
    case KEY_LEFT:
    case KEY_PPAGE:
      if(--page < 1)
        page = 1;
      start = 0;
      break;
    case KEY_DOWN:
    case ' ':
      if(start < KEYS-10)
        start++;
      break;
    case KEY_UP:
      if(start > 0)
        start--;
      break;
    default:
      pstate = ST_BROWSE;
  }
  return 0;
}


void help_init() {
  page = 1;
  start = 0;
  pstate = ST_HELP;
}


