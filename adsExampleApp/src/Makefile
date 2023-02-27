# SPDX-FileCopyrightText: 2022 Cosylab d.d.
#
# SPDX-License-Identifier: MIT-0

TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = adsExample
# adsExample.dbd will be created and installed
DBD += adsExample.dbd

# adsExample.dbd will be made up from these files:
adsExample_DBD += base.dbd

# Include dbd files from all support applications:
adsExample_DBD += asyn.dbd
adsExample_DBD += ads.dbd

# Add all the support libraries needed by this IOC
adsExample_LIBS += ads
adsExample_LIBS += autoparamDriver
adsExample_LIBS += asyn

# adsExample_registerRecordDeviceDriver.cpp derives from adsExample.dbd
adsExample_SRCS += adsExample_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
adsExample_SRCS_DEFAULT += adsExampleMain.cpp
adsExample_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#adsExample_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
adsExample_LIBS += $(EPICS_BASE_IOC_LIBS)
ifdef TCDIR
adsExample_SYS_LIBS_WIN32 += $(TCDIR)/AdsApi/TcAdsDll/x64/lib/TcAdsDll
endif
adsExample_SYS_LIBS_WIN32 += ws2_32

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

