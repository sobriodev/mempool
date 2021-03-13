#ifndef MEMPOOL_BIT_H
#define MEMPOOL_BIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

/* ------------------------------------------------------------ */
/* ---------------------------- Macros ------------------------ */
/* ------------------------------------------------------------ */

/** Major version */
#define BIT_API_VERSION_MAJOR 0
/** Minor version */
#define BIT_API_VERSION_MINOR 1
/** Revision version */
#define BIT_API_VERSION_REVISION 0

/** Get value with bit set at specific position */
#define BIT_32_GET_AT_POS(POS) ((u32)1 << (POS))

/** Bitwise NOT */
#define BIT_32_NOT(V) (~((u32)(V)))

/** Set bit at specific position */
#define BIT_32_SET(V, B) ((V) = ((u32)(V) | BIT_32_GET_AT_POS(B)))

/** Clear bit at specific position */
#define BIT_32_CLR(V, B) ((V) = ((V) & BIT_32_NOT(BIT_32_GET_AT_POS(B))))

/** Check if specific bit is set */
#define BIT_32_IS_SET(V, B) ((bool)((u32)(V) & BIT_32_GET_AT_POS(B)))

/** Check if specific bit is not set */
#define BIT_32_IS_NOT_SET(V, B) (!BIT_32_IS_SET(V, B))

/** Set multiple bits using MSK and POS values */
#define BIT_32_SET_MUL(V, MSK, POS, B) ((V) = ((u32)(V) & ~((MSK) << (POS))) | (((u32)(B) & (MSK)) << (POS)))

/** Get multiple bits using MSK and POS values */
#define BIT_32_GET_MUL(V, MSK, POS) (((u32)(V) & ((MSK) << (POS))) >> (POS))

#ifdef __cplusplus
}
#endif

#endif //MEMPOOL_BIT_H
