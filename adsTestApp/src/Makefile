# SPDX-FileCopyrightText: 2003 Argonne National Laboratory
#
# SPDX-License-Identifier: EPICS

TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = adsTest
# adsTest.dbd will be created and installed
DBD += adsTest.dbd

# adsTest.dbd will be made up from these files:
adsTest_DBD += base.dbd

# Include dbd files from all support applications:
adsTest_DBD += asyn.dbd
adsTest_DBD += ads.dbd
adsTest_DBD += pvxsIoc.dbd

# Add all the support libraries needed by this IOC
adsTest_LIBS += ads
adsTest_LIBS += asyn
adsTest_LIBS += autoparamDriver
adsTest_LIBS += pvxsIoc pvxs

# adsTest_registerRecordDeviceDriver.cpp derives from adsTest.dbd
adsTest_SRCS += adsTest_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
adsTest_SRCS_DEFAULT += adsTestMain.cpp
adsTest_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#adsTest_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
adsTest_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

