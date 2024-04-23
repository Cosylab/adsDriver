.. SPDX-FileCopyrightText: 2022 Cosylab d.d.
..
.. SPDX-License-Identifier: MIT

.. _heading-ref-manual:

################
Reference Manual
################
This section describes how to configure database records and the commands available in the IOC shell.

Database record configuration
=============================
Database records are configured as follows:

**DTYP** field:
    specifies the asyn interface (e.g. *asynInt32*, *asynFloat64*, *asynInt8ArrayIn*) and is set according to the record type. Some records allow different asyn interfaces to be set. For more information see asynDriver distribution's ``dbd`` directory, ``devAsyn*.dbd`` (e.g. ``devAsynInt32.dbd``) files. See :ref:`supported-record-types` for a list of supported asyn interfaces.
**INP** and **OUT** fields:
    uses the asyn *drvParams* optional parameter (see `asyn driver manual <https://epics.anl.gov/modules/soft/asyn/>`_, section Generic Device Support for EPICS records) to specify the ADS register/variable that the record is connecting to, number of elements and the operation to be performed (read or write). For example: ``INP=@asyn(plc-01 0 0) USINT R P=300 V=Main.Number`` or ``OUT=@asyn(plc-02 0 0) REAL[] N=10 W P=IO V=Main.SetCalibration``.
**SCAN** field:
    determines when the record is processed and, depending on the record type, how their value is handled:

    * Output records with ``SCAN=Passive/periodic`` write to their corresponding ADS variable immediately when they are processed.
    * Input records with ``SCAN=Passive/periodic`` have their *VAL* field populated with the latest value from their ADS variable's corresponding sum-read data buffers when processed.
    * Input records with ``SCAN=I/O Intr`` have their *VAL* field populated immediately after the port driver detects their corresponding ADS variable's value in the sum-read databa buffer has changed.
**PINI** field:
    Because the driver needs to perform initialization routines after IOC init, this field should be set to 'NO'. If it is set to 'YES', expect errors at IOC start, because of attempted reads before driver is initialized.

.. warning::
   In current ADS port driver version, a large number of simultaneous write requests can saturate the ADS connection and cause the system to become unresponsive and cause records to time out.

The format used to specify the ADS variable in the INP/OUT fields depends if the record targets a scalar variable or array: 
* ``<DATA_TYPE> <OPERATION> P=<PORT> V=<VARIABLE>`` is used for scalars,
* ``<DATA_TYPE>[] N=<NELEM> <OPERATION> P=<PORT> V=<VARIABLE>`` is used for arrays. *STRING* datatype requires N=<NELEM>, but not '[]'.

**DATA_TYPE**:
    specifies one of the supported PLC data types, e.g., *USINT*, *LREAL*, *BOOL*, etc. See :ref:`supported-data-types` for a list of supported PLC data types. If the target variable is an array, append the '[]' to the datatype, except for strings, e.g., *USINT[]*, *LREAL[]*, *STRING*.
**NELEM** (optional):
    is used to specify number of elements for array access, as well as to specify the length of *STRING* PLC variables, e.g., ``N=25``.
**OPERATION**:
    specifies if the PLC variable is read (``R``) or written (``W``).
**PORT**: 
    ADS port in string or numerical format. The same parameter constraints apply as for register access, e.g., ``P=PLC_TC3``.
**VARIABLE**:
    ADS variable name in string format, e.g. ``V=Main.temperature``.

Example variable name specifiers:
---------------------------------
Write an array of 5 UINT (16-bit unsigned int) values to PLC variable named Main.Waveform:
  ``UINT[] N=5 W P=PLC_TC3 V=Main.Waveform``
Write a REAL (32-bit float) value to PLC variable named Main.CorrectionFactor. The lack of [nelem] implies a scalar type:
  ``REAL W P=PLC_TC3 V=Main.CorrectionFactor``
Read 10 BYTE (8-bit int) values from PLC variable named Main.Values:
  ``BYTE[] N=10 R P=PLC_TC3 V=Main.Values``

Example database record configuration:
--------------------------------------

