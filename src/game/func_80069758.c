#include "proximity_list.h"

extern void func_800556D0(int32_t failed, const char *condition, const char *source, uint32_t line);

static __inline__ int32_t proximity_signed_bits(uint32_t bits) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
    return (int32_t)bits;
#else
    return bits <= INT32_MAX ? (int32_t)bits : -1 - (int32_t)(UINT32_MAX - bits);
#endif
}

void func_80069758(void) {
    int32_t reference_z = 0;
    int32_t reference_x = 0;
    ProximityObject *reference = func_8005327C();
    ProximityNode *node;

    g_proximity_count = 0;
    if (reference != 0) {
        reference_x = reference->position->x;
        reference_z = reference->position->z;
    }

    func_800529D0(&node);
    if (node != 0) {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
        register ProximityObject **objects __asm__("$19") = g_proximity_objects;
#else
        ProximityObject **objects = g_proximity_objects;
#endif
        do {
            ProximityNode *current = node;
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            register int32_t radius __asm__("$5");
#else
            int32_t radius;
#endif
            ProximityObject *object = current->object;

            if (reference == 0) {
                goto next_current;
            }
            radius = object->radius;
            if (radius == 0) {
                goto next_current;
            }
            {
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
                register ProximityPosition *position __asm__("$4") = object->position;
                register int32_t delta_x __asm__("$3");
                register int32_t delta_z __asm__("$2");
#else
                ProximityPosition *position = object->position;
                int32_t delta_x;
                int32_t delta_z;
#endif
                int32_t x = position->x;
                int32_t z;

                delta_x = proximity_signed_bits((uint32_t)x - (uint32_t)reference_x);
                if (delta_x < 0) {
                    delta_x = proximity_signed_bits((uint32_t)reference_x - (uint32_t)x);
                }
                z = position->z;
                delta_z = proximity_signed_bits((uint32_t)z - (uint32_t)reference_z);
                if (delta_z >= 0) {
                    delta_z = proximity_signed_bits((uint32_t)delta_x + (uint32_t)delta_z);
                    if (delta_z >= radius) {
                        goto skip_positive;
                    }
                } else {
                    delta_z = proximity_signed_bits((uint32_t)reference_z - (uint32_t)z);
                    delta_z = proximity_signed_bits((uint32_t)delta_x + (uint32_t)delta_z);
                    if (delta_z >= radius) {
                        goto next_node;
                    }
                }
                {
                    uint16_t count = g_proximity_count;
                    int16_t next_count = (int16_t)(uint16_t)(count + 1u);
                    g_proximity_count = (uint16_t)next_count;
                    objects[(int16_t)count] = object;
                    if (next_count >= 40) {
                        func_800556D0(1, g_proximity_limit_condition, g_proximity_source_path, 0x342);
                    }
                }
            }
skip_positive:
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            /* Keep the two distance exits distinct without emitting instructions. */
            __asm__ volatile("");
#endif
next_node:
#ifdef BUBSY3D_MATCH_ORIGINAL_L0
            /* Preserve the original reload of the callback-visible list slot. */
            __asm__ volatile("" : "=m"(node) : "m"(node));
#endif
            current = node;
next_current:
            node = current->next;
        } while (node != 0);
    }
}
