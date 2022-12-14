// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#include "Types.h"

#ifndef AMSPORT_R0_PLC_TC3
#define AMSPORT_R0_PLC_TC3 851
#endif

std::map<ADSDataType, size_t> ads_datatype_sizes = {
    {ADSDataType::BOOL, 1},  {ADSDataType::SINT, 1},  {ADSDataType::BYTE, 1},
    {ADSDataType::USINT, 1}, {ADSDataType::INT, 2},   {ADSDataType::WORD, 2},
    {ADSDataType::DWORD, 4}, {ADSDataType::UINT, 2},  {ADSDataType::DINT, 4},
    {ADSDataType::UDINT, 4}, {ADSDataType::REAL, 4},  {ADSDataType::LREAL, 8},
    {ADSDataType::LINT, 8},  {ADSDataType::STRING, 1}};

std::map<ADSDataType, std::string> ads_datatypes_str = {
    {ADSDataType::BOOL, "BOOL"},   {ADSDataType::SINT, "SINT"},
    {ADSDataType::BYTE, "BYTE"},   {ADSDataType::USINT, "USINT"},
    {ADSDataType::INT, "INT"},     {ADSDataType::WORD, "WORD"},
    {ADSDataType::DWORD, "DWORD"}, {ADSDataType::UINT, "UINT"},
    {ADSDataType::DINT, "DINT"},   {ADSDataType::UDINT, "UDINT"},
    {ADSDataType::REAL, "REAL"},   {ADSDataType::LREAL, "LREAL"},
    {ADSDataType::LINT, "LINT"},   {ADSDataType::STRING, "STRING"}};

std::map<std::string, ADSDataType> ads_datatypes = {
    {"BOOL", ADSDataType::BOOL},   {"SINT", ADSDataType::SINT},
    {"BYTE", ADSDataType::BYTE},   {"USINT", ADSDataType::USINT},
    {"INT", ADSDataType::INT},     {"WORD", ADSDataType::WORD},
    {"DWORD", ADSDataType::DWORD}, {"UINT", ADSDataType::UINT},
    {"DINT", ADSDataType::DINT},   {"UDINT", ADSDataType::UDINT},
    {"REAL", ADSDataType::REAL},   {"LREAL", ADSDataType::LREAL},
    {"LINT", ADSDataType::LINT},   {"STRING", ADSDataType::STRING}};

/* Map to constants defines in AdsDef.h */
std::map<std::string, uint16_t> ads_port_map = {
    {"LOGGER", AMSPORT_LOGGER},        {"RTIME", AMSPORT_R0_RTIME},
    {"TRACE", AMSPORT_R0_TRACE},       {"IO", AMSPORT_R0_IO},
    {"SPS", AMSPORT_R0_SPS},           {"NC", AMSPORT_R0_NC},
    {"ISG", AMSPORT_R0_ISG},           {"PCS", AMSPORT_R0_PCS},
    {"PLC", AMSPORT_R0_PLC},           {"PLC_RTS1", AMSPORT_R0_PLC_RTS1},
    {"PLC_RTS2", AMSPORT_R0_PLC_RTS2}, {"PLC_RTS3", AMSPORT_R0_PLC_RTS3},
    {"PLC_RTS4", AMSPORT_R0_PLC_RTS4}, {"PLC_TC3", AMSPORT_R0_PLC_TC3}};

std::map<int, std::string> ads_states = {
    {ADSState::Invalid, "invalid state"},
    {ADSState::Idle, "idle state"},
    {ADSState::Reset, "reset state"},
    {ADSState::Init, "initialized"},
    {ADSState::Start, "started"},
    {ADSState::Run, "running"},
    {ADSState::Stop, "stopped"},
    {ADSState::Savecfg, "saved configuration"},
    {ADSState::Loadcfg, "load configuration"},
    {ADSState::Powerfailure, "power failure"},
    {ADSState::Powergood, "power good"},
    {ADSState::Error, "error state"},
    {ADSState::Shutdown, "shutting down"},
    {ADSState::Suspend, "suspended"},
    {ADSState::Resume, "resumed"},
    {ADSState::Config, "system in config mode"},
    {ADSState::Reconfig, "system should restart in config mode"},
    {ADSState::Maxstates, "max states"}};
