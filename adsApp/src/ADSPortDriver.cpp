// SPDX-FileCopyrightText: 2022 Cosylab d.d.
//
// SPDX-License-Identifier: MIT

#include <ADSAddress.h>
#include <Variable.h>
#include <autoparamHandler.h>
#include <err.h>
#ifndef USE_TC_ADS
#include <standalone/AdsDef.h>
#endif /* USE_TC_ADS */
#include <Connection.h>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <sstream>
#include <Types.h>
#include <err.h>
#include <ADSPortDriver.h>
#include <stdexcept>
#include <thread>
#include <vector>
#include <boost/algorithm/string.hpp>

bool ADSDeviceAddress::operator==(DeviceAddress const &other) const {
    ADSDeviceAddress const &b = static_cast<ADSDeviceAddress const &>(other);
    return address.get_var_name() == b.address.get_var_name() &&
           address.get_data_type() == b.address.get_data_type() &&
           address.get_ads_port() == b.address.get_ads_port() &&
           address.get_nelem() == b.address.get_nelem() &&
           address.get_operation() == b.address.get_operation();
}

ADSDeviceAddress::ADSDeviceAddress(std::string const &func,
                                   std::vector<std::string> const &args)
    : address(func, args) {}

DeviceAddress *ADSPortDriver::parseDeviceAddress(std::string const &function,
                                                 std::string const &arguments) {
    std::istringstream stream(arguments);
    std::string arg;
    std::vector<std::string> parsedArguments;
    while (stream >> arg) {
        parsedArguments.push_back(arg);
    }

    ADSDeviceAddress *adsDeviceAddr = nullptr;
    try {
        adsDeviceAddr = new ADSDeviceAddress(function, parsedArguments);
    } catch (std::invalid_argument &err) {
        LOG_ERR("%s: ERROR, %s\n", __FUNCTION__, err.what());
        delete adsDeviceAddr;
        return nullptr;
    }

    // caller of this function is the owner of the returned pointer
    return adsDeviceAddr;
}

ADSDeviceVar::ADSDeviceVar(DeviceVariable *baseInfo, ADSPortDriver *driver)
    : DeviceVariable(baseInfo), driver(driver),
      adsPV(new ADSVariable(
          std::make_shared<ADSAddress>(
              static_cast<ADSDeviceAddress const &>(address()).address),
          false)) {}

DeviceVariable *ADSPortDriver::createDeviceVariable(DeviceVariable *baseInfo) {
    ADSDeviceVar *adsDeviceVar = nullptr;
    try {
        adsDeviceVar = new ADSDeviceVar(baseInfo, this);
    } catch (std::invalid_argument &err) {
        LOG_ERR("%s: ERROR, %s\n", __FUNCTION__, err.what());
        delete adsDeviceVar;
        return nullptr;
    }

    adsDeviceVar->adsPV->set_connection(adsConnection);

    if (adsDeviceVar->adsPV->addr->get_operation() == Operation::Read) {
        ads_read_vars.push_back(adsDeviceVar->adsPV);
    } else {
        ads_write_vars.push_back(adsDeviceVar->adsPV);
    }

    // caller of this function is the owner of the returned pointer
    return adsDeviceVar;
}

