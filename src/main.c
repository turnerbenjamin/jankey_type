#include "constants.h"
#include "err.h"
#include "jankey_type.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

void init_ncurses(Err **err);
void cleanup_ncurses(void);
void clean_up(Err **err, JankeyType **jt);

int main() {
    Err *err = NULL;
    JankeyType *jt = NULL;

    srand((unsigned int)time(NULL));

    init_ncurses(&err);
    if (err) {
        clean_up(&err, &jt);
        return EXIT_FAILURE;
    }

    jankey_type_init(&err, &jt);
    if (err) {
        clean_up(&err, &jt);
        return EXIT_FAILURE;
    }

    jankey_type_run(&err, jt, JANKEY_STATE_RUNNING_TEST);
    if (err) {
        clean_up(&err, &jt);
        return EXIT_FAILURE;
    }

    clean_up(&err, &jt);
    return EXIT_SUCCESS;
}

void clean_up(Err **err, JankeyType **jt) {
    cleanup_ncurses();

    if (err && *err) {
        err_print(*err, stderr);
        err_destroy(err);
    }
    if (jt && *jt) {
        jankey_type_destroy(jt);
    }
}

void init_ncurses(Err **err) {
    initscr();

    if (!has_colors()) {
        *err = ERR_MAKE("Colors not available for the terminal");
        return;
    }
    start_color();

    clear();
    refresh();
    cbreak();
    noecho();

    keypad(stdscr, TRUE);

    if (init_extended_pair(COLOR_PAIR_WHITE, COLOR_WHITE, COLOR_BLACK) == ERR) {
        *err = ERR_MAKE("Unable to initialise pair");
        return;
    }

    if (init_extended_pair(COLOR_PAIR_GREEN, COLOR_GREEN, COLOR_BLACK) == ERR) {
        *err = ERR_MAKE("Unable to initialise pair");
        return;
    }

    if (init_extended_pair(COLOR_PAIR_RED, COLOR_RED, COLOR_BLACK) == ERR) {
        *err = ERR_MAKE("Unable to initialise pair");
        return;
    }
}

void cleanup_ncurses(void) {
    clear();           // Clear the screen
    refresh();         // Make sure changes are displayed
    reset_prog_mode(); // Reset terminal to program mode
    endwin();          // End ncurses mode
}
