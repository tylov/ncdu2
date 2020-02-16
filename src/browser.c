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

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <sys/stat.h>

static int graph = 3, info_show = 0, info_page = 0, info_start = 0, show_items = 1, show_mtime = 1;
static char *message = NULL;

extern int si;


static void browse_draw_info(struct dir *dr) {
  struct dir *t;
  struct dir *e = dr->flags & FF_EXT ? dr : NULL;
  char mbuf[46];
  int i;

  nccreate(11, 60, "Item info");

  if(dr->hlnk) {
    nctab(41, info_page == 0, 1, "Info");
    nctab(50, info_page == 1, 2, "Links");
  }

  switch(info_page) {
  case 0:
    attron(A_BOLD);
    ncaddstr(2, 3, "Name:");
    ncaddstr(3, 3, "Path:");
    if(!e)
      ncaddstr(4, 3, "Type:");
    else {
      ncaddstr(4, 3, "Mode:");
      ncaddstr(4, 21, "UID:");
      ncaddstr(4, 33, "GID:");
      ncaddstr(5, 3, "Last modified:");
    }
    ncaddstr(6, 3, "   Disk usage:");
    ncaddstr(7, 3, "Apparent size:");
    attroff(A_BOLD);

    ncaddstr(2,  9, cropstr(dr->name, 49));
    ncaddstr(3,  9, cropstr(getpath(dr->parent), 49));
    ncaddstr(4,  9, dr->flags & FF_DIR ? "Directory" : dr->flags & FF_FILE ? "File" : "Other");

    if(e) {
      ncaddstr(4, 9, fmtmode(e->mode));
      ncprint(4, 26, "%d", e->uid);
      ncprint(4, 38, "%d", e->gid);
      time_t t = (time_t)e->mtime;
      strftime(mbuf, sizeof(mbuf), "%Y-%m-%d %H:%M:%S %z", localtime(&t));
      ncaddstr(5, 18, mbuf);
    }

    ncmove(6, 18);
    printsize(UIC_DEFAULT, dr->size);
    addstrc(UIC_DEFAULT, " (");
    addstrc(UIC_NUM, fullsize(dr->size));
    addstrc(UIC_DEFAULT, " B)");

    ncmove(7, 18);
    printsize(UIC_DEFAULT, dr->asize);
    addstrc(UIC_DEFAULT, " (");
    addstrc(UIC_NUM, fullsize(dr->asize));
    addstrc(UIC_DEFAULT, " B)");
    break;

  case 1:
    for(i=0,t=dr->hlnk; t!=dr; t=t->hlnk,i++) {
      if(info_start > i)
        continue;
      if(i-info_start > 5)
        break;
      ncaddstr(2+i-info_start, 3, cropstr(getpath(t), 54));
    }
    if(t!=dr)
      ncaddstr(8, 25, "-- more --");
    break;
  }

  ncaddstr(9, 31, "Press ");
  addchc(UIC_KEY, 'i');
  addstrc(UIC_DEFAULT, " to hide this window");
}

static char get_draw_flag(struct dir *n) {
    return
      n == dirlist_parent ? ' ' :
        n->flags & FF_EXL ? '<' :
        n->flags & FF_ERR ? '!' :
       n->flags & FF_SERR ? '.' :
      n->flags & FF_OTHFS ? '>' :
      n->flags & FF_HLNKC ? 'H' :
     !(n->flags & FF_FILE
    || n->flags & FF_DIR) ? '@' :
        n->flags & FF_DIR
        && n->sub == NULL ? 'e' :
                            ' ' ;
}

static void browse_draw_flag(struct dir *n, int *x) {
  addchc(n->flags & FF_BSEL ? UIC_FLAG_SEL : UIC_FLAG, get_draw_flag(n));
  *x += 2;
}


