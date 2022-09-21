// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#ifndef _TYPE_H_
#define _TYPE_H_

#include <map>

#include "epics-ads/Types.h"

std::map<std::string, uint16_t> portMap = 
{ 
    { "LOGGER", 100 },
    { "RTIME", 200 },
    { "TRACE", 290 },
    { "IO", 300 },
    { "SPS", 400 },
    { "NC", 500 },
    { "ISG", 550 },
    { "PCS", 600 },
    { "PLC", 801 },
    { "PLC_RTS1", 801 },
    { "PLC_RTS2", 811 },
    { "PLC_RTS3", 821 },
    { "PLC_RTS4", 831 },
    { "PLC_TC3", 851 }
};

/*std::map<std::string, size_t> lengthMap = 
{
    { "BOOL", 1 },
    { "SINT", 1 },
    { "BYTE", 1 },
    { "USINT", 1 },
    { "INT", 2 },
    { "WORD", 2 },
    { "UINT", 2 },
    { "DINT", 4 },
    { "UDINT", 4 },
    { "REAL", 4 },
    { "LREAL", 8 },
    { "STRING", 40}
};*/

#endif /* _TYPE_H_ */
