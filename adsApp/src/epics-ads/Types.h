// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#ifndef EPICSADS_TYPES_H
#define EPICSADS_TYPES_H

#include <string>
#include <map>

#ifdef USE_TC_ADS
#include <windows.h>
#include <TcAdsDef.h>
#include <TcAdsApi.h>
#else
#include <AdsLib.h>
#endif

enum Operation { Read, Write };

enum class ADSDataType {
    UNKNOWN,
    BOOL,
    SINT,
    BYTE,
    USINT,
    INT,
    WORD,
    DWORD,
    UINT,
    DINT,
    UDINT,
    LINT,
    REAL,
    LREAL,
    STRING
};

/* See ADSSTATE (enum nAdsState) on Beckhoff Infosys */
enum ADSState {
    Invalid = 0,
    Idle = 1,
    Reset = 2,
    Init = 3,
    Start = 4,
    Run = 5,
    Stop = 6,
    Savecfg = 7,
    Loadcfg = 8,
    Powerfailure = 9,
    Powergood = 10,
    Error = 11,
    Shutdown = 12,
    Suspend = 13,
    Resume = 14,
    Config = 15,
    Reconfig = 16,
    Maxstates
};

extern std::map<ADSDataType, size_t> ads_datatype_sizes;
extern std::map<std::string, ADSDataType> ads_datatypes;
extern std::map<ADSDataType, std::string> ads_datatypes_str;

/* Map port names (e.g. "PLC_TC3" to constants (e.g. AMSPORT_R0_PLC_TC3)
 * defined in AdsDef.h */
extern std::map<std::string, uint16_t> ads_port_map;

extern std::map<int, std::string> ads_states;

#endif /* EPICSADS_TYPES_H */