static void browse_draw_graph(struct dir *n, int *x) {
  float pc = 0.0f;
  int o, i;
  enum ui_coltype c = n->flags & FF_BSEL ? UIC_SEL : UIC_DEFAULT;
  int show_as = dirlist_sort_col == DL_COL_ASIZE;

  if(!graph)
    return;

  *x += graph == 1 ? 13 : graph == 2 ? 9 : 20;
  if(n == dirlist_parent)
    return;

  addchc(c, '[');

  /* percentage (6 columns) */
  if(graph == 2 || graph == 3) {
    pc = (float)(show_as ? n->parent->asize : n->parent->size);
    if(pc < 1)
      pc = 1.0f;
    uic_set(c == UIC_SEL ? UIC_NUM_SEL : UIC_NUM);
    printw("%5.1f", ((float)(show_as ? n->asize : n->size) / pc) * 100.0f);
    addchc(c, '%');
  }

  if(graph == 3)
    addch(' ');

  /* graph (10 columns) */
  if(graph == 1 || graph == 3) {
    uic_set(c == UIC_SEL ? UIC_GRAPH_SEL : UIC_GRAPH);
    o = (int)(10.0f*(float)(show_as ? n->asize : n->size) / (float)(show_as ? dirlist_maxa : dirlist_maxs));
    for(i=0; i<10; i++)
      addch(i < o ? '#' : ' ');
  }

  addchc(c, ']');
}


static void get_draw_graph(struct dir *n, int* x, char *out) {
  float pc = 0.0f;
  int o, i;
  int show_as = dirlist_sort_col == DL_COL_ASIZE;
  char buf[64];
  int add = graph == 1 ? 13 : graph == 2 ? 9 : 20;
  sprintf(out, "%22s", " ");
  
  if(graph == 0) {
    return;
  }

  *x += add;

  if(n == dirlist_parent) {
    return;
  }

  *out++ = '[';

  /* percentage (6 columns) */
  if(graph == 2 || graph == 3) {
    pc = (float)(show_as ? n->parent->asize : n->parent->size);
    if(pc < 1)
      pc = 1.0f;
    sprintf(buf, "%5.1f%%", ((float)(show_as ? n->asize : n->size) / pc) * 100.0f);
    strncpy(out, buf, 6);
    out += 6;
  }

  if(graph == 3)
    *out++ = ' ';

  /* graph (10 columns) */
  if(graph == 1 || graph == 3) {
    o = (int)(10.0f*(float)(show_as ? n->asize : n->size) / (float)(show_as ? dirlist_maxa : dirlist_maxs));
    for(i=0; i<10; i++)
      *out++ = (i < o ? '#' : ' ');
  }
  strcpy(out, "]  ");
}


static void get_draw_count(struct dir *n, int *x, char* out) {
  if(!show_items) {
    *out = '\0';
    return;
  }
  
  *x += 8;
  if (n->items == 0) {
    sprintf(out, "          ");
  } else if (n->items < 1000*1000) {
    sprintf(out, "%6d  ", n->items);
  } else if (n->items < 100*1000*1000) {
    sprintf(out, "%5.2fM  ", n->items / 1e6);
  } else if (n->items < 1000*1000*1000) {
    sprintf(out, "%5.1fM  ", n->items / 1e6);
  } else {
    sprintf(out, "%5.2fB  ", n->items / 1e9);
  }
}



static void browse_draw_count(struct dir *n, int *x) {
  enum ui_coltype cn = n->flags & FF_BSEL ? UIC_NUM_SEL : UIC_NUM;
  char buf[32];
  
  if(!show_items)
    return;

  get_draw_count(n, x, buf);
  uic_set(cn);
  printw(buf);
}


static void get_draw_mtime(struct dir *n, int *x, char* out) {
  char mbuf[32] = "....-..-.. ..:..", mdbuf[32] =  "----------";
  char ubuf[32] = "-no-user", gbuf[32] = "-no-group";
  struct dir *e = NULL;
  time_t t;

  if (n->flags & FF_EXT) {
    e = n;
  } else if (!strcmp(n->name, "..") && (n->parent->flags & FF_EXT)) {
    e = n->parent;
  } 
  if (e) {
    t = (time_t) e->mtime;
    strftime(mbuf, sizeof(mbuf), "%Y-%m-%d %H:%M", localtime(&t));
    strcpy(mdbuf, fmtmode(e->mode));
    get_username(e->uid, ubuf, 9);
    get_groupname(e->gid, gbuf, 9);
  }
  sprintf(out, "%s  %s  %-9s %-9s   ", mbuf, mdbuf, ubuf, gbuf);
  *x += 50;
}

static void browse_draw_mtime(struct dir *n, int *x) {
  enum ui_coltype c = n->flags & FF_BSEL ? UIC_SEL : UIC_DEFAULT;
  char buf[64]; // 50 needed
  get_draw_mtime(n, x, buf);
  uic_set(c == UIC_SEL ? UIC_NUM_SEL : UIC_NUM);
  printw(buf);
}