ADSPortDriver::ADSPortDriver(
    char const *portName, char const *ipAddr, char const *amsNetId,
    uint16_t sumBufferSize = defaultSumBuferNelem,
    uint32_t adsFunctionTimeout = defaultADSCallTimeout_ms, uint16_t deviceReadAdsPort = defaultDeviceReadADSPort)
    : Autoparam::Driver(portName, Autoparam::DriverOpts()
                                      .setAutoInterrupts(false)
                                      .setAutoConnect(true)
                                      .setAutoDestruct()
                                      .setInitHook(initHook)),
      portName(portName), ipAddr(ipAddr), amsNetId{0, 0, 0 ,0 ,0 ,0},
      sumBufferSize(sumBufferSize), adsFunctionTimeout(adsFunctionTimeout),
      deviceReadAdsPort(deviceReadAdsPort),  adsConnection(new Connection()),
      SumRead(sumBufferSize, adsConnection),
      exitCalled(false), initialized(false),
      currentDeviceState(ADSSTATE_INVALID) {

#ifdef USE_TC_ADS
    std::vector<std::string> split_ams;
    boost::split(split_ams, amsNetId, boost::is_any_of("."));
    for(int i=0; i < split_ams.size() && i < 6; ++i)
    {
        this->amsNetId.b[i] = atoi(split_ams[i].c_str());
    }
#else
    this->amsNetId = std::string(amsNetId);
#endif

    // scalars
    registerHandlers<epicsInt32>(ads_datatypes_str.at(ADSDataType::BOOL),
                                 integerRead<epicsInt8, epicsInt32>,
                                 integerWrite<epicsInt8, epicsInt32>, NULL);
    registerHandlers<epicsInt32>(ads_datatypes_str.at(ADSDataType::SINT),
                                 integerRead<epicsInt8, epicsInt32>,
                                 integerWrite<epicsInt8, epicsInt32>, NULL);
    registerHandlers<epicsInt32>(ads_datatypes_str.at(ADSDataType::BYTE),
                                 integerRead<epicsInt8, epicsInt32>,
                                 integerWrite<epicsInt8>, NULL);
    registerHandlers<epicsInt32>(ads_datatypes_str.at(ADSDataType::USINT),
                                 integerRead<epicsUInt8, epicsInt32>,
                                 integerWrite<epicsUInt8, epicsInt32>, NULL);
    registerHandlers<epicsInt32>(ads_datatypes_str.at(ADSDataType::INT),
                                 integerRead<epicsInt16, epicsInt32>,
                                 integerWrite<epicsInt16, epicsInt32>, NULL);
    registerHandlers<epicsInt32>(ads_datatypes_str.at(ADSDataType::WORD),
                                 integerRead<epicsUInt16, epicsInt32>,
                                 integerWrite<epicsUInt16, epicsInt32>, NULL);
    registerHandlers<epicsInt32>(ads_datatypes_str.at(ADSDataType::UINT),
                                 integerRead<epicsUInt16, epicsInt32>,
                                 integerWrite<epicsUInt16, epicsInt32>, NULL);
    registerHandlers<epicsInt32>(ads_datatypes_str.at(ADSDataType::DINT),
                                 integerRead<epicsInt32, epicsInt32>,
                                 integerWrite<epicsInt32, epicsInt32>, NULL);
    registerHandlers<epicsInt64>(ads_datatypes_str.at(ADSDataType::DWORD),
                                 integerRead<epicsUInt32, epicsInt64>,
                                 integerWrite<epicsUInt32, epicsInt64>, NULL);
    registerHandlers<epicsInt64>(ads_datatypes_str.at(ADSDataType::UDINT),
                                 integerRead<epicsUInt32, epicsInt64>,
                                 integerWrite<epicsUInt32, epicsInt64>, NULL);
    registerHandlers<epicsInt64>(ads_datatypes_str.at(ADSDataType::LINT),
                                 integerRead<epicsInt64, epicsInt64>,
                                 integerWrite<epicsInt64, epicsInt64>, NULL);
    registerHandlers<epicsFloat64>(ads_datatypes_str.at(ADSDataType::REAL),
                                   floatRead<epicsFloat32>,
                                   floatWrite<epicsFloat32>, NULL);
    registerHandlers<epicsFloat64>(ads_datatypes_str.at(ADSDataType::LREAL),
                                   floatRead<epicsFloat64>,
                                   floatWrite<epicsFloat64>, NULL);

    // digital IO
    registerHandlers<epicsUInt32>(
        ads_datatypes_str.at(ADSDataType::BOOL) + "_digi",
        digitalRead<epicsUInt8>, digitalWrite<epicsUInt8>, NULL);

    registerHandlers<epicsUInt32>(
        ads_datatypes_str.at(ADSDataType::BYTE) + "_digi",
        digitalRead<epicsUInt8>, digitalWrite<epicsUInt8>, NULL);

    registerHandlers<epicsUInt32>(
        ads_datatypes_str.at(ADSDataType::USINT) + "_digi",
        digitalRead<epicsUInt8>, digitalWrite<epicsUInt8>, NULL);

    registerHandlers<epicsUInt32>(
        ads_datatypes_str.at(ADSDataType::WORD) + "_digi",
        digitalRead<epicsUInt16>, digitalWrite<epicsUInt16>, NULL);

    registerHandlers<epicsUInt32>(
        ads_datatypes_str.at(ADSDataType::UINT) + "_digi",
        digitalRead<epicsUInt16>, digitalWrite<epicsUInt16>, NULL);

    registerHandlers<epicsUInt32>(
        ads_datatypes_str.at(ADSDataType::DWORD) + "_digi",
        digitalRead<epicsUInt32>, digitalWrite<epicsUInt32>, NULL);

    registerHandlers<epicsUInt32>(
        ads_datatypes_str.at(ADSDataType::UDINT) + "_digi",
        digitalRead<epicsUInt32>, digitalWrite<epicsUInt32>, NULL);

    // arrays
    registerHandlers<Array<epicsInt8>>(ads_datatypes_str.at(ADSDataType::BOOL) +
                                           "[]",
                                       arrayRead<epicsInt8, epicsInt8>,
                                       arrayWrite<epicsInt8, epicsInt8>, NULL);
    registerHandlers<Array<epicsInt8>>(ads_datatypes_str.at(ADSDataType::SINT) +
                                           "[]",
                                       arrayRead<epicsInt8, epicsInt8>,
                                       arrayWrite<epicsInt8, epicsInt8>, NULL);
    registerHandlers<Array<epicsInt8>>(ads_datatypes_str.at(ADSDataType::BYTE) +
                                           "[]",
                                       arrayRead<epicsInt8, epicsInt8>,
                                       arrayWrite<epicsInt8, epicsInt8>, NULL);
    registerHandlers<Array<epicsInt8>>(
        ads_datatypes_str.at(ADSDataType::USINT) + "[]",
        arrayRead<epicsUInt8, epicsInt8>, arrayWrite<epicsUInt8, epicsInt8>,
        NULL);
    registerHandlers<Array<epicsInt16>>(
        ads_datatypes_str.at(ADSDataType::INT) + "[]",
        arrayRead<epicsInt16, epicsInt16>, arrayWrite<epicsInt16, epicsInt16>,
        NULL);
    registerHandlers<Array<epicsInt16>>(
        ads_datatypes_str.at(ADSDataType::WORD) + "[]",
        arrayRead<epicsInt16, epicsInt16>, arrayWrite<epicsInt16, epicsInt16>,
        NULL);
    registerHandlers<Array<epicsInt16>>(
        ads_datatypes_str.at(ADSDataType::UINT) + "[]",
        arrayRead<epicsUInt16, epicsInt16>, arrayWrite<epicsUInt16, epicsInt16>,
        NULL);
    registerHandlers<Array<epicsInt32>>(
        ads_datatypes_str.at(ADSDataType::DINT) + "[]",
        arrayRead<epicsInt32, epicsInt32>, arrayWrite<epicsInt32, epicsInt32>,
        NULL);
    registerHandlers<Array<epicsInt32>>(
        ads_datatypes_str.at(ADSDataType::UDINT) + "[]",
        arrayRead<epicsUInt32, epicsInt32>, arrayWrite<epicsUInt32, epicsInt32>,
        NULL);
    registerHandlers<Array<epicsInt64>>(
        ads_datatypes_str.at(ADSDataType::LINT) + "[]",
        arrayRead<epicsInt64, epicsInt64>, arrayWrite<epicsInt64, epicsInt64>,
        NULL);
    registerHandlers<Array<epicsFloat32>>(
        ads_datatypes_str.at(ADSDataType::REAL) + "[]",
        arrayRead<epicsFloat32, epicsFloat32>,
        arrayWrite<epicsFloat32, epicsFloat32>, NULL);
    registerHandlers<Array<epicsFloat64>>(
        ads_datatypes_str.at(ADSDataType::LREAL) + "[]",
        arrayRead<epicsFloat64, epicsFloat64>,
        arrayWrite<epicsFloat64, epicsFloat64>, NULL);

    // strings
    registerHandlers<Octet>(ads_datatypes_str.at(ADSDataType::STRING),
                            stringRead, stringWrite, NULL);

    LOG_TRACE("ADSPortDriver parameters: %s, %s, %s, %d, %d %d", portName, ipAddr,
              amsNetId, sumBufferSize, adsFunctionTimeout, deviceReadAdsPort);
    LOG_TRACE("ADSPortDriver instance: %p, ip: %s", this, ipAddr);

    adsScanThread = std::thread(&ADSPortDriver::adsScan, this);
}

