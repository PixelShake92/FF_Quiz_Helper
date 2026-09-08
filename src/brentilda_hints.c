#include "modding.h"

#include <ultra64.h>
#include "functions.h"
#include "variables.h"

// Brentilda tells ten sets of three facts, and Grunty asks about them in Furnace Fun.
// This records which facts have been heard and marks the matching answer with her head.

// 0xD4 to 0xDA are unused in memory. So utilise them. 
#define BRENTILDA_HEARD_BASE 0xD4
#define BRENTILDA_COUNT      10
#define BRENTILDA_FACTS      3

#define FFQT_3_GRUNTY            3
#define GCQUIZ_STATE_0_IDLE      0
#define ZOOMBOX_SPRITE_BRENTILDA 0x57

typedef struct {
    s8    question_type;
    s8    question_index;
    s8    unk2;
    u8    quiz_question_time;
    s32   unk4;
    void *unk8;
    void *unkC;
    u8    state;
    s8    unk11;
    s8    selected_answer_index;
    u8    answer_cursor_index;
    u8    unk14;
    u8    answer_cursor_cooldown;
    u8    unk16;
    u8    unk17;
    u8    answer_str_counts[4];
    s8    answer_values[4];
    s8    portrait_ids[4];
    void *zoomboxes[4];
} QuizState;

extern QuizState *sD_803830E0;

void gczoombox_loadSprite(void *zoombox, s32 sprite_id);
void gczoombox_loadSprite(void *zoombox, s32 sprite_id);
void gczoombox_draw(void *zoombox, Gfx **gfx, Mtx **mtx, Vtx **vtx);
void fileProgressFlag_set(enum file_progress_e index, s32 set);
bool fileProgressFlag_get(enum file_progress_e index);

bool brentilda_hint_applied = FALSE;

s32 brentilda_heard_flag(s32 brentilda, s32 fact) {
    return BRENTILDA_HEARD_BASE + (brentilda * BRENTILDA_FACTS) + fact;
}

// Brentilda's actorTypeSpecificField counts from 1, the flags from 0
RECOMP_HOOK("chBrentilda_update") void brentilda_record_fact(Actor *this) {
    s32 brentilda = this->actorTypeSpecificField - 1;

    if (brentilda < 0 || brentilda >= BRENTILDA_COUNT) {
        return;
    }
    if (this->unk10_12 >= BRENTILDA_FACTS) {
        return;
    }

    fileProgressFlag_set(brentilda_heard_flag(brentilda, this->unk10_12), TRUE);
}

// question_index runs across all thirty facts, three per Brentilda
RECOMP_PATCH void gcquiz_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx) {
    s32 i;

    if (sD_803830E0 == NULL) {
        return;
    }

    if (!brentilda_hint_applied
        && sD_803830E0->state >= 2
        && sD_803830E0->state <= 7
        && sD_803830E0->question_type == FFQT_3_GRUNTY) {
        s32 q = sD_803830E0->question_index;
        s32 brentilda = q / BRENTILDA_FACTS;
        s32 fact = q % BRENTILDA_FACTS;

        if (brentilda >= 0 && brentilda < BRENTILDA_COUNT
            && fileProgressFlag_get(brentilda_heard_flag(brentilda, fact))) {
            for (i = 1; i < 4; i++) {
                if (sD_803830E0->answer_values[i] == 1
                    && sD_803830E0->zoomboxes[i] != NULL) {
                    gczoombox_loadSprite(sD_803830E0->zoomboxes[i], ZOOMBOX_SPRITE_BRENTILDA);
                    break;
                }
            }
        }
        brentilda_hint_applied = TRUE;
    }

    if (sD_803830E0->state == GCQUIZ_STATE_0_IDLE) {
        brentilda_hint_applied = FALSE;
    }

    for (i = 0; i < 4; i++) {
        if (sD_803830E0->zoomboxes[i] != NULL) {
            gczoombox_draw(sD_803830E0->zoomboxes[i], gfx, mtx, vtx);
        }
    }
}