static void browse_draw_item(struct dir *n, int row) {
  int x = 0;
  enum ui_coltype c = n->flags & FF_BSEL ? UIC_SEL : UIC_DEFAULT;
  int show_as = dirlist_sort_col == DL_COL_ASIZE;
  
  uic_set(c);
  mvhline(row, 0, ' ', wincols);
  move(row, 0);

  browse_draw_flag(n, &x);
  move(row, x);

  if(n != dirlist_parent) {
    printsize(c, show_as ? n->asize : n->size);
    if (show_as) printw("'");
  }
  x += 10;
  move(row, x);

  browse_draw_graph(n, &x);
  move(row, x);

  browse_draw_count(n, &x);
  move(row, x);

  if (extended_info && show_mtime) {
    browse_draw_mtime(n, &x);
    move(row, x);
  }

  if(n->flags & FF_DIR)
    c = c == UIC_SEL ? UIC_DIR_SEL : UIC_DIR;
  addchc(c, n->flags & FF_DIR ? '/' : ' ');
  addstrc(c, cropstr(n->name, wincols-x-1));
}

static void get_draw_item(struct dir *n, char *line) {
  int x = 0;
  int show_as = dirlist_sort_col == DL_COL_ASIZE;

  // flags
  sprintf(&line[x], "%c ", get_draw_flag(n));
  x += 2;

  sprintf(&line[x], "%s", cropstr(n->name, 255));

  // size
  if(n != dirlist_parent) {
    char* unit;
    float value = formatsize(show_as ? n->asize : n->size, &unit);
    sprintf(&line[x], "%5.1f %s%c  ", value, unit, show_as ? '\'' : ' ');
  } else {
    sprintf(&line[x], "              ");
  }
  x += 10;
  
  // graph
  get_draw_graph(n, &x, &line[x]);
  // item count
  get_draw_count(n, &x, &line[x]);
  // mod time
  if (extended_info && show_mtime) {
    get_draw_mtime(n, &x, &line[x]);
  }
  // dir slash
  sprintf(&line[x++], "%c", (n->flags & FF_DIR ? '/' : ' '));
  // file/dir name
  sprintf(&line[x], "%s", cropstr(n->name, 255));
}


void get_sortflags(char* out) {
  sprintf(out, "%c%c%c%c%c", 
          dirlist_sort_id == 1 ? 'u' : (dirlist_sort_id == 2 ? 'g' : '-'),
          dirlist_sort_df ? 'f' : '-', 
          dirlist_sort_col == DL_COL_ASIZE ? 'a' :
          dirlist_sort_col == DL_COL_SIZE ? 's' :
          dirlist_sort_col == DL_COL_ITEMS ? 'c' :
          dirlist_sort_col == DL_COL_NAME ? 'n' : 'm',
          dirlist_sort_desc ? '-' : '^',
          dirlist_hidden ? 'x' : '-');
}


