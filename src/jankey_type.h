#ifndef JANKEY_TYPE_H
#define JANKEY_TYPE_H

#include "constants.h"
#include "err.h"

typedef struct JankeyType JankeyType;

void jankey_type_init(Err **err, JankeyType **jankey_type);
void jankey_type_run(Err **err, JankeyType *jankey_type,
                     JankeyState initialState);
void jankey_type_destroy(JankeyType **jankey_type);

#endif
