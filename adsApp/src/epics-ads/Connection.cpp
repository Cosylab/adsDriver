// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#include <mutex>
#include "err.h"

#include "Connection.h"

#ifndef AMSPORT_R0_PLC_TC3
#define AMSPORT_R0_PLC_TC3 851
#endif

bool Connection::is_connected() { return (this->ads_port != 0 ? true : false); }

const AmsNetId Connection::get_remote_ams_netid() {
    return this->remote_ams_netid;
}

long Connection::get_ads_port() { return this->ads_port; }

Connection::Connection() {}

Connection::~Connection() {}

void Connection::set_local_ams_id(const AmsNetId ams_id) {
#ifndef USE_TC_ADS
    AdsSetLocalAddress(ams_id);
#endif
}

int Connection::connect(const AmsNetId ams_id, const std::string address) {
    std::lock_guard<epicsMutex> lock(this->mtx);

    /* Add AMS route */
#ifndef USE_TC_ADS
    long rc = AdsAddRoute(ams_id, address.c_str());
    if (rc != 0) {
        LOG_ERR("could not add ADS rout (%li): %s", rc, errorMap[rc].c_str());
        return EPICSADS_DISCONNECTED;
    }
#endif

    const long port = AdsPortOpenEx();
    if (port == 0) {
        LOG_ERR("could not open port to ADS device");
        return EPICSADS_DISCONNECTED;
    }

    this->remote_ams_netid = ams_id;
    this->ads_port = port;

    return 0;
}

int Connection::disconnect() {
    std::lock_guard<epicsMutex> lock(this->mtx);

    if (this->is_connected() == false) {
        return EPICSADS_DISCONNECTED;
    }

    AdsPortCloseEx(this->ads_port);
    this->ads_port = 0;

#ifndef USE_TC_ADS
    /* TODO: does this bork other ADS connections? */
    AdsDelRoute(this->remote_ams_netid);
#endif
    this->remote_ams_netid = {0, 0, 0, 0, 0, 0};

    return 0;
}

void Connection::set_disconnected() {
    std::lock_guard<epicsMutex> lock(this->mtx);

    this->ads_port = 0;
    this->remote_ams_netid = {0, 0, 0, 0, 0, 0};
}

int Connection::resolve_variable(std::shared_ptr<ADSVariable> ads_variable) {
    std::vector<std::shared_ptr<ADSVariable>> vec;
    vec.push_back(ads_variable);

    return this->resolve_variables(vec);
}

int Connection::resolve_variables(
    const std::vector<std::shared_ptr<ADSVariable>> &ads_variables) {
    std::lock_guard<epicsMutex> lock(this->mtx);

    if (ads_variables.size() == 0) {
        return EPICSADS_NO_DATA;
    }

    if (this->is_connected() == false) {
        return EPICSADS_DISCONNECTED;
    }

    for (size_t i = 0; i < ads_variables.size(); i++) {
        std::shared_ptr<ADSVariable> ads_var = ads_variables[i];
        if (ads_var->addr->is_resolved() == true) {
            continue;
        }

        // TODO: resolve handles using sum commands
        uint32_t handle = 0;
        AmsAddr ams_addr = {this->remote_ams_netid,
                            ads_var->addr->get_ads_port()};
        long rc = AdsSyncReadWriteReqEx2(
            this->ads_port,                        // ADS port
            &ams_addr,                             // AMS address
            ADSIGRP_SYM_HNDBYNAME,                 // index group
            0,                                     // index offset
            sizeof(handle),                        // read length
            &handle,                               // read data
            ads_var->addr->get_var_name().size(),  // write length
            const_cast<char*>(ads_var->addr->get_var_name().c_str()), // write data
            nullptr);                              // bytes read

        if (rc != 0) {
            LOG_WARN("could not resolve ADS variable '%s'",
                     ads_var->addr->get_var_name().c_str());
            return ads_rc_to_epicsads_error(rc);
        }

        rc = ads_var->addr->resolve(ADSIGRP_SYM_VALBYHND, handle);
        if (rc != 0) {
            return EPICSADS_ERROR;
        }
    }

    return 0;
}

