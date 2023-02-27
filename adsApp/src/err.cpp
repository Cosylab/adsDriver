// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#include "err.h"

#ifndef GLOBALERR_MISSING_ROUTE
#define GLOBALERR_MISSING_ROUTE 0x07
#endif

std::map<int, std::string> ads_errors = {
    { -1, "<unknown error>" },
    { EPICSADS_OK, "OK" },
    { EPICSADS_ERROR, "internal error" },
    { EPICSADS_INV_PARAM, "one or more parameters is invalid" },
    { EPICSADS_INV_CALL, "invalid function call" },
    { EPICSADS_NOT_ALLOCATED, "buffer not allocated" },
    { EPICSADS_NOT_INITIALIZED, "object not intialized" },
    { EPICSADS_NOT_RESOLVED, "ADS variable name not resolved" },
    { EPICSADS_OUT_OF_RANGE, "value is out of range" },
    { EPICSADS_LIMIT, "limit/capacity reached" },
    { EPICSADS_OVERFLOW, "reading/writing to buffer would overflow" },
    { EPICSADS_NO_DATA, "data could not be read" },
    { EPICSADS_TIMEOUT, "operation timed out" },
    { EPICSADS_DISCONNECTED, "ADS device is not connected" },
    { EPICSADS_UNHANDLED_RC, "unhandled ADS return code" }
};

std::map<long, int> ads_rc_to_epicsads_error_map = {
    { -1, -1 },
    { 0, EPICSADS_OK },
    { ADSERR_DEVICE_ERROR, EPICSADS_ERROR },
    { GLOBALERR_MISSING_ROUTE, EPICSADS_DISCONNECTED }, /* Occurs when ADS calls are made after AMS connection is closed */
    { ADSERR_CLIENT_SYNCTIMEOUT, EPICSADS_DISCONNECTED }, /* Occurs when ADS device is disconnected, e.g. cable is unplugged */
    { ADSERR_DEVICE_SYMBOLNOTFOUND, EPICSADS_NOT_RESOLVED } /* ADS symbol not found on ADS device */
};

