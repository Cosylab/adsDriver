// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "ADSAddress.h"
#include "autoparamHandler.h"
#ifndef USE_TC_ADS
#include <standalone/AdsDef.h>
#endif /* ifndef USE_TC_ADS */
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#ifdef USE_TC_ADS
#include <windows.h>
#include <TcAdsDef.h>
#include <TcAdsApi.h>
#else
#include <AdsLib.h>
#endif /* ifdef USE_TC_ADS */
#include <autoparamDriver.h>
#include <SumReadRequest.h>
#include <Types.h>
#include <Variable.h>

using namespace Autoparam::Convenience;

constexpr uint16_t defaultSumBuferNelem = 500;
constexpr uint32_t defaultADSCallTimeout_ms = 500;
constexpr std::chrono::seconds deviceInfoPeriod{5};
constexpr std::chrono::milliseconds waitForConnectionPeriod{500};
constexpr std::chrono::milliseconds sumReadPeriod{1};

class ADSPortDriver;

class ADSDeviceAddress : public DeviceAddress {
  public:
    ADSDeviceAddress(std::string const &func,
                     std::vector<std::string> const &args);
    bool operator==(DeviceAddress const &other) const;
    ADSAddress address;
};

class ADSDeviceVar : public DeviceVariable {

  public:
    ADSDeviceVar(DeviceVariable *baseInfo, ADSPortDriver *driver);
    ADSPortDriver *driver;
    std::shared_ptr<ADSVariable> adsPV;
};

class ADSPortDriver : public Autoparam::Driver {
  public:
    ADSPortDriver(char const *portName, char const *ipAddr,
                  char const *amsNetId, uint16_t sumBufferSize,
                  uint32_t adsFunctionTimeout);

    ~ADSPortDriver();

    asynStatus ADSConnect(asynUser *pasynUser);
    asynStatus ADSDisconnect(asynUser *pasynUser);

  private:
    std::string portName;
    std::string ipAddr;
    AmsNetId amsNetId;
    uint16_t const sumBufferSize;
    uint32_t const adsFunctionTimeout;
    const std::shared_ptr<Connection> adsConnection;

    SumReadRequest SumRead;

    std::thread adsScanThread;
    std::atomic<bool> exitCalled;

    std::atomic<bool> initialized;

    std::vector< std::shared_ptr<ADSVariable>> ads_read_vars;
    std::vector< std::shared_ptr<ADSVariable>> ads_write_vars;

    DeviceVariable *createDeviceVariable(DeviceVariable *baseVar);
    DeviceAddress *parseDeviceAddress(std::string const &function,
                                      std::string const &arguments);

    ADSState currentAdsState;
    uint16_t currentDeviceState;
    std::string deviceInfo;
    AdsVersion adsVersion;

    // device info and state
    asynStatus readADSDeviceInfo();
    asynStatus readADSDeviceState();
    asynStatus doSumRead();

    template <typename PLCDataType, typename epicsDataType>
    void performArrayCallbacks(ADSDeviceVar &parentDeviceVar,
                               unsigned int const &nelem);
    void performIOIntr();

    void signalExit();
    void adsScan();

    // read/write for scalars
    template <typename PLCDataType, typename epicsDataType>
    static Result<epicsDataType> integerRead(DeviceVariable &deviceVar);
    template <typename PLCDataType>
    static UInt32ReadResult digitalRead(DeviceVariable &deviceVar,
                                        epicsUInt32 const mask = 0xffff);
    template <typename PLCDataType>
    static Float64ReadResult floatRead(DeviceVariable &deviceVar);

    template <typename PLCDataType, typename epicsDataType>
    static WriteResult integerWrite(DeviceVariable &deviceVar,
                                    epicsDataType val);

    template <typename PLCDataType>
    static WriteResult floatWrite(DeviceVariable &deviceVar, epicsFloat64 val);

    template <typename PLCDataType>
    static WriteResult digitalWrite(DeviceVariable &deviceVar,
                                    epicsUInt32 const val,
                                    epicsUInt32 const mask = 0xffff);

    // read/write for arrays
    template <typename PLCDataType, typename epicsDataType>
    static ArrayReadResult arrayRead(DeviceVariable &deviceVar,
                                     Array<epicsDataType> &val);

    template <typename PLCDataType, typename epicsDataType>
    static WriteResult arrayWrite(DeviceVariable &deviceVar,
                                  Array<epicsDataType> const &val);

    // strings
    static OctetReadResult stringRead(DeviceVariable &deviceVar, Octet &val);
    static WriteResult stringWrite(DeviceVariable &deviceVar, Octet const &val);

    static void initHook(Autoparam::Driver *driver);
};
