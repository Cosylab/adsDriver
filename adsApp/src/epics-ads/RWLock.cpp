// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#include <stdexcept>
#include "err.h"
#include "RWLock.h"

#ifdef LINUX_USE_CPP11
static int unlock(pthread_rwlock_t *rwlock);
#endif


RWLock::RWLock(int flags) {
#ifdef LINUX_USE_CPP11
    int rc = pthread_rwlockattr_init(&this->rwlock_attr);
    if (rc != 0) {
        throw std::runtime_error("could not initialize RW lock attributes; pthread_rwlockattr_init: " + rc);
    }

    rc = pthread_rwlockattr_setkind_np(&this->rwlock_attr, flags);
    if (rc != 0) {
        throw std::runtime_error("could not set RW lock attributes; pthread_rwlockattr_setkind_np: " + rc);
    }

    rc = pthread_rwlock_init(&this->rwlock, &this->rwlock_attr);
    if (rc != 0) {
        throw std::runtime_error("could not initialize RW lock; pthread_rwlock_init: " + rc);
    }
#endif
}

RWLock::~RWLock() {
#ifdef LINUX_USE_CPP11
    int rc = pthread_rwlock_destroy(&this->rwlock);
    if (rc != 0) {
        /* Very, very bad. */
        LOG_ERR("failed to destroy RW lock; pthread_rwlock_destroy: %i", rc);
    }

    rc = pthread_rwlockattr_destroy(&this->rwlock_attr);
    if (rc != 0) {
        /* Also very bad. */
        LOG_ERR("failed to destroy rwlock_attr; pthread_rwlockattr_destroy: %i", rc);
    }
#endif
}

int RWLock::lock_read() {
#ifdef LINUX_USE_CPP11
    int rc = pthread_rwlock_rdlock(&this->rwlock);
    if (rc != 0) {
        return EPICSADS_ERROR;
    }

    return 0;
#else
    mutex_.lock_shared();
    return 0;
#endif
}

int RWLock::try_lock_read() {
#ifdef LINUX_USE_CPP11
    int rc = pthread_rwlock_tryrdlock(&this->rwlock);
    if (rc != 0) {
        return EPICSADS_ERROR;
    }

    return 0;
#else
    return mutex_.try_lock_shared() ? 0 : EPICSADS_ERROR;
#endif
}

int RWLock::unlock_read() {
#ifdef LINUX_USE_CPP11
    int rc = pthread_rwlock_unlock(&this->rwlock);
    if (rc != 0) {
        return EPICSADS_ERROR;
    }

    return 0;
#else
    mutex_.unlock_shared();
    return 0;
#endif
}

int RWLock::lock_write() {
#ifdef LINUX_USE_CPP11
    int rc = pthread_rwlock_wrlock(&this->rwlock);
    if (rc != 0) {
        return EPICSADS_ERROR;
    }

    return 0;
#else
    mutex_.lock();
    return 0;
#endif
}

int RWLock::try_lock_write() {
#ifdef LINUX_USE_CPP11
    int rc = pthread_rwlock_trywrlock(&this->rwlock);
    if (rc != 0) {
        return EPICSADS_ERROR;
    }

    return 0;
#else
    return mutex_.try_lock() ? 0 : EPICSADS_ERROR;
#endif
}

int RWLock::unlock_write() {
#ifdef LINUX_USE_CPP11
    int rc = unlock(&this->rwlock);
    if (rc != 0) {
        return EPICSADS_ERROR;
    }

    return 0;
#else
    mutex_.unlock();
    return 0;
#endif
}

#ifdef LINUX_USE_CPP11
static int unlock(pthread_rwlock_t *rwlock) {
    return pthread_rwlock_unlock(rwlock);
}
#endif