std::map<long, std::string> errorMap =
{
    { 1, "Internal error" },
    { 2, "No Rtime"},
    { 3, "Allocation locked memory error"},
    { 4, "Insert mailbox error"},
    { 5, "Wrong receive HMSG"},
    { 6, "target port not found"},
    { 7, "target machine not found"},
    { 8, "Unknown command ID"},
    { 9, "Bad task ID"},
    { 10, "No IO"},
    { 11, "Unknown ADS command"},
    { 12, "Win 32 error"},
    { 13, "Port not connected"},
    { 14, "Invalid ADS length"},
    { 15, "Invalid ADS Net ID"},
    { 16, "Low Installation level"},
    { 17, "No debug available"},
    { 18, "Port disabled"},
    { 19, "Port already connected"},
    { 20, "ADS Sync Win32 error"},
    { 21, "ADS Sync Timeout"},
    { 22, "ADS Sync AMS error"},
    { 23, "ADS Sync no index map"},
    { 24, "Invalid ADS port"},
    { 25, "No memory"},
    { 26, "TCP send error"},
    { 27, "Host unreachable"},
    { 28, "Invalid AMS fragment"},
    { 1280, "No locked memory can be allocated"},
    { 1281, "The size of the router memory could not be changed"},
    { 1282, "The mailbox has reached the maximum number of possible messages. The current sent message was rejected"},
    { 1283, "The mailbox has reached the maximum number of possible messages. The sent message will not be displayed in the debug monitor"},
    { 1284, "The port type is unknown"},
    { 1285, "Router is not initialised"},
    { 1286, "RThe desired port number is already assigned"},
    { 1287, "Port not registered"},
    { 1288, "The maximum number of Ports reached"},
    { 1289, "The port is invalid."},
    { 1290, "TwinCAT Router not active"},
    { 1291, "ROUTERERR_FRAGMENTBOXFULL"},
    { 1292, "ROUTERERR_FRAGMENTTIMEOUT"},
    { 1293, "ROUTERERR_TOBEREMOVED"},
    { 1792, "error class <device error>"},
    { 1793, "Service is not supported by server"},
    { 1794, "invalid index group"},
    { 1795, "invalid index offset"},
    { 1796, "reading/writing not permitted"},
    { 1797, "parameter size not correct"},
    { 1798, "invalid parameter value(s)"},
    { 1799, "device is not in a ready state"},
    { 1800, "device is busy"},
    { 1801, "invalid context (must be in Windows)"},
    { 1802, "out of memory"},
    { 1803, "invalid parameter value(s)"},
    { 1804, "not found (files, ...)"},
    { 1805, "syntax error in command or file"},
    { 1806, "objects do not match"},
    { 1807, "object already exists"},
    { 1808, "symbol not found"},
    { 1809, "symbol version invalid"},
    { 1810, "server is in invalid state"},
    { 1811, "AdsTransMode not supported"},
    { 1812, "Notification handle is invalid"},
    { 1813, "Notification client not registered"},
    { 1814, "no more notification handles"},
    { 1815, "size for watch too big"},
    { 1816, "device not initialized"},
    { 1817, "device has a timeout"},
    { 1818, "query interface failed"},
    { 1819, "wrong interface required"},
    { 1820, "class ID is invalid"},
    { 1821, "object ID is invalid"},
    { 1822, "request is pending"},
    { 1823, "request is aborted"},
    { 1824, "signal warning"},
    { 1825, "invalid array index"},
    { 1826, "symbol not active"},
    { 1827, "access denied"},
    { 1856, "Error class <client error>"},
    { 1857, "invalid parameter at service"},
    { 1858, "polling list is empty"},
    { 1859, "var connection already in use"},
    { 1860, "invoke ID in use"},
    { 1861, "timeout elapsed"},
    { 1862, "error in win32 subsystem"},
    { 1863, "Invalid client timeout value"},
    { 1864, "ads-port not opened"},
    { 1872, "internal error in ads sync"},
    { 1873, "hash table overflow"},
    { 1874, "key not found in hash"},
    { 1875, "no more symbols in cache"},
    { 1876, "invalid response received"},
    { 1877, "sync port is locked"},
    { 4096, "Internal fatal error in the TwinCAT real-time system"},
    { 4097, "Timer value not vaild"},
    { 4098, "Task pointer has the invalid value ZERO"},
    { 4099, "Task stack pointer has the invalid value ZERO"},
    { 4100, "The demand task priority is already assigned"},
    { 4101, "No more free TCB (Task Control Block) available. Maximum number of TCBs is 64"},
    { 4102, "No more free semaphores available. Maximum number of semaphores is 64"},
    { 4103, "No more free queue available. Maximum number of queue is 64"},
    { 4104, "TwinCAT reserved."},
    { 4105, "TwinCAT reserved."},
    { 4106, "TwinCAT reserved."},
    { 4107, "TwinCAT reserved."},
    { 4108, "TwinCAT reserved."},
    { 4109, "An external synchronisation interrupt is already applied"},
    { 4110, "No external synchronsiation interrupt applied"},
    { 4111, "The apply of the external synchronisation interrupt failed"},
    { 4112, "Call of a service function in the wrong context"},
    { 4119, "Intel VT-x extension is not supported."},
    { 4120, "Intel VT-x extension is not enabled in BIOS."},
    { 4121, "Missing feature in Intel VT-x extension."},
    { 4122, "Enabling Intel VT-x fails."},
    { 10060, "A socket operation was attempted to an unreachable host."},
    { 10061, "A connection attempt failed because the connected party did not properly respond after a period of time, or established connection failed because connected host has failed to respond."},
    { 10065, "No connection could be made because the target machine actively refused it"}
};

int ads_rc_to_epicsads_error(long ads_rc) {
    auto it = ads_rc_to_epicsads_error_map.find(ads_rc);
    if (it == ads_rc_to_epicsads_error_map.end()) {
        LOG_ERR("unhandled ADS return code '%li'", ads_rc);
        return EPICSADS_UNHANDLED_RC;
    }

    return it->second;
}