void browse_draw() {
  struct dir *t;
  char *tmp;
  int selected = 0, i;
  char buf[32];

  erase();
  t = dirlist_get(0);

  /* top line - basic info */
  uic_set(UIC_HD);
  mvhline(0, 0, ' ', wincols);
  mvprintw(0,0,"%s %s ~ Use the arrow keys to navigate, press ", PACKAGE_NAME, PACKAGE_VERSION);
  addchc(UIC_KEY_HD, '?');
  addstrc(UIC_HD, " or ");
  addchc(UIC_KEY_HD, 'h');
  addstrc(UIC_HD, " for help, and ");
  addchc(UIC_KEY_HD, 'q');
  addstrc(UIC_HD, " to quit.");
  
  if(dir_import_active) {
    strftime(buf, sizeof(buf), "[imported %Y-%m-%d]", localtime(&dir_import_timestamp));
    mvaddstr(0, wincols-21, buf);
  } else if(read_only)
    mvaddstr(0, wincols-11, "[read-only]");

  /* second line - the path */
  mvhlinec(UIC_DEFAULT, 1, 0, '-', wincols);
  if(dirlist_par) {
    mvaddchc(UIC_DEFAULT, 1, 3, ' ');
    tmp = getpath(dirlist_par);
    mvaddstrc(UIC_DIR, 1, 4, cropstr(tmp, wincols-8));
    mvaddchc(UIC_DEFAULT, 1, 4+((int)strlen(tmp) > wincols-8 ? wincols-8 : (int)strlen(tmp)), ' ');
  }

  /* bottom line - stats */
  uic_set(UIC_HD);
  mvhline(winrows-1, 0, ' ', wincols);
  if(t) {
    mvaddstr(winrows-1, 1, "Disk usage:");
    printsize(UIC_HD, t->parent->size);
    addstrc(UIC_HD, "  Apparent size:");
    uic_set(UIC_NUM_HD);
    printsize(UIC_HD, t->parent->asize);
    addstrc(UIC_HD, "  Items:");
    uic_set(UIC_NUM_HD);
    printw(" %d", t->parent->items);
    addstrc(UIC_HD, "  Sort flags: ");
    uic_set(UIC_NUM_HD);
    get_sortflags(buf);
    printw(buf);
#ifdef USERSTATS
    get_username(getuid(), buf, 12);
    printw("  User %s:", buf);
    struct userdirstats *us = get_userdirstats(t->parent, getuid());
    if (us) {
      addstrc(UIC_HD, "  Disk usage:");
      uic_set(UIC_NUM_HD);
      printsize(UIC_HD, us->size);
      addstrc(UIC_HD, "  Items:");
      uic_set(UIC_NUM_HD);
      printw(" %d", us->items);
    } else {
      printw(" no files");
    }
#endif
  } else
    mvaddstr(winrows-1, 0, " No items to display.");
  uic_set(UIC_DEFAULT);

  /* nothing to display? stop here. */
  if(!t)
    return;

  /* get start position */
  t = dirlist_top(0);

  /* print the list to the screen */
  for(i=0; t && i<winrows-3; t=dirlist_next(t),i++) {
    browse_draw_item(t, 2+i);
    /* save the selected row number for later */
    if(t->flags & FF_BSEL)
      selected = i;
  }

  /* draw message window */
  if(message) {
    nccreate(6, 60, "Message");
    ncaddstr(2, 2, message);
    ncaddstr(4, 34, "Press any key to continue");
  }

  /* draw information window */
  t = dirlist_get(0);
  if(!message && info_show && t != dirlist_parent)
    browse_draw_info(t);

  /* move cursor to selected row for accessibility */
  move(selected+2, 0);
}

int compare_stats(const void *a, const void *b)
{
  struct userdirstats *stats_a = (struct userdirstats *) a,
                      *stats_b = (struct userdirstats *) b;
  // Dont take diff, because they are unsigned 64 bits.
  return stats_b->size < stats_a->size ? -1 : stats_b->size > stats_a->size ? 1 : 0;
  // Items:
  //return stats_a->items - stats_b->items;
  // Username:
  //char buf_a[64], buf_b[64];
  //get_username(stats_a->uid, buf_a, 63);
  //get_username(stats_b->uid, buf_b, 63);
  //return strcmp(buf_a, buf_b);
}

char* replace_char(char* str, char find, char replace) {
  char *current_pos = strchr(str,find);
  while (current_pos) {
    *current_pos = replace;
    current_pos = strchr(current_pos + 1, find);
  }
  return str;
}

