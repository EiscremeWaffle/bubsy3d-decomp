#include "player_model.h"

int32_t player_actor_set_flag_04(PlayerActorFlagByte *actor) {
    actor->flags_04 |= 0x04;
    return 0;
}

int32_t player_actor_clear_flag_04(PlayerActorFlagByte *actor) {
    actor->flags_04 &= (uint8_t)~0x04;
    return 0;
}
