// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <vector>

#include <epicsMutex.h>
#ifdef USE_TC_ADS
#include <windows.h>
#include <TcAdsDef.h>
#include <TcAdsApi.h>
#else
#include <AdsLib.h>
#endif

#include "Variable.h"

class Connection {
  protected:
    AmsNetId remote_ams_netid;  /* Remote ADS device AMS net ID */
    std::string remote_address; /* Remote ADS device address */
    long ads_port = 0;          /* ADS connection handle */
    uint16_t device_read_ads_port;
    bool connected = false;

    /* Maximum number of `commands` that will be sent to the ADS device in a
     * single request. For sum read operations, this is the number of variables
     * that will be retrieved in one request. 500 is the recommended limit
     * according to Beckhoff documentation.
     *
     * See "ADS-sum command: Read of Write a list of variables with one single
     * ADS-command" for more information:
     * https://infosys.beckhoff.com/english.php?content=../content/1033/tcadscommon/html/note.htm&id=5044385556872883899
     */
    unsigned int sum_operations_max_commands = 500;

  public:
    /* True if ADS connection is established */
    bool is_connected();

    /* Remote ADS device AMS net ID */
    const AmsNetId get_remote_ams_netid();

    /* ADS port for the open connection, as returned by AdsPortOpenEx() */
    long get_ads_port();

    /* Connection mutex to use when calling class methods in multiple threads */
    epicsMutex mtx;

    Connection();
    ~Connection();

    void set_local_ams_id(const AmsNetId ams_id);
    int connect(const AmsNetId ams_id, const std::string address, const uint16_t deviceReadAdsPort);

    /* Disconnect from the ADS device, i.e. close the ADS port and remove the
     * remote AMS route. */
    int disconnect();

    /* Sets the connection state as 'disconnected', without calling any ADS
     * disconnect/port close functions.
     *
     * This can be used as a workaround when exit() is called in IOC shell. In
     * that scenario the IOC calls libc exit(), which deinitializes a static
     * AmsRouter object in the Beckhoff ADS library, causing subsequent calls
     * to ADS functions to fail or segfault. */
    void set_disconnected();

    /* Resolve (acquire handle) for a single ADS variable specified with a
     * symbolic name. */
    int resolve_variable(std::shared_ptr<ADSVariable> ads_variable);

    /* Resolve (acquire handles) for ADS variables specified with a symbolic
     * name. */
    int resolve_variables(
        const std::vector<std::shared_ptr<ADSVariable>> &ads_variables);

    /* Unresolve (release handle) for a single ADS variable specified with a
     * symbolic name. */
    int unresolve_variable(std::shared_ptr<ADSVariable> ads_variable);

    /* Unresolve (release handles) for ADS variables specified with a symbolic
     * name. */
    int unresolve_variables(
        const std::vector<std::shared_ptr<ADSVariable>> &ads_variables);

    /* Read ADS device information into DEVICE_INFO of size SIZE_DEVICE_INFO
     * bytes and ADS_VERSION. DEVICE_INFO should be at least 16 bytes long. */
    int read_device_info(char *device_info, size_t size_device_info,
                         AdsVersion *ads_version);

    /* Read ADS device and protocol state. */
    int read_device_state(uint16_t *device_state, ADSState *ads_state);
};

#endif /* CONNECTION_H */
