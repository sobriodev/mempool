#ifndef MEMPOOL_COMMON_H
#define MEMPOOL_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------ */
/* -------------------------- Macros -------------------------- */
/* ------------------------------------------------------------ */

/** Likely branch prediction */
#define LIKELY(X)   __builtin_expect(!!(X), 1)
/** Unlikely branch prediction */
#define UNLIKELY(X) __builtin_expect(!!(X), 0)

#ifdef __cplusplus
}
#endif

#endif //MEMPOOL_COMMON_H
