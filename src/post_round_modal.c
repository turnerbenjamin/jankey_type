
#include "post_round_modal.h"
#include "constants.h"
#include "helpers.h"
#include "ncurses.h"
#include "stdlib.h"
#include "string.h"

struct PostRoundModal {
    WINDOW *win;
};

void post_round_modal_render(PostRoundModal *modal);

void post_round_modal_init(Err **err, PostRoundModal **modal) {
    PostRoundModal *m = ZALLOC(sizeof(*m));
    if (!m) {
        *err = ERR_MAKE("Unable to allocate memory for round end view");
        return;
    }

    int h = 7;
    int w = 50;
    int x = (int)((COLS - w) / 2);
    int y = (int)((LINES - h) / 2);
    m->win = newwin(h, w, y, x);
    if (!m->win) {
        *err = ERR_MAKE("Unable to initialise ncurses window");
        post_round_modal_destroy(&m);
        return;
    }

    *modal = m;
}

void post_round_modal_run(Err **err, JankeyState *state,
                          PostRoundModal *modal) {
    if (*err) {
        return;
    }
    post_round_modal_render(modal);
    while (true) {
        int ui = getch();
        if (ui < 0) {
            continue;
        }
        char c = (char)ui;
        switch (c) {
        case 'n':
        case 'N':
            *state = JANKEY_STATE_RUNNING_TEST;
            return;
        case 'q':
        case 'Q':
            *state = JANKEY_STATE_QUITTING;
            return;
        default:
            continue;
        }
    }

    clear();
    refresh();
}

void post_round_modal_render(PostRoundModal *modal) {
    curs_set(0);

    box(modal->win, 0, 0);

    const char *instructions = " [N]ew    [Q]uit ";
    wmove(modal->win, 2, 2);
    waddstr(modal->win, "TIME            65s");
    wmove(modal->win, 3, 2);
    waddstr(modal->win, "WPM:            15.6");
    wmove(modal->win, 4, 2);
    waddstr(modal->win, "ACCURACY:       95%");

    wmove(modal->win, 6, (int)((50 - strlen(instructions)) / 2));
    waddstr(modal->win, instructions);

    wrefresh(modal->win);
}

void post_round_modal_destroy(PostRoundModal **modal) {
    if (!modal || !*modal) {
        return;
    }
    PostRoundModal *m = *modal;
    if (m->win) {
        delwin(m->win);
        m->win = NULL;
    }

    free(m);
    m = NULL;
    *modal = NULL;
}