.. code-block::
   
   record(ai, "BECKHOFF:ANALOG_IN:01") {
       field(DESC, "Read first 16bit analog input")
       field(DTYP, "asynInt32")
       field(INP,  "@asyn(beckhoff 0 0) INT R P=IO V=Main.testVar")
   }

Refer to section :ref:`database-examples` for more examples on how to configure database records.

IOC shell commands
==================
This chapter describes the iocsh commands that are provided by the ADS device support software.

.. _iocsh-1:

AdsSetLocalAMSNetId
-------------------
**Description**:
    Set the local AMS net ID of the IOC. This is used in ADS communication to represent the identity of the IOC – the ADS client. This command must be called before any calls to :ref:`iocsh-2`, i.e. before any ADS connections are open.

**Interface**:
    ``AdsSetLocalAMSNetId(ams_net_id)``

**Parameters**:
    ``ams_net_id``: AMS net ID that will be set locally on the IOC, e.g. "192.168.20.10.1.5".

**Example**:

.. code-block::

   # Set the local AMS net ID to "192.168.20.10.1.5".
   AmsSetLocalAMSNetId("192.168.20.10.1.5")

.. _iocsh-2:

AdsOpen
-------
**Description**:
    Configure a new ADS connection. This command must be called before corresponding database records are loaded, i.e. before *dbLoadRecord* is called.

**Interface**:
    ``AdsOpen(port_name, ip_addr, ams_net_id, sum_buffer_nelem, ads_timeout)``

**Parameters**:
    * **port_name**: The port name that is registered with asynManager and is used in the INP/OUT address specifications for the records.
    * **ip_addr**: IP address of the remote ADS device.
    * **ams_net_id**: AMS net ID of the remote ADS device.
    * **sum_buffer_nelem** (optional): The maximum number of PVs that sum-read request and data buffers can contain. Defaults to 500, as per `recommendation by Beckhoff <https://infosys.beckhoff.com/english.php?content=../content/1033/tcsample_vc/html/tcadsdll_api_cpp_sample17.htm&id=5851162267582607595>`_.
    * **ads_timeout** (optional): Current version of the ADS device support (v2.0.0) does not implement *ADS function timeout* feature. ADS client library uses the default value of 5000 ms.

**Example**:

.. code-block::

   # Configure an ADS connection with the optional parameters not specified.
   AdsOpen("plc-01", "10.5.0.115", "10.5.0.115.1.1")
   
   # Configure an ADS connection with all parameters. Here ADS sum operation buffer PV limit is set to 250, ads timeout to 1 second, auto connect is disabled and the default thread priority is used. 
   AdsOpen("plc-02", "10.5.0.120", "10.5.0.120.1.15", 250, 1000)

.. _supported-record-types:

Supported EPICS record types
============================
This table lists EPICS records that are supported by the ADS device support, and asyn interfaces that are available for each record type.

.. table::
   :widths: auto
   
   =================== ======= ======= ================ ==================== =========== ======================= ==========
   asyn interface       ai/ao   bi/bo   longin/longout   stringin/stringout   mbbi/mbbo   mbbiDirect/mbboDirect   waveform 
   =================== ======= ======= ================ ==================== =========== ======================= ==========
   asynInt32              X       X           X                                   X                                        
   asynInt64              X                   X
   asynUInt32Digital              X                                                                 X
   asynFloat64            X                                                                                                
   asynOctet                                                     X                                                         
   asynInt8Array                                                                                                      X    
   asynInt16Array                                                                                                     X    
   asynInt32Array                                                                                                     X    
   asynFloat32Array                                                                                                   X    
   asynFloat64Array                                                                                                   X    
   =================== ======= ======= ================ ==================== =========== ======================= ==========

.. _supported-data-types:

Supported TwinCAT PLC data types 
================================
This table lists asyn interfaces and `TwinCAT PLC data types <https://infosys.beckhoff.com/english.php?content=../content/1033/tcplccontrol/html/tcplcctrl_plc_data_types_overview.htm&id>`_ that the interfaces support.

.. warning::
   ULINT datatype is not supported. 


