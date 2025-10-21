#include "round_end_view.h"
#include "../helpers.h"
#include "ncurses.h"
#include "string.h"

struct RoundEndView {
    WINDOW *win;
};

void ttv_roundend_init(Err **err, RoundEndView **view) {
    RoundEndView *v = ZALLOC(sizeof(*v));
    if (!v) {
        *err = ERR_MAKE("Unable to allocate memory for round end view");
        return;
    }

    int h = 7;
    int w = 50;
    int x = (int)((COLS - w) / 2);
    int y = 1;
    v->win = newwin(h, w, y, x);
    if (!v->win) {
        *err = ERR_MAKE("Unable to initialise ncurses window");
        ttv_roundend_destroy(&v);
        return;
    }

    *view = v;
}

void ttv_roundend_render(Err **err, RoundEndView *view) {
    curs_set(0);
    if (*err) {
        return;
    }

    box(view->win, 0, 0);

    const char *instructions = " [N]ew    [R]esume    [Q]uit ";
    wmove(view->win, 2, 2);
    waddstr(view->win, "TIME            65s");
    wmove(view->win, 3, 2);
    waddstr(view->win, "WPM:            15.6");
    wmove(view->win, 4, 2);
    waddstr(view->win, "ACCURACY:       95%");

    wmove(view->win, 6, (int)((50 - strlen(instructions)) / 2));
    waddstr(view->win, instructions);

    wrefresh(view->win);
}

void ttv_roundend_destroy(RoundEndView **view) {
    if (!view || !*view) {
        return;
    }
    RoundEndView *v = *view;
    if (v->win) {
        delwin(v->win);
        v->win = NULL;
    }

    free(v);
    v = NULL;
    *view = NULL;
}
