#ifndef ROUND_END_VIEW
#define ROUND_END_VIEW

#include "../err.h"

typedef struct RoundEndView RoundEndView;

void ttv_roundend_init(Err **err, RoundEndView **view);
void ttv_roundend_render(Err **err, RoundEndView *view);
void ttv_roundend_destroy(RoundEndView **view);

#endif
