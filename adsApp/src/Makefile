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

# RWLock.h implements POSIX style R/W lock and needs libpthread
# If building with c++14, boost::shared_mutex is used instead
# Building on Windows requires c++14
ifeq ($(LINUX_USE_CPP11), YES)
USR_CXXFLAGS_Linux += -DLINUX_USE_CPP11
USR_CXXFLAGS_Linux += -std=c++11
ads_SYS_LIBS_Linux += pthread
else
USR_CXXFLAGS_Linux += -std=c++14
ads_SYS_LIBS += boost_thread
endif

USR_CXXFLAGS_WIN32 += -DNOMINMAX

# use ADS from twincat
ifdef TCDIR
USR_CXXFLAGS += -DUSE_TC_ADS
USR_CXXFLAGS += -I$(TCDIR)/3.1/sdk/Include
endif

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

ifndef TCDIR
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
	$(CP) $< $@

endif

ads_LIBS += autoparamDriver
ads_LIBS += asyn
ads_LIBS += $(EPICS_BASE_IOC_LIBS)
ifdef TCDIR
ads_SYS_LIBS_WIN32 += $(TCDIR)/AdsApi/TcAdsDll/x64/lib/TcAdsDll
endif
ads_SYS_LIBS_WIN32 += ws2_32

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# Explicit dependency needed for generated header file
#ADSPortDriver$(DEP): $(COMMON_DIR)/$(GENVERSION)