void write_report(void)
{
    int i, n;
    char line[4096];
    char timebuf[32], path1[512], path2[512] = "", sflagsbuf[16];
    FILE* fp;
    time_t tm = time(NULL);
    struct dir *t = dirlist_get_head();
    float value;
    char* unit;
    struct stat sb;

    if (!t || !t->parent) {
      message = "No current dir";
      return;
    }

    if (dir_import_active) {
      tm = dir_import_timestamp;
    }
    sprintf(path1, "%s/.ncdu2", getenv("HOME"));
    if (stat(path1, &sb) == -1) {
      if (mkdir(path1, 0775) == -1) {
        message = "Cannot create $HOME/.ncdu2  folder";
        return;
      }
    }
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d", localtime(&tm));
    sprintf(path2, "%s/report-%s", path1, timebuf);
    if (stat(path2, &sb) == -1) {
      if (mkdir(path2, 0775) == -1) {
        message = "Cannot create report in $HOME/.ncdu2 folder";
        return;
      }
    }
    
    strcpy(line, getpath(t->parent));
    replace_char(line, '/', '.');
    get_sortflags(sflagsbuf);  
    sprintf(path1, "%s/report-%s%s#%c%c.txt", path2, timebuf, line, sflagsbuf[0], sflagsbuf[2]);

    fp = fopen(path1, "w");
    fprintf(fp, "NCDU2 disk usage report\n");
    fprintf(fp, "-----------------------\n");
    fprintf(fp, "       Directory : %s\n", getpath(t->parent));
    fprintf(fp, "            Date : %s%s\n", timebuf, (dir_import_active ? " [imported]" : ""));
    value = formatsize(t->parent->size, &unit);
    fprintf(fp, "      Disk usage : %6.2f %s\n", value, unit);
    value = formatsize(t->parent->asize, &unit);
    fprintf(fp, "   Apparent size : %6.2f %s\n", value, unit);
    fprintf(fp, "     Items count : %d\n", t->parent->items);
    fprintf(fp, "      Sort flags : %s\n\n", sflagsbuf);
#ifdef USERSTATS
    fprintf(fp, "Disk usage per user\n");
    fprintf(fp, "-------------------\n");
    n = cvec_size(t->parent->users);
    qsort(t->parent->users.data, n, sizeof(struct userdirstats), compare_stats);
    struct userdirstats *us = t->parent->users.data;
    for (i = 0; i < n; ++i, ++us) {
      get_username(us->uid, path1, 15);
      value = formatsize(us->size, &unit);
      fprintf(fp, "  %-15s: disk: %6.2f %s  items: %d\n", path1, value, unit, us->items);
    }
#endif
    /* print the list to a file */
    fprintf(fp, "\n");
    for(i = 0; t != NULL; t = dirlist_next(t), ++i) {
      get_draw_item(t, line);
      fprintf(fp, "%s\n", line);
    }
    fclose(fp);

    message = "Report saved under $HOME/.ncdu2/";
}


