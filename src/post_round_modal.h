#ifndef POST_ROUND_MODAL_H
#define POST_ROUND_MODAL_H

#include "constants.h"
#include "err.h"

typedef struct PostRoundModal PostRoundModal;

void post_round_modal_init(Err **err, PostRoundModal **modal);
void post_round_modal_run(Err **err, JankeyState *state, PostRoundModal *modal);
void post_round_modal_destroy(PostRoundModal **view);

#endif