ADSPortDriver::~ADSPortDriver() {
    LOG_WARN_ASYN(pasynUserSelf, "Shutting down");
    {
        std::lock_guard<ADSPortDriver> guard(*this);
        exitCalled = true;
    }

    LOG_WARN_ASYN(pasynUserSelf, "Waiting for threads to join");
    adsScanThread.join();

    LOG_WARN_ASYN(pasynUserSelf, "Shutdown complete");
}

void ADSPortDriver::initHook(Autoparam::Driver *driver) {
    auto *self = static_cast<ADSPortDriver *>(driver);
    LOG_TRACE_ASYN(self->pasynUserSelf, "Entering");

    LOG_TRACE("ADSPortDriver instance: %p, ip: %s", self, self->ipAddr.c_str());

    if (self->initialized) {
        LOG_TRACE_ASYN(self->pasynUserSelf, "%s already initialized",
                       self->portName.c_str());
        return;
    }

    {
        std::lock_guard<ADSPortDriver> guard(*self);

        auto status = self->SumRead.allocate(self->ads_read_vars);
        if (status) {
            LOG_ERR_ASYN(self->pasynUserSelf,
                         "Error allocating sum-read request buffers (%i): %s",
                         status, ads_errors[status].c_str());
            return;
        }

        LOG_TRACE_ASYN(
            self->pasynUserSelf,
            "Successfully allocated sum-read request buffers for %lu variables",
            self->ads_read_vars.size());

        self->initialized = true;
    }
}

