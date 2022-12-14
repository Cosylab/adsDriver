# SPDX-FileCopyrightText: 2022 Cosylab d.d.
#
# SPDX-License-Identifier: MIT-0

TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library
LIBRARY += ads

# install ads.dbd into <top>/dbd
DBD += ads.dbd

EPICS_ADS=$(TOP)/adsApp/src

USR_CXXFLAGS += -std=c++11

# specify all source files to be compiled and added to the library
SRC_DIRS += $(EPICS_ADS)/epics-ads
ads_SRCS += ADSPortDriver.cpp
ads_SRCS += ioc_commands.cpp
ads_SRCS += ADSAddress.cpp
ads_SRCS += Connection.cpp
ads_SRCS += SumReadBuffer.cpp
ads_SRCS += Variable.cpp
ads_SRCS += SumReadRequest.cpp
ads_SRCS += RWLock.cpp
ads_SRCS += Types.cpp
ads_SRCS += err.cpp

# specify beckhoff ads lib source files to be compiled and added
SRC_DIRS += $(EPICS_ADS)/beckhoffAdsLib/AdsLib
ads_SRCS += AdsDef.cpp
ads_SRCS += AdsDevice.cpp
ads_SRCS += AdsLib.cpp
ads_SRCS += AdsFile.cpp
ads_SRCS += LicenseAccess.cpp
ads_SRCS += Log.cpp
ads_SRCS += RouterAccess.cpp
ads_SRCS += RTimeAccess.cpp
ads_SRCS += Sockets.cpp
ads_SRCS += Frame.cpp
ads_SRCS += standalone_AdsLib.cpp
ads_SRCS += standalone_AmsConnection.cpp
ads_SRCS += standalone_AmsNetId.cpp
ads_SRCS += standalone_AmsPort.cpp
ads_SRCS += standalone_AmsRouter.cpp
ads_SRCS += standalone_NotificationDispatcher.cpp

# Because AdsLib.cpp exists both in AdsLib/ and AdsLib/standalone, those in standalone/ are copied and renamed to standalone_*.
$(filter standalone_%,$(ads_SRCS)): standalone_%: $(EPICS_ADS)/beckhoffAdsLib/AdsLib/standalone/%
	cp $< $@


# libpthread is needed for POSIX R/W locks used in RWLock.h
ads_SYS_LIBS += pthread

# ads_LIBS += DynamicAsynPort
ads_LIBS += autoparamDriver
ads_LIBS += asyn
ads_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# Explicit dependency needed for generated header file
#ADSPortDriver$(DEP): $(COMMON_DIR)/$(GENVERSION)

