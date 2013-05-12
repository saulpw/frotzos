#include "frotzos.h"

f_setup_t f_setup;
extern char *story_name;

void os_process_arguments (int argc, char *argv[])
{
    story_name = "story";
//    graphics_filename = "gfx";
}

void 	os_beep (int volume) { NOTIMPL; }
void 	os_more_prompt (void) { NOTIMPL; }
int  	os_peek_colour (void) { NOTIMPL; return 0; }
int  	os_read_file_name (char *fn, const char *default_fn, int flag) { NOTIMPL; return 0; }
zchar	os_read_key (int timeout, int show_cursor) { NOTIMPL; return 0; }
zchar   os_read_line (int max, zchar *buf, int timeout, int width, int continued) { NOTIMPL; return 0;}

void 	os_prepare_sample (int number) { NOTIMPL; }
void 	os_start_sample (int number, int volume, int repeats, zword eos) { NOTIMPL; }
void 	os_stop_sample () { NOTIMPL; }
void 	os_finish_with_sample () { NOTIMPL; }

void os_restart_game (int stage) {}

int os_random_seed (void)
{
    return rdtsc();
}

void os_fatal(const char *s, ...)
{
    os_display_string(s);
    os_display_string("  HALT");
    halt();
}

int os_picture_data (int num, int *h, int *w)
{
    NOTIMPL;
    return 0;
}

void os_draw_picture (int num, int row, int col)
{
    NOTIMPL;
}