asynStatus ADSPortDriver::ADSConnect(asynUser *pasynUser) {
    LOG_TRACE_ASYN(pasynUser, "Entering");
    LOG_TRACE("ADSPortDriver instance: %p, ip: %s", this, ipAddr.c_str());
    asynStatus status;

    if (exitCalled) {
        return asynError;
    }
    if (!initialized) {
        return asynError;
    }

    if (adsConnection->is_connected()) {
        LOG_WARN_ASYN(pasynUser, "Already connected to ADS device");
        return asynSuccess;
    }

    // connect to the ADS device
    status = static_cast<asynStatus>(adsConnection->connect(amsNetId, ipAddr, deviceReadAdsPort));

    if (status) {
        LOG_ERR_ASYN(pasynUser, "Could not connect to ADS device (%i): %s",
                     status, ads_errors[status].c_str());
        return status;
    }
    LOG_WARN_ASYN(pasynUser, "Connected to ADS device (IP: %s)",
                  ipAddr.c_str());

    // resolving means translating symbolic names to actual addresses
    // it is done separately for read and write vars
    LOG_WARN_ASYN(pasynUser, "Resolving ADS variable names");
    LOG_WARN_ASYN(pasynUser, "Note that this can take a "
                             "minute, depending on amount of variables");

    if (ads_read_vars.size()) {
        status = static_cast<asynStatus>(
            adsConnection->resolve_variables(ads_read_vars));

        if (status) {
            LOG_ERR_ASYN(pasynUser,
                         "Could not resolve ADS read variable names (%i): %s",
                         status, ads_errors[status].c_str());
            return status;
        }
    }

    if (ads_write_vars.size()) {
        status = static_cast<asynStatus>(
            adsConnection->resolve_variables(ads_write_vars));

        if (status) {
            LOG_ERR_ASYN(pasynUser,
                         "Could not resolve ADS write variable names(%i): %s",
                         status, ads_errors[status].c_str());
            return status;
        }
    }
    LOG_WARN_ASYN(pasynUser, "Resolved %lu read and %lu write variable names",
                  ads_read_vars.size(), ads_write_vars.size());

    // initialize sum-read buffers
    status = static_cast<asynStatus>(SumRead.initialize());
    if (status) {
        LOG_ERR_ASYN(pasynUser,
                     "Error initializing sum-read request buffers (%i): %s",
                     status, ads_errors[status].c_str());
        return status;
    }

    LOG_WARN_ASYN(pasynUser, "Initialized sum-read request buffers");

    status = doSumRead();
    LOG_WARN_ASYN(pasynUser, "Inital sum-read status (%i): %s", status,
                  ads_errors[status].c_str());

    return status;
}

asynStatus ADSPortDriver::ADSDisconnect(asynUser *pasynUser) {
    LOG_TRACE_ASYN(pasynUser, "Entering");
    SumRead.deinitialize();

    // If exitCalled is true, it means the driver is shutting down
    // so unresolving variables doesn't make sense
    if (!exitCalled) {
        if (adsConnection->is_connected() == false) {
            LOG_WARN_ASYN(pasynUser, "Already disconnected");
            return asynSuccess;
        }

        LOG_TRACE_ASYN(pasynUser, "Unresolving ADS read variables");
        adsConnection->unresolve_variables(ads_read_vars);
        adsConnection->unresolve_variables(ads_write_vars);
    }

    LOG_TRACE_ASYN(pasynUser, "Disconnecting from ADS device");
    adsConnection->disconnect();

    adsConnection->set_disconnected();

    return asynSuccess;
}