.. table::
   :widths: auto

   ================== =====================
   asyn interface     supported TwinCAT PLC data types
   ================== =====================
   asynInt32          BOOL, SINT, BYTE, INT, WORD, DINT, USINT, UINT
   asynInt64          LINT, UDINT
   asynUInt32Digital  USINT, UINT, UDINT
   asynFloat64        REAL, LREAL
   asynOctet          STRING
   asynInt8Array      BOOL, SINT, BYTE, USINT
   asynInt16Array     INT, WORD, UINT
   asynInt32Array     DINT, UDINT
   asynFloat32Array   REAL
   asynFloat64Array   LREAL
   ================== =====================

.. _database-examples:

Database record configuration examples
======================================
This section contains examples of database record configurations.

*longin* record read from a 16-bit integer ADS variable:

.. code-block::

   record(longin, "$(P):reg_int32_counter") {
       field(DTYP, "asynInt32")
       field(INP,  "@asyn($(PORT) 0 0) INT R P=350 V=Main.counter")
   }

*longout* record writes to a 32-bit integer ADS variable:

.. code-block::

   record(longout, "$(P):reg_int32_writable") {
       field(DTYP, "asynInt32")
       field(OUT,  "@asyn($(PORT) 0 0) DINT W P=350 V=Main.writable")
   }

*longout* record writes to a 32-bit integer ADS variable and reads current value back. Because ``info(asyn:READBACK, "1")`` is set, the ADS device support will update the output variable if it was changed outside of EPICS:

.. code-block::

   record(longout, "$(P):reg_int32_readwritable") {
       field(DTYP, "asynInt32")
       field(OUT,  "@asyn($(PORT) 0 0) DINT W P=350 V=Main.writable")
       info(asyn:READBACK, "1")
   }

*stringin* record reads from a string ADS variable of length 9. Due to ``SCAN=I/O Intr``, the ADS device support will update the record immediately when it detects that the addressed ADS variable has changed:

.. code-block::

   record(stringin, "$(P):types_stringin") {
       field(DTYP, "asynOctetRead")
       field(SCAN, "I/O Intr")
       field(INP,  "@asyn($(PORT) 0 0) STRING N=9 R P=350 V=Test.types_stringin")
   }

*mbbi* record reads from a 16-bit unsigned integer ADS variable.

.. code-block::

   record(mbbiDirect, "$(P):types_mbbi") {
       field(DTYP, "asynInt32")
       field(INP,  "@asyn($(PORT) 0 0) UINT R P=PLC_TC3 V=Test.types_mbbi")
   }

*waveform* record reads 10 elements of 32-bit unsigned integer ADS variables. Due to ``SCAN=I/O Intr``, the ADS device support will update the record immediately when it detects that the addressed ADS variable has changed.

.. code-block::

   record(waveform, "$(P):types_wf_ulong_in") {
       field(DTYP, "asynInt32ArrayIn")
       field(FTVL, "ULONG")
       field(NELM, "10")
       field(SCAN, "I/O Intr")
       field(INP,  "@asyn($(PORT) 0 0) UDINT[] N=10 R P=PLC_TC3 V=Test.types_wf_ulong_in")
   }

*waveform* record writes 10 elements of 32-bit floating point ADS variables, addressed by port and symbolic name:

.. code-block::

   record(waveform, "$(P):types_wf_float_out") {
       field(DTYP, "asynFloat32ArrayOut")
       field(FTVL, "FLOAT")
       field(PREC, "2")
       field(NELM, "10")
       field(INP,  "@asyn($(PORT) 0 0) REAL[] N=10 W P=PLC_TC3 V=TestPlan.types_wf_float_out")
   }

.. _supported-ads-names:

Supported ADS port names
========================
This table lists the ADS port names that can be specified by name in the record's *INP/OUT* fields (the ``P=`` parameter).


.. table::
   :widths: auto

   ========== =====
   Port name  Value
   ========== =====
   LOGGER     100
   RTIME      200
   TRACE      290
   IO         300
   SPS        400
   NC         500
   ISG        550
   PCS        600
   PLC        801
   PLC_RTS1   801
   PLC_RTS2   811
   PLC_RTS3   821
   PLC_RTS4   831
   PLC_TC3    851
   ========== =====

