#!../../bin/linux-x86_64/adsTest

#- SPDX-FileCopyrightText: 2003 Argonne National Laboratory
#-
#- SPDX-License-Identifier: EPICS

#- Test IOC startup script for ADS module testing
#- Uses mock ADS server at localhost:48898

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "$(ADS)/dbd/adsTest.dbd"
adsTest_registerRecordDeviceDriver pdbbase

## Configure ADS connection
# Set local AMS net ID (must be unique on network)
AdsSetLocalAMSNetID("127.0.0.2.1.1")

## Environment variables for macros
epicsEnvSet("PREFIX", "TEST")
epicsEnvSet("PORT", "test-port")
epicsEnvSet("ADS_PORT", "PLC_TC3")

## Open ADS connection to mock server
# AdsOpen(port_name, remote_ip, remote_ams_net_id)
AdsOpen("$(PORT)", "127.0.0.1", "127.0.0.1.1.1")

## Load test databases
dbLoadRecords("$(ADS)/db/test_scalars.db", "P=$(PREFIX),PORT=$(PORT),ADS_PORT=$(ADS_PORT)")
dbLoadRecords("$(ADS)/db/test_waveforms.db", "P=$(PREFIX),PORT=$(PORT),ADS_PORT=$(ADS_PORT)")
dbLoadRecords("$(ADS)/db/test_strings.db", "P=$(PREFIX),PORT=$(PORT),ADS_PORT=$(ADS_PORT)")

## Load asynRecord for connection monitoring
dbLoadRecords("$(ASYN)/db/asynRecord.db", "P=$(PREFIX):,R=asyn,PORT=$(PORT),ADDR=0,OMAX=80,IMAX=80")

## Optional: Enable asyn trace for debugging
# asynSetTraceMask("$(PORT)", 0, 0x21)
# asynSetTraceIOMask("$(PORT)", 0, 0x2)

cd "${TOP}/iocBoot/${IOC}"
iocInit