void ADSPortDriver::adsScan() {
    LOG_TRACE_ASYN(pasynUserSelf, "ADS scan thread starting");

    // used for checking if periodic tasks should be performed
    auto lastADSUpdate = std::chrono::steady_clock::now();

    while (!exitCalled) {

        // get ADS connection status
        bool adsConnected = adsConnection->is_connected();

        if (!initialized) {
            continue;
        }

        if (!adsConnected || !SumRead.is_initialized() ||
            !SumRead.is_allocated()) {

            {
                std::lock_guard<ADSPortDriver> guard(*this);

                auto status = ADSConnect(pasynUserSelf);
                if (status) {
                    ADSDisconnect(pasynUserSelf);
                }
            }

            std::this_thread::sleep_for(waitForConnectionPeriod);
            continue;
        }

        // device info and state should be fetched periodically
        auto timeNow = std::chrono::steady_clock::now();
        std::chrono::duration<double> timeDiff = timeNow - lastADSUpdate;
        if (timeDiff > deviceInfoPeriod) {

            if (readADSDeviceInfo()) {
                continue;
            }
            if (readADSDeviceState()) {
                continue;
            }

            lastADSUpdate = timeNow;
        }

        // perform sum-read and trigger callbacks for I/O intr records
        if (doSumRead()) {
            continue;
        }

        {
            std::lock_guard<ADSPortDriver> guard(*this);
            performIOIntr();
        }

        std::this_thread::sleep_for(sumReadPeriod);
    }

    LOG_TRACE_ASYN(pasynUserSelf, "ADS scan thread exiting");

    if (adsConnection->is_connected()) {
        std::lock_guard<ADSPortDriver> guard(*this);
        ADSDisconnect(pasynUserSelf);
    }
}

asynStatus ADSPortDriver::readADSDeviceInfo() {
    char info[16];
    AdsVersion version;

    asynStatus status = static_cast<asynStatus>(
        adsConnection->read_device_info(info, sizeof(info), &version));

    if (status) {
        LOG_WARN_ASYN(pasynUserSelf, "Cannot read ADS device info");
        ADSDisconnect(pasynUserSelf);
        return status;
    }

    deviceInfo = info;
    adsVersion = version;
    return status;
}

asynStatus ADSPortDriver::readADSDeviceState() {
    uint16_t deviceState;
    ADSState state;

    asynStatus status = static_cast<asynStatus>(
        adsConnection->read_device_state(&deviceState, &state));

    if (status) {
        LOG_WARN_ASYN(pasynUserSelf, "Cannot read ADS device state");
        ADSDisconnect(pasynUserSelf);
        return status;
    }
    currentAdsState = state;
    currentDeviceState = deviceState;
    return status;
}

asynStatus ADSPortDriver::doSumRead() {
    asynStatus status = static_cast<asynStatus>(SumRead.read());

    if (status) {
        LOG_WARN_ASYN(pasynUserSelf, "Cannot perform sum-read");
        ADSDisconnect(pasynUserSelf);
    }

    return status;
}

template <typename PLCDataType, typename epicsDataType>
void ADSPortDriver::performArrayCallbacks(ADSDeviceVar &parentDeviceVar,
                                          unsigned int const &nelem) {

    std::vector<epicsDataType> buffer(nelem);
    Autoparam::Array<epicsDataType> readArray(buffer.data(), buffer.size());

    ArrayReadResult result = arrayRead<PLCDataType, epicsDataType>(parentDeviceVar, readArray);

    doCallbacksArray(parentDeviceVar, readArray, result.status,
                     result.alarmStatus, result.alarmSeverity);
}