int Connection::unresolve_variable(std::shared_ptr<ADSVariable> ads_variable) {
    std::vector<std::shared_ptr<ADSVariable>> vec;
    vec.push_back(ads_variable);

    return this->unresolve_variables(vec);
}

int Connection::unresolve_variables(
    const std::vector<std::shared_ptr<ADSVariable>> &ads_variables) {
    std::lock_guard<epicsMutex> lock(this->mtx);

    if (ads_variables.size() == 0) {
        return EPICSADS_NO_DATA;
    }

    bool ads_is_connected = this->is_connected();
    bool unresolve_errors = false;

    for (size_t i = 0; i < ads_variables.size(); i++) {
        std::shared_ptr<ADSVariable> ads_var = ads_variables[i];
        if (ads_var->addr->is_resolved() == false) {
            continue;
        }

        /* ADS handles on the PLC can be released while ADS connection is
         * established. */
        if (ads_is_connected == true) {
            uint32_t handle = ads_var->addr->get_index_offset();

            AmsAddr ams_addr = {this->remote_ams_netid,
                                ads_var->addr->get_ads_port()};
            long rc = AdsSyncWriteReqEx(this->ads_port,         // AMS port
                                        &ams_addr,              // AMS address
                                        ADSIGRP_SYM_RELEASEHND, // index group
                                        0,                      // index offset
                                        sizeof(handle),         // buffer length
                                        &handle);               // buffer
            if (rc != 0) {
                /* Skip trying to unresolve variables on the ADS device if
                 * there is no connection, but still mark the input variables
                 * as unresolved. */
                if (ads_rc_to_epicsads_error(rc) == EPICSADS_DISCONNECTED) {
                    ads_is_connected = false;
                }

                unresolve_errors = true;
            }
        }

        /* Variables are marked as unresolved if ADS communication is
         * established or not. */
        ads_var->addr->unresolve();
    }

    if (ads_is_connected == false) {
        return EPICSADS_DISCONNECTED;
    } else if (unresolve_errors == true) {
        return EPICSADS_NOT_RESOLVED;
    }

    return 0;
}

int Connection::read_device_info(char *device_info, size_t size_device_info,
                                 AdsVersion *ads_version) {
    if (device_info == nullptr || ads_version == nullptr) {
        return EPICSADS_INV_PARAM;
    }

    if (size_device_info < 16) {
        return EPICSADS_OVERFLOW;
    }

    if (this->is_connected() == false) {
        return EPICSADS_DISCONNECTED;
    }

    std::lock_guard<epicsMutex> lock(this->mtx);
    AmsAddr ams_addr = {this->remote_ams_netid, AMSPORT_R0_PLC_TC3};
    long rc = AdsSyncReadDeviceInfoReqEx(this->ads_port, // ADS port
                                         &ams_addr,      // AMS address
                                         device_info,    // device name
                                         ads_version);   // ADS version

    return ads_rc_to_epicsads_error(rc);
}

int Connection::read_device_state(uint16_t *device_state, ADSState *ads_state) {
    if (device_state == nullptr || ads_state == nullptr) {
        return EPICSADS_INV_PARAM;
    }

    if (this->is_connected() == false) {
        return EPICSADS_DISCONNECTED;
    }

    std::lock_guard<epicsMutex> lock(this->mtx);
    AmsAddr ams_addr = {this->remote_ams_netid, AMSPORT_R0_PLC_TC3};
    uint16_t ads_state_value = 0;
    long rc = AdsSyncReadStateReqEx(this->ads_port,   // ADS port
                                    &ams_addr,        // AMS address
                                    &ads_state_value, // ADS state
                                    device_state);    // device state
    if (rc == 0) {
        *ads_state = (ADSState)ads_state_value;
    }

    return ads_rc_to_epicsads_error(rc);
}
