#include "frotzos.h"

f_setup_t f_setup;
extern char *story_name;

void os_init_setup()
{
    enable_interrupts();

    f_setup.attribute_assignment = 0;
	f_setup.attribute_testing = 0;
	f_setup.context_lines = 0;
	f_setup.object_locating = 0;
	f_setup.object_movement = 0;
	f_setup.left_margin = 0;
	f_setup.right_margin = 0;
	f_setup.ignore_errors = 0;
	f_setup.piracy = 0;		/* enable the piracy opcode */
	f_setup.undo_slots = MAX_UNDO_SLOTS;
	f_setup.expand_abbreviations = 0;
	f_setup.script_cols = 80;
	f_setup.save_quetzal = 1;
	f_setup.sound = 1;
	f_setup.err_report_mode = ERR_DEFAULT_REPORT_MODE;
}

void os_process_arguments (int argc, char *argv[])
{
    const char * const * files = enumfiles();
    story_name = (char *) files[1];
//    graphics_filename = "gfx";
}

void 	os_beep (int volume) { NOTIMPL; }

int  	os_peek_colour (void) { NOTIMPL; return 0; }

int os_read_file_name (char *fn, const char *default_fn, int flag)
{
    NOTIMPL; return 0;
}

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

void abort()
{
    os_fatal("abort()");
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

void 	os_prepare_sample (int number) { NOTIMPL; }
void 	os_start_sample (int number, int volume, int repeats, zword eos) { NOTIMPL; }
void 	os_stop_sample () { NOTIMPL; }
void 	os_finish_with_sample () { NOTIMPL; }

