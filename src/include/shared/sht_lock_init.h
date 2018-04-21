#ifndef _SHT_LOCK_INIT_H
#define _SHT_LOCK_INIT_H

#include "shared/lock_init.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHT_MUTEX_ATTR(attr) WS_MUTEX_ATTR(attr)
#ifdef WS_LOCK_DBG
#define SHT_LOCK_DECL(spin) WS_MUTEX_DECL(spin)
#define SHT_LOCK_INIT(spin,mutex_attr) WS_MUTEX_INIT(spin,mutex_attr)
#define SHT_LOCK_DESTROY(spin) WS_MUTEX_DESTROY(spin)
#define SHT_LOCK(spin) WS_MUTEX_LOCK(spin)
#define SHT_UNLOCK(spin) WS_MUTEX_UNLOCK(spin)
#else
#define SHT_LOCK_DECL(spin) WS_SPINLOCK_DECL(spin)
#define SHT_LOCK_INIT(spin,mutex_attr) WS_SPINLOCK_INIT(spin)
#define SHT_LOCK_DESTROY(spin) WS_SPINLOCK_DESTROY(spin)
#define SHT_LOCK(spin) WS_SPINLOCK_LOCK(spin)
#define SHT_UNLOCK(spin) WS_SPINLOCK_UNLOCK(spin)
#endif // WS_LOCK_DBG

#ifdef __cplusplus
CPP_CLOSE
#endif // __cplusplus

#endif // _SHT_LOCK_INIT_H