void ADSPortDriver::performIOIntr() {

    auto vars = getInterruptVariables();

    for (auto itr = vars.begin(); itr != vars.end(); itr++) {
        auto &adsVar = *static_cast<ADSDeviceVar *>(*itr);
        auto dataType = adsVar.adsPV->addr->get_data_type();
        auto func = adsVar.function();

        if (func.find("[]") != std::string::npos ||
            func.find("STRING") != std::string::npos) {

            auto const &nelem = adsVar.adsPV->addr->get_nelem();

            switch (dataType) {
            case ADSDataType::BOOL:
            case ADSDataType::BYTE:
            case ADSDataType::SINT: {
                performArrayCallbacks<epicsInt8, epicsInt8>(adsVar, nelem);
                break;
            }

            case ADSDataType::INT: {
                performArrayCallbacks<epicsInt16, epicsInt16>(adsVar, nelem);
                break;
            }

            case ADSDataType::DINT: {
                performArrayCallbacks<epicsInt32, epicsInt32>(adsVar, nelem);
                break;
            }
            case ADSDataType::LINT: {
                performArrayCallbacks<epicsInt64, epicsInt64>(adsVar, nelem);
                break;
            }

            case ADSDataType::REAL: {
                performArrayCallbacks<epicsFloat32, epicsFloat32>(adsVar,
                                                                  nelem);
                break;
            }

            case ADSDataType::LREAL: {
                performArrayCallbacks<epicsFloat64, epicsFloat64>(adsVar,
                                                                  nelem);
                break;
            }

            case ADSDataType::USINT: {
                performArrayCallbacks<epicsUInt8, epicsInt8>(adsVar, nelem);
                break;
            }
            case ADSDataType::WORD:
            case ADSDataType::UINT: {
                performArrayCallbacks<epicsUInt16, epicsInt16>(adsVar, nelem);
                break;
            }
            case ADSDataType::DWORD:
            case ADSDataType::UDINT: {
                performArrayCallbacks<epicsUInt32, epicsInt32>(adsVar, nelem);
                break;
            }

            case ADSDataType::STRING: {
                std::vector<char> buffer(nelem);
                Autoparam::Octet readArray(buffer.data(), buffer.size());

                OctetReadResult result = stringRead(adsVar, readArray);

                setParam(adsVar, readArray, result.status, result.alarmStatus,
                         result.alarmSeverity);
                break;
            }
            default:
                // error unknown data type
                break;
            }

        } else if (func.find("_digi") != std::string::npos) {
            switch (dataType) {
            case ADSDataType::BOOL:
            case ADSDataType::BYTE:
            case ADSDataType::USINT: {
                UInt32ReadResult result =
                    digitalRead<epicsUInt8>(adsVar, 0xFFFF);

                setParam(adsVar, result.value, result.status,
                         result.alarmStatus, result.alarmSeverity);
                break;
            }
            case ADSDataType::WORD:
            case ADSDataType::UINT: {
                UInt32ReadResult result =
                    digitalRead<epicsUInt16>(adsVar, 0xFFFF);

                setParam(adsVar, result.value, result.status,
                         result.alarmStatus, result.alarmSeverity);
                break;
            }
            case ADSDataType::DWORD:
            case ADSDataType::UDINT: {
                UInt32ReadResult result =
                    digitalRead<epicsUInt32>(adsVar, 0xFFFF);

                setParam(adsVar, result.value, result.status,
                         result.alarmStatus, result.alarmSeverity);
                break;
            }
            default:
                break;
            }

        } else {
            switch (dataType) {
            case ADSDataType::BOOL:
            case ADSDataType::BYTE:
            case ADSDataType::SINT: {
                Int32ReadResult result =
                    integerRead<epicsInt8, epicsInt32>(adsVar);

                setParam(adsVar, result.value, result.status,
                         result.alarmStatus, result.alarmSeverity);
                break;
            }

            case ADSDataType::INT: {
                Int32ReadResult result =
                    integerRead<epicsInt16, epicsInt32>(adsVar);

                setParam(adsVar, result.value, result.status,
                         result.alarmStatus, result.alarmSeverity);
                break;
            }

            case ADSDataType::DINT: {
                Int32ReadResult result =
                    integerRead<epicsInt32, epicsInt32>(adsVar);

                setParam(adsVar, result.value, result.status,
                         result.alarmStatus, result.alarmSeverity);
                break;
            }

            case ADSDataType::LINT: {
                Int64ReadResult result =
                    integerRead<epicsInt64, epicsInt64>(adsVar);

                setParam(adsVar, result.value, result.status,
                         result.alarmStatus, result.alarmSeverity);

                break;
            }

            case ADSDataType::REAL: {
                Float64ReadResult result = floatRead<epicsFloat32>(adsVar);

                setParam(adsVar, result.value, result.status,
                         result.alarmStatus, result.alarmSeverity);
                break;
            }

            case ADSDataType::LREAL: {
                Float64ReadResult result = floatRead<epicsFloat64>(adsVar);

                setParam(adsVar, result.value, result.status,
                         result.alarmStatus, result.alarmSeverity);
                break;
            }

            case ADSDataType::USINT: {
                Int32ReadResult result =
                    integerRead<epicsInt16, epicsInt32>(adsVar);

                setParam(adsVar, result.value, result.status,
                         result.alarmStatus, result.alarmSeverity);
                break;
            }

            case ADSDataType::WORD:
            case ADSDataType::UINT: {
                Int32ReadResult result =
                    integerRead<epicsInt32, epicsInt32>(adsVar);

                setParam(adsVar, result.value, result.status,
                         result.alarmStatus, result.alarmSeverity);
                break;
            }

            case ADSDataType::DWORD:
            case ADSDataType::UDINT: {
                Int64ReadResult result =
                    integerRead<epicsInt64, epicsInt64>(adsVar);

                setParam(adsVar, result.value, result.status,
                         result.alarmStatus, result.alarmSeverity);
                break;
            }

            default:
                // error unknown data type
                break;
            }
        }
    }
    callParamCallbacks();
}

