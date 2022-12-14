// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#ifndef ERR_H
#define ERR_H

#include <string>
#include <map>
#include <cstring>
#include <errlog.h>
#include "asynDriver.h"
#ifdef USE_TC_ADS
#include <windows.h>
#include <TcAdsDef.h>
#include <TcAdsApi.h>
#else
#include <AdsLib.h>
#endif

#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

/* Error logging macros for use when asyn_user is available */
#define LOG_MSG_ASYN(asyn_user, log_level, log_level_str, format, ...) \
{ \
    const char *port_name = nullptr; \
    pasynManager->getPortName(asyn_user, &port_name); \
    asynPrint(asyn_user, log_level, "[%s] %s:%u %s(): [%s] " format "\n", log_level_str, FILENAME, __LINE__, __func__, (port_name != nullptr ? port_name : "/"), ##__VA_ARGS__); \
}

#define LOG_ERR_ASYN(asyn_user, format, ...) \
    LOG_MSG_ASYN(asyn_user, ASYN_TRACE_ERROR, "ERROR", format, ##__VA_ARGS__)

#define LOG_WARN_ASYN(asyn_user, format, ...) \
    LOG_MSG_ASYN(asyn_user, ASYN_TRACE_WARNING, "WARNING", format, ##__VA_ARGS__)

#define LOG_TRACE_ASYN(asyn_user, format, ...) \
    LOG_MSG_ASYN(asyn_user, ASYN_TRACE_FLOW, "TRACE", format, ##__VA_ARGS__)

/* Error logging macros for use when asyn_user is not available */
#define LOG_MSG(log_level_str, format, ...) \
    errlogPrintf("[%s] %s:%u %s(): " format "\n", log_level_str, FILENAME, __LINE__, __func__, ##__VA_ARGS__)

#define LOG_ERR(format, ...) \
    LOG_MSG("ERROR", format, ##__VA_ARGS__)

#define LOG_WARN(format, ...) \
    LOG_MSG("WARNING", format, ##__VA_ARGS__)

#define LOG_TRACE(format, ...) \
    LOG_MSG("TRACE", format, ##__VA_ARGS__)

/* EPICS ADS specific return codes */
#define EPICSADS_BASE 1000
#define EPICSADS_OK   0
#define EPICSADS_ERROR              EPICSADS_BASE + 1
#define EPICSADS_INV_PARAM          EPICSADS_BASE + 2
#define EPICSADS_INV_CALL           EPICSADS_BASE + 3
#define EPICSADS_NOT_ALLOCATED      EPICSADS_BASE + 4
#define EPICSADS_NOT_INITIALIZED    EPICSADS_BASE + 5
#define EPICSADS_NOT_RESOLVED       EPICSADS_BASE + 6
#define EPICSADS_OUT_OF_RANGE       EPICSADS_BASE + 7
#define EPICSADS_LIMIT              EPICSADS_BASE + 8
#define EPICSADS_OVERFLOW           EPICSADS_BASE + 9
#define EPICSADS_NO_DATA            EPICSADS_BASE + 10
#define EPICSADS_TIMEOUT            EPICSADS_BASE + 11
#define EPICSADS_DISCONNECTED       EPICSADS_BASE + 12
#define EPICSADS_UNHANDLED_RC       EPICSADS_BASE + 13

/* EPICS ADS return code descriptions */
extern std::map<int, std::string> ads_errors;

/* Map of ADS return codes to EPICS ADS return codes. Use with
 * ads_rc_to_epicsads_error(), which handles unknown ADS return code cases. */
extern std::map<long, int> ads_rc_to_epicsads_error_map;

/* ADS return codes */
extern std::map<long, std::string> errorMap;

/* Return EPICSADS_xxx return code for the specified ADS return code. */
int ads_rc_to_epicsads_error(long ads_rc);

#endif /* ERR_H */