int browse_key(int ch) {
  struct dir *t, *sel;
  int i, catch = 0;

  /* message window overwrites all keys */
  if(message) {
    message = NULL;
    return 0;
  }

  sel = dirlist_get(0);

  /* info window overwrites a few keys */
  if(info_show && sel)
    switch(ch) {
    case '1':
      info_page = 0;
      break;
    case '2':
      if(sel->hlnk)
        info_page = 1;
      break;
    case KEY_RIGHT:
      if(sel->hlnk) {
        info_page = 1;
        catch++;
      }
      break;
    case KEY_LEFT:
      if(sel->hlnk) {
        info_page = 0;
        catch++;
      }
      break;
    case KEY_UP:
      if(sel->hlnk && info_page == 1) {
        if(info_start > 0)
          info_start--;
        catch++;
      }
      break;
    case KEY_DOWN:
      if(sel->hlnk && info_page == 1) {
        for(i=0,t=sel->hlnk; t!=sel; t=t->hlnk)
          i++;
        if(i > info_start+6)
          info_start++;
        catch++;
      }
      break;
    }

  if(!catch)
    switch(ch) {
    /* selecting items */
    case KEY_UP:
      dirlist_select(dirlist_get(-1));
      dirlist_top(-1);
      info_start = 0;
      break;
    case KEY_DOWN:
      dirlist_select(dirlist_get(1));
      dirlist_top(1);
      info_start = 0;
      break;
    case KEY_HOME:
      dirlist_select(dirlist_next(NULL));
      dirlist_top(2);
      info_start = 0;
      break;
    case KEY_LL:
    case KEY_END:
      dirlist_select(dirlist_get(1<<30));
      dirlist_top(1);
      info_start = 0;
      break;
    case KEY_PPAGE:
      dirlist_select(dirlist_get(-1*(winrows-3)));
      dirlist_top(-1);
      info_start = 0;
      break;
    case KEY_NPAGE:
      dirlist_select(dirlist_get(winrows-3));
      dirlist_top(1);
      info_start = 0;
      break;

    /* sorting items */
    case 'n': // name
      dirlist_set_sort(DL_COL_NAME, dirlist_sort_col == DL_COL_NAME ? !dirlist_sort_desc : 0, DL_NOCHANGE);
      info_show = 0;
      break;
    case 's': // dis[k] usage
      dirlist_set_sort(DL_COL_SIZE, dirlist_sort_col == DL_COL_SIZE ? !dirlist_sort_desc : 1, DL_NOCHANGE);
      info_show = 0;
      break;
    case 'a': // asize
      dirlist_set_sort(DL_COL_ASIZE, dirlist_sort_col == DL_COL_ASIZE ? !dirlist_sort_desc : 1, DL_NOCHANGE);
      info_show = 0;
      break;
    case 'c': // count
      dirlist_set_sort(DL_COL_ITEMS, dirlist_sort_col == DL_COL_ITEMS ? !dirlist_sort_desc : 1, DL_NOCHANGE);
      info_show = 0;
      break;
    case 'm': // time
      if (extended_info) {
        dirlist_set_sort(DL_COL_MTIME, dirlist_sort_col == DL_COL_MTIME ? !dirlist_sort_desc : 1, DL_NOCHANGE);
        info_show = 0;
      }
      break;
    case 'f': // folder first
      dirlist_set_sort(DL_NOCHANGE, DL_NOCHANGE, !dirlist_sort_df);
      info_show = 0;
      break;
    case 'u':
      if (extended_info) {
        dirlist_sort_id = (dirlist_sort_id != 1 ? 1 : 0);
        dirlist_set_sort(DL_NOCHANGE, DL_NOCHANGE, DL_NOCHANGE);
        info_show = 0;
      }
      break;
    case 'g':
      if (extended_info) {
        dirlist_sort_id = (dirlist_sort_id != 2 ? 2 : 0);
        dirlist_set_sort(DL_NOCHANGE, DL_NOCHANGE, DL_NOCHANGE);
        info_show = 0;
      }
      break;      
      
    /* browsing */
    case 10:
    case KEY_RIGHT:
      if(sel != NULL && sel->flags & FF_DIR) {
        dirlist_open(sel == dirlist_parent ? dirlist_par->parent : sel);
        dirlist_top(-3);
      }
      info_show = 0;
      break;
    case KEY_LEFT:
    case KEY_BACKSPACE:
      if(dirlist_par && dirlist_par->parent != NULL) {
        dirlist_open(dirlist_par->parent);
        dirlist_top(-3);
      }
      info_show = 0;
      break;

    /* and other stuff */
    case 'r':
      if(dir_import_active) {
        message = "Directory imported from file, won't refresh.";
        break;
      }
      if(dirlist_par) {
        dir_ui = 2;
        dir_mem_init(dirlist_par);
        dir_scan_init(getpath(dirlist_par));
      }
      info_show = 0;
      break;
    case 'q':
      if(info_show)
        info_show = 0;
      else
        if (confirm_quit)
          quit_init();
        else return 1;
      break;

    case '1': // view sizes in 10^3 vs 2^10 base
      si = !si;
      dirlist_set_sort(DL_NOCHANGE, DL_NOCHANGE, DL_NOCHANGE);
      info_show = 0;
      break;      
    case '2':
      if(++graph > 3)
        graph = 0;
      info_show = 0;
      break;
    case '3':
      show_items = !show_items;
      break;
    case '4':
      if (extended_info)
        show_mtime = !show_mtime;
      break;
  case 'x': // hidden
      dirlist_set_hidden(!dirlist_hidden);
      info_show = 0;
      break;
    case 'i':
      info_show = !info_show;
      break;
    case 'h':
    case '?':
      help_init();
      info_show = 0;
      break;
    case KEY_DC:
    case 'd':
      if(read_only >= 1 || dir_import_active) {
        message = read_only >= 1
          ? "File deletion disabled in read-only mode."
          : "File deletion not available for imported directories.";
        break;
      }
      if(sel == NULL || sel == dirlist_parent)
        break;
      info_show = 0;
      if((t = dirlist_get(1)) == sel)
        if((t = dirlist_get(-1)) == sel || t == dirlist_parent)
          t = NULL;
      delete_init(sel, t);
      break;
     case 'b':
      if(read_only >= 2 || dir_import_active) {
        message = read_only >= 2
          ? "Shell feature disabled in read-only mode."
          : "Shell feature not available for imported directories.";
        break;
      }
      shell_init();
      break;

    case 'p':
      write_report();
      break;
    }

  /* make sure the info_* options are correct */
  sel = dirlist_get(0);
  if(!info_show || sel == dirlist_parent)
    info_show = info_page = info_start = 0;
  else if(sel && !sel->hlnk)
    info_page = info_start = 0;

  return 0;
}


void browse_init(struct dir *par) {
  pstate = ST_BROWSE;
  message = NULL;
  dirlist_open(par);
}

