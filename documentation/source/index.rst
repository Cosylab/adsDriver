.. SPDX-FileCopyrightText: 2022 Cosylab d.d.
..
.. SPDX-License-Identifier: MIT

EPICS ADS Device Support User Manual
====================================
**EPICS ADS device support** library is intended to connect EPICS database records to ADS variables and registers on ADS capable devices, also called ADS servers.
`ADS <https://infosys.beckhoff.com/content/1033/tcinfosys3/11291871243.html?id=6446904803799887467>`_ is a client/server interface or a set of commands (further referred as ADS protocol) that is used for communication with ADS capable devices. The basic set of ADS commands is:

* Read/write ADS data.
* Add/remove ADS data notification (subscribtion to value change on the device).
* Read ADS device state and information.

The ADS protocol is encapsulated within the `AMS protocol <https://infosys.beckhoff.com/content/1033/tcinfosys3/11291871243.html?id=6446904803799887467>`_, which is itself usually encapsulated within the TCP protocol.


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   introduction/index
   installation/index
   using/index
   reference-manual/index
   troubleshooting/index

