// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#include <stdexcept>
#include "err.h"
#include "RWLock.h"

static int unlock(pthread_rwlock_t *rwlock);


RWLock::RWLock(int flags) {
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
}

RWLock::~RWLock() {
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
}

int RWLock::lock_read() {
    int rc = pthread_rwlock_rdlock(&this->rwlock);
    if (rc != 0) {
        return EPICSADS_ERROR;
    }

    return 0;
}

int RWLock::try_lock_read() {
    int rc = pthread_rwlock_tryrdlock(&this->rwlock);
    if (rc != 0) {
        return EPICSADS_ERROR;
    }

    return 0;
}

int RWLock::unlock_read() {
    int rc = pthread_rwlock_unlock(&this->rwlock);
    if (rc != 0) {
        return EPICSADS_ERROR;
    }

    return 0;
}

int RWLock::lock_write() {
    int rc = pthread_rwlock_wrlock(&this->rwlock);
    if (rc != 0) {
        return EPICSADS_ERROR;
    }

    return 0;
}

int RWLock::try_lock_write() {
    int rc = pthread_rwlock_trywrlock(&this->rwlock);
    if (rc != 0) {
        return EPICSADS_ERROR;
    }

    return 0;
}

int RWLock::unlock_write() {
    int rc = unlock(&this->rwlock);
    if (rc != 0) {
        return EPICSADS_ERROR;
    }

    return 0;
}

static int unlock(pthread_rwlock_t *rwlock) {
    return pthread_rwlock_unlock(rwlock);
}
