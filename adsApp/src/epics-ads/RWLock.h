// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#ifndef RWLOCK_H
#define RWLOCK_H

#ifdef _WIN32
#include <mutex>
#include <shared_mutex>
#else
#include <pthread.h>
#endif

/* Reader/writer lock implementation based on POSIX rwlocks.
 *
 * - Don't acquire locks recursively.
 * - Don't release locks if currently not holding a lock.
 * */
class RWLock {
protected:
#ifdef _WIN32
    std::shared_mutex mutex_;
#else
    pthread_rwlock_t rwlock;
    pthread_rwlockattr_t rwlock_attr;

#endif
    RWLock();

public:
#ifdef _WIN32
#define PTHREAD_RWLOCK_PREFER_READER_NP 0
#define PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP 1
#endif
    static const int prefer_reader = PTHREAD_RWLOCK_PREFER_READER_NP;
    static const int prefer_writer = PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP;

    /* Flags chould be set to either prefer_reader or prefer_writer. */
    RWLock(int flags);
    ~RWLock();

    int lock_read();
    int try_lock_read();
    int unlock_read();

    int lock_write();
    int try_lock_write();
    int unlock_write();
};

#endif /* RWLOCK_H */