template <typename PLCDataType, typename epicsDataType>
Result<epicsDataType> ADSPortDriver::integerRead(DeviceVariable &deviceVar) {
    Result<epicsDataType> result;
    auto &info = static_cast<ADSDeviceVar &>(deviceVar);
    auto adsVar = info.adsPV;

    if (adsVar->addr->get_operation() == Operation::Write) {
        result.status = asynError;
        return result;
    }

    PLCDataType val;
    auto status = adsVar->read_from_buffer(sizeof(PLCDataType),
                                           reinterpret_cast<char *>(&val));

    if (status) {
        LOG_ERR_ASYN(info.driver->pasynUserSelf, "Read failed(%i): %s", status,
                     ads_errors[status].c_str());
        result.status = asynError;
        return result;
    }

    result.value = val;
    return result;
}

template <typename PLCDataType>
UInt32ReadResult ADSPortDriver::digitalRead(DeviceVariable &deviceVar,
                                            epicsUInt32 const mask) {
    UInt32ReadResult result;
    auto &info = static_cast<ADSDeviceVar &>(deviceVar);
    auto adsVar = info.adsPV;

    if (adsVar->addr->get_operation() == Operation::Write) {
        result.status = asynError;
        return result;
    }

    PLCDataType val;
    auto status = adsVar->read_from_buffer(sizeof(PLCDataType),
                                           reinterpret_cast<char *>(&val));

    if (status) {
        LOG_ERR_ASYN(info.driver->pasynUserSelf, "Read failed(%i): %s", status,
                     ads_errors[status].c_str());
        result.status = asynError;
        return result;
    }

    result.value = static_cast<epicsUInt32>(val & mask);
    return result;
}

template <typename PLCDataType>
Float64ReadResult ADSPortDriver::floatRead(DeviceVariable &deviceVar) {
    Float64ReadResult result;
    auto &info = static_cast<ADSDeviceVar &>(deviceVar);
    auto adsVar = info.adsPV;

    if (adsVar->addr->get_operation() == Operation::Write) {
        result.status = asynError;
        return result;
    }

    PLCDataType val;
    auto status = adsVar->read_from_buffer(sizeof(PLCDataType),
                                           reinterpret_cast<char *>(&val));

    if (status) {
        LOG_ERR_ASYN(info.driver->pasynUserSelf, "Read failed(%i): %s", status,
                     ads_errors[status].c_str());
        result.status = asynError;
        return result;
    }

    result.value = val;
    return result;
}

template <typename PLCDataType, typename epicsDataType>
WriteResult ADSPortDriver::integerWrite(DeviceVariable &deviceVar,
                                        epicsDataType val) {
    WriteResult result;
    auto &info = static_cast<ADSDeviceVar &>(deviceVar);
    auto adsVar = info.adsPV;

    auto status = adsVar->write(reinterpret_cast<char const *>(&val),
                                sizeof(PLCDataType));

    if (status) {
        LOG_ERR_ASYN(info.driver->pasynUserSelf, "Write failed(%i): %s", status,
                     ads_errors[status].c_str());
        result.status = asynError;
    }

    return result;
}

template <typename PLCDataType>
WriteResult ADSPortDriver::floatWrite(DeviceVariable &deviceVar,
                                      epicsFloat64 val) {
    WriteResult result;
    auto &info = static_cast<ADSDeviceVar &>(deviceVar);
    auto adsVar = info.adsPV;

    PLCDataType rawVal = static_cast<PLCDataType>(val);
    auto status = adsVar->write(reinterpret_cast<char const *>(&rawVal),
                                sizeof(PLCDataType));
    if (status) {
        LOG_ERR_ASYN(info.driver->pasynUserSelf, "Write failed(%i): %s", status,
                     ads_errors[status].c_str());
        result.status = asynError;
    }

    return result;
}

