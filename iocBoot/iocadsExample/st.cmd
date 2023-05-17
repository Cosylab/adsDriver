#!../../bin/linux-x86_64/adsExample

# SPDX-FileCopyrightText: 2022 Cosylab d.d.
#
# SPDX-License-Identifier: MIT-0

#- You may have to change adsExample to something else
#- everywhere it appears in this file

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/adsExample.dbd"
adsExample_registerRecordDeviceDriver pdbbase


# Set local AMS net ID
AdsSetLocalAMSNetID("192.168.122.75.1.1")

# example PLC program connection parameters
epicsEnvSet("PREFIX_ADSEXAMPLE", "ADS-EXAMPLE-01")
epicsEnvSet("PORT_ADSEXAMPLE", "ads-example-port")
epicsEnvSet("IP_ADSEXAMPLE", "192.168.122.146")
epicsEnvSet("AMS_ID_ADSEXAMPLE", "10.0.2.15.1.1")

## Load record instances
dbLoadRecords("db/example.db","P=$(PREFIX_ADSEXAMPLE), PORT=$(PORT_ADSEXAMPLE)")

## Uncomment the following to test 10000 variables
#dbLoadRecords("db/many_vars.db", "P=$(PREFIX_ADSEXAMPLE), PORT=$(PORT_ADSEXAMPLE)")

# Open ADS port
# AdsOpen(port_name,
#         remote_ip_address,
#         remote_ams_net_id,
#         sum_buffer_nelem (default: 500),
#         ads_timeout (default: 500 ms),
#         device_ads_port (default: 851),
#         sum_read_period (default: 1 ms))
AdsOpen("$(PORT_ADSEXAMPLE)", "$(IP_ADSEXAMPLE)", "$(AMS_ID_ADSEXAMPLE)")

# Enable asyn trace output for errors and warnings
asynSetTraceMask("$(PORT_ADSEXAMPLE)", 0, 0x21)
# Alternatively, output everything
#asynSetTraceMask("$(PORT_ADSEXAMPLE)", 0, 0xff)

cd "${TOP}/iocBoot/${IOC}"
iocInit

## Start any sequence programs
#seq sncxxx,"user=vagrant"
