// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#include <cstring>
#include <map>
#include <memory>
#include <iocsh.h>
#include <errlog.h>
#include "ADSPortDriver.h"
#include <epicsExport.h>

static const iocshArg ads_open_arg = {"parameters", iocshArgArgv};
static const iocshArg *ads_open_args[1] = {&ads_open_arg};
static const iocshFuncDef ads_open_func_def = {"AdsOpen", 1, ads_open_args};

static const iocshArg ads_find_io_arg = {"port_name", iocshArgString};
static const iocshArg *ads_find_io_args[] = {&ads_find_io_arg};
static const iocshFuncDef ads_find_io_func_def = {"AdsFindIOIntrRecord", 1,
                                                  ads_find_io_args};

static const iocshArg ads_set_ams_arg = {"ams_net_id", iocshArgString};
static const iocshArg *ads_set_ams_args[] = {&ads_set_ams_arg};
static const iocshFuncDef ads_set_local_amsNetID_func_def = {
    "AdsSetLocalAMSNetID", 1, ads_set_ams_args};

epicsShareFunc int ads_open(int argc, const char *const *argv) {
    std::string port_name;
    std::string ip_addr;
    std::string ams_net_id;
    int sum_buffer_nelem = defaultSumBuferNelem;
    int ads_function_timeout_ms = -1;

    if (argc < 3 || argc > 6) {
        errlogPrintf(
            "AdsOpen <port_name> <ip_addr> <ams_net_id>"
            " | optional: <sum_buffer_nelem (default: %u)> <ads_timeout "
            "(default: %u) [ms]>\n",
            defaultSumBuferNelem, defaultADSCallTimeout_ms);
        return -1;
    }

    for (int i = 1; i < argc; i++) {
        switch (i) {
        case 1:
            port_name = argv[i];
            break;
        case 2:
            ip_addr = argv[i];
            break;
        case 3:
            ams_net_id = argv[i];
            break;
        case 4:
            sum_buffer_nelem = strtol(argv[i], nullptr, 10);
            if (sum_buffer_nelem < 1) {
                errlogPrintf(
                    "Error: sum_buffer_nelem must be a positive integer (%i)\n",
                    sum_buffer_nelem);
                return -1;
            }
            break;
        case 5:
            ads_function_timeout_ms = strtol(argv[i], nullptr, 10);
            if (ads_function_timeout_ms < 1) {
                errlogPrintf(
                    "ads_function_timeout must be positive integer!\n");
                return -1;
            }
            break;
        default:
            break;
        }
    }

    new ADSPortDriver(port_name.c_str(), ip_addr.c_str(), ams_net_id.c_str(),
                      sum_buffer_nelem, ads_function_timeout_ms);

    return 0;
}

epicsShareFunc void ads_set_local_amsNetID(const char *ams) {
    if (ams == NULL)
        errlogPrintf("Local Ams Net ID must be specified\n");
    else
#ifndef USE_TC_ADS
        AdsSetLocalAddress((std::string)ams);
#endif

    return;
}

static void ads_open_call_func(const iocshArgBuf *args) {
    ads_open(args[0].aval.ac, args[0].aval.av);
}

static void ads_set_local_amsNetID_call_func(const iocshArgBuf *args) {
    ads_set_local_amsNetID(args[0].sval);
}

static void ads_open_register_command(void) {
    static int already_registered = 0;

    if (already_registered == 0) {
        iocshRegister(&ads_open_func_def, ads_open_call_func);
        already_registered = 1;
    }
}

static void ads_set_local_amsNetID_register_command(void) {
    static int already_registered = 0;

    if (already_registered == 0) {
        iocshRegister(&ads_set_local_amsNetID_func_def,
                      ads_set_local_amsNetID_call_func);
        already_registered = -1;
    }
}

extern "C" {
epicsExportRegistrar(ads_open_register_command);
epicsExportRegistrar(ads_set_local_amsNetID_register_command);
}
