#include "jankey_type.h"
#include "constants.h"
#include "helpers.h"
#include "post_round_modal.h"
#include "typing_test.h"
#include "typing_test_stats.h"
#include "word_store.h"

struct JankeyType {
    WordStore *word_store;
    TypingTest *typing_test;
    TypingTestStats *stats;
    PostRoundModal *post_round_modal;
};

void jankey_type_init(Err **err, JankeyType **jankey_type) {

    JankeyType *jt = ZALLOC(sizeof(*jt));
    if (!jt) {
        *err = ERR_MAKE("Unable to allocate memory for Jankey Type");
        return;
    }

    word_store_init(err, &jt->word_store, "dict/en_gb.txt");
    if (*err) {
        jankey_type_destroy(&jt);
        return;
    }

    typing_test_init(err, &jt->typing_test);
    if (*err) {
        jankey_type_destroy(&jt);
        return;
    }

    tt_stats_init(err, &jt->stats);
    if (*err) {
        jankey_type_destroy(&jt);
        return;
    }

    post_round_modal_init(err, &jt->post_round_modal);
    if (*err) {
        jankey_type_destroy(&jt);
        return;
    }

    *jankey_type = jt;
    return;
}

void jankey_type_run(Err **err, JankeyType *jankey_type,
                     JankeyState initial_state) {
    JankeyState state = initial_state;
    Err *e = NULL;
    while (state != JANKEY_STATE_QUITTING && !e) {
        switch (state) {
        case JANKEY_STATE_RUNNING_TEST:
            typing_test_run(&e, &state, jankey_type->typing_test,
                            jankey_type->word_store, WORDS_PER_TEST,
                            jankey_type->stats);
            break;
        case JANKEY_STATE_DISPLAYING_POST_TEST_MODAL:
            post_round_modal_run(&e, &state, jankey_type->post_round_modal,
                                 jankey_type->stats);
            break;
        case JANKEY_STATE_QUITTING:
            break;
        default:
            break;
        }
    }

    *err = e;
    return;
}

void jankey_type_destroy(JankeyType **jankey_type) {
    if (!jankey_type || !*jankey_type) {
        return;
    }

    JankeyType *jt = *jankey_type;
    if (jt->post_round_modal) {
        post_round_modal_destroy(&jt->post_round_modal);
    }
    if (jt->typing_test) {
        typing_test_destroy(&jt->typing_test);
    }
    if (jt->word_store) {
        word_store_destroy(&jt->word_store);
    }

    free(jt);
    jt = NULL;
    *jankey_type = NULL;
}
