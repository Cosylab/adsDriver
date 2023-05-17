.. SPDX-FileCopyrightText: 2022 Cosylab d.d.
..
.. SPDX-License-Identifier: MIT

.. _heading-troubleshooting:

###############
Troubleshooting
###############
IOC fails to connect with "read frame failed with error: Connection reset by peer" errors
=========================================================================================
During IOC boot, when trying to connect to an ADS device the IOC reports *Error: read frame failed with error: Connection reset by peer* errors.

**Example**:

.. code-block::
   :emphasize-lines: 3

   2020-07-10T17:35:28+0200 Info: Connected to 10.68.6.45
   2020/07/10 17:35:28.326 [WARNING] ADSPortDriver.cpp:193 connect(): [ads-test-plan] connected to ADS device (IP: 10.68.6.45)
   2020-07-10T17:35:28+0200 Error: read frame failed with error: Connection reset by peer
   2020-07-10T17:35:28+0200 Info: connection closed by remote
   [WARNING] Connection.cpp:122 resolve_variables(): could not resolve ADS variable 'TestPlan.types_stringin'
   2020/07/10 17:35:33.327 [ERROR] ADSPortDriver.cpp:199 connect(): [ads-test-plan] could not resolve ADS read variable names (1012): ADS device is not connected
   2020-07-10T17:35:33+0200 Error: write frame failed with error: 32
   2020/07/10 17:35:33.328 [ERROR] ADSPortDriver.cpp:735 ads_comm_task(): [ads-test-plan] error reading ADS device info (-1): <unknown error>
   iocRun: All initialization complete
   # End of IOC initialization

**Possible reason**:
    The ADS device you are connecting to does not have a configured ADS route for the IOC client.

**Problem solution**:
    Add an ADS route to the ADS device that the IOC is connecting to, see :ref:`using-twincat-setup`.

IOC fails to connect with "Info: connection closed by remote" messages
======================================================================
During IOC boot, when trying to connect to an ADS device the IOC reports *Info: connection closed by remote* messages.

**Example**:

.. code-block::
   :emphasize-lines: 3

   2020-07-10T17:40:52+0200 Info: Connected to 10.68.6.45
   2020/07/10 17:40:52.771 [WARNING] ADSPortDriver.cpp:193 connect(): [ads-test-plan] connected to ADS device (IP: 10.68.6.45)
   2020-07-10T17:40:52+0200 Info: connection closed by remote
   [WARNING] Connection.cpp:122 resolve_variables(): could not resolve ADS variable 'TestPlan.types_stringin'
   2020/07/10 17:40:57.777 [ERROR] ADSPortDriver.cpp:199 connect(): [ads-test-plan] could not resolve ADS read variable names (1012): ADS device is not connected

**Possible reason**:
    The PLC is running in *Configuration* mode instead of *Run* mode.

**Problem solution**:
    Activate configuration and run the PLC with TwinCAT XAE, see :ref:`using-twincat-setup`.

IOC fails to connect with "Connect TCP socket failed with: 111" errors
======================================================================
During IOC boot, when trying to connect to an ADS device the IOC reports *Error: Connect TCP socket failed with: 111* errors.

**Example**:

.. code-block::
   :emphasize-lines: 1

   2020-07-10T17:46:53+0200 Error: Connect TCP socket failed with: 111
   [ERROR] Connection.cpp:42 connect(): could not add ADS rout (6): target port not found
   2020/07/10 17:46:53.314 [ERROR] ADSPortDriver.cpp:190 connect(): [ads-test-plan] could not connect to ADS device (1012): ADS device is not connected

**Possible reason**:
    The return code *111* of *Connect TCP socket* resolves into a **connection refused error**. The IP address that the IOC is connecting is reachable, but the ADS server is not listening on the IP port.

**Problem solution**:
    Make sure that the IOC is connecting to a valid ADS device. If connecting to a TwinCAT XAE ADS device, make sure that it is in the Active (running) mode and that the firewall permits ADS traffic.

IOC fails to connect with "Connect TCP socket failed with 110; Adding ADS route failed" error
=============================================================================================
During IOC boot, when trying to connect to an ADS device the IOC reports *Connect TCP socket failed with: 110* error.

**Example**:

.. code-block::
   :emphasize-lines: 1

   2020-07-10T17:56:18+0200 Error: Connect TCP socket failed with: 110
   [ERROR] Connection.cpp:42 connect(): could not add ADS rout (6): target port not found
   2020/07/10 17:56:18.095 [ERROR] ADSPortDriver.cpp:190 connect(): [ads-test-plan] could not connect to ADS device (1012): ADS device is not connected

**Possible reason**:
    The return code *110* of the *Connect TCP socket failed* error usually resolves into a **connection timed out** error.

**Problem solution**:
    Make sure that the IOC is connecting to an existing ADS device. If connecting to a TwinCAT XAE ADS device, make sure that it is in the Active (running) mode and that the firewall permits ADS traffic.

IOC loses ADS connection with "read frame failed with error: Connection reset by peer" and "connection closed by remote" messages
=================================================================================================================================
During IOC runtime, the ADS connection is suddenly lost, followed by *Error: read frame failed with error: Connection reset by peer* and *Info: connection closed by remote* messages.

**Example**:

.. code-block::
   :emphasize-lines: 1,2

   2020-07-14T14:02:33+0200 Error: read frame failed with error: Connection reset by peer
   2020-07-14T14:02:33+0200 Info: connection closed by remote
   2020/07/14 14:02:38.768 [WARNING] ADSPortDriver.cpp:776 ads_comm_task(): [port-ads] sum-read failed due to no connection to ADS device
   2020-07-14T14:02:38+0200 Error: write frame failed with error: 32
   2020-07-14T14:02:38+0200 Error: write frame failed with error: 32

**Possible reason**:
    Another ADS client from the same IP address and using the same AMS ID may have open a new connection to the ADS device.

**Problem solution**:
    Make sure there is no other ADS client using the same AMS ID running on the same host.

IOC fails to connect with "Connect TCP socket failed with 113; Adding ADS route failed" error
=============================================================================================
During IOC boot, when trying to connect to an ADS device the IOC first reports *Connect TCP socket failed with: 113*, followed by *Adding ADS route failed, did you specify valid address* error.

.. code-block::
   :emphasize-lines: 1

   2018-09-07T11:17:52+0200 Error: Connect TCP socket failed with: 113
   [ERROR] Connection.cpp:42 connect(): could not add ADS rout (6): target port not found
   2020/07/10 17:56:18.095 [ERROR] ADSPortDriver.cpp:190 connect(): [ads-test-plan] could not connect to ADS device (1012): ADS device is not connected

**Possible reason**:
    The return code *113* of the *Connect TCP socket failed* error usually resolves into a **no route to host** error. The IP address that the IOC is trying to connect to does not exist or is not reachable.

**Problem solution**:
    Make sure that the ADS device IP address is correct and that it is reachable from the IOC.

IOC connects but prints "Port 0x2710 is out of range" and "No response pending" warnings
========================================================================================
During IOC runtime, you can see it connect to the PLC but periodically prints *Port 0x2710 is out of range* and *No response pending* warnings.

.. code-block::
   :emphasize-lines: 1

   2020-07-10T18:02:03+0200 Warning: Port 0x2710 is out of range
   2020-07-10T18:02:03+0200 Warning: No response pending

**Possible reason**:
    One known reason is that on the ADS device, TwinCAT XAE is active.

**Problem solution**:
    Either ignore the warnings or close TwinCAT XAE on the target ADS device. The PLC program should still continue to run normally.
