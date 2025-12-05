# SPDX-FileCopyrightText: 1997 Argonne National Laboratory
#
# SPDX-License-Identifier: EPICS

TOP = ..
include $(TOP)/configure/CONFIG

ifeq ($(PVXS),)
    $(warning )
    $(warning Tests require the PVXS module. Test IOC will not be built.)
else
    DIRS += $(wildcard src* *Src*)
    DIRS += $(wildcard db* *Db*)
endif

include $(TOP)/configure/RULES_DIRS