template <typename PLCDataType>
WriteResult ADSPortDriver::digitalWrite(DeviceVariable &deviceVar,
                                        epicsUInt32 val,
                                        epicsUInt32 const mask) {
    WriteResult result;
    auto &info = static_cast<ADSDeviceVar &>(deviceVar);
    auto adsVar = info.adsPV;

    PLCDataType valueToWrite = static_cast<PLCDataType>(val);

    if (mask != 0xFFFFFFFF) {
        // since the mask implies only certain bits should be written to the
        // device the rest must be read before applying the mask
        UInt32ReadResult currentRead =
            digitalRead<PLCDataType>(deviceVar, 0xFFFFFFFF);

        // encode bits
        currentRead.value |= (val & mask);
        currentRead.value &= (val | ~mask);
        valueToWrite = static_cast<PLCDataType>(currentRead.value);
    }

    auto status = adsVar->write(reinterpret_cast<char *>(&valueToWrite),
                                sizeof(PLCDataType));
    if (status) {
        // log error
        LOG_ERR_ASYN(info.driver->pasynUserSelf, "Write failed(%i): %s", status,
                     ads_errors[status].c_str());
        result.status = asynError;
    }

    return result;
}

template <typename PLCDataType, typename epicsDataType>
ArrayReadResult ADSPortDriver::arrayRead(DeviceVariable &deviceVar,
                                         Array<epicsDataType> &val) {
    ArrayReadResult result;
    auto &info = static_cast<ADSDeviceVar &>(deviceVar);
    auto adsVar = info.adsPV;

    if (adsVar->addr->get_operation() == Operation::Write) {
        result.status = asynError;
        return result;
    }

    uint32_t nelem = adsVar->addr->get_nelem();
    size_t bytesToRead = sizeof(PLCDataType) * nelem;
    val.setSize(adsVar->addr->get_nelem());

    if (val.maxSize() < nelem) {
        LOG_ERR_ASYN(
            info.driver->pasynUserSelf,
            "Read failed: val.maxSize=%zu, val.size=%zu, bytesToRead=%zu",
            val.maxSize(), val.size(), bytesToRead);
        result.status = asynError;
        return result;
    }

    auto status = adsVar->read_from_buffer(
        bytesToRead, reinterpret_cast<char *>(val.data()));

    if (status) {
        LOG_ERR_ASYN(info.driver->pasynUserSelf, "Read failed(%i): %s", status,
                     ads_errors[status].c_str());
        result.status = asynError;
        return result;
    }

    return result;
}

template <typename PLCDataType, typename epicsDataType>
WriteResult ADSPortDriver::arrayWrite(DeviceVariable &deviceVar,
                                      Array<epicsDataType> const &val) {
    WriteResult result;
    auto &info = static_cast<ADSDeviceVar &>(deviceVar);
    auto adsVar = info.adsPV;

    size_t bytesToWrite = sizeof(PLCDataType) * adsVar->addr->get_nelem();
    auto status =
        adsVar->write(reinterpret_cast<char *>(val.data()), bytesToWrite);
    if (status) {
        LOG_ERR_ASYN(info.driver->pasynUserSelf, "Write failed(%i): %s", status,
                     ads_errors[status].c_str());
        result.status = asynError;
    }

    return result;
}

OctetReadResult ADSPortDriver::stringRead(DeviceVariable &deviceVar,
                                          Octet &val) {
    OctetReadResult result;
    auto &info = static_cast<ADSDeviceVar &>(deviceVar);
    auto adsVar = info.adsPV;

    if (adsVar->addr->get_operation() == Operation::Write) {
        result.status = asynError;
        return result;
    }

    size_t bytesToRead = adsVar->addr->get_nelem();
    std::vector<char> buffer(bytesToRead);
    auto status = adsVar->read_from_buffer(bytesToRead, buffer.data());
    if (status) {
        LOG_ERR_ASYN(info.driver->pasynUserSelf, "Read failed(%i): %s", status,
                     ads_errors[status].c_str());
        result.status = asynError;
        return result;
    }

    val.fillFrom(buffer.data(), buffer.size());
    return result;
}

WriteResult ADSPortDriver::stringWrite(DeviceVariable &deviceVar,
                                       Octet const &val) {
    WriteResult result;
    auto &info = static_cast<ADSDeviceVar &>(deviceVar);
    auto adsVar = info.adsPV;

    size_t bytesToWrite = adsVar->addr->get_nelem();
    auto status = adsVar->write(val.data(), bytesToWrite);
    if (status) {
        LOG_ERR_ASYN(info.driver->pasynUserSelf, "Write failed(%i): %s", status,
                     ads_errors[status].c_str());
        result.status = asynError;
    }

    return result;
}
