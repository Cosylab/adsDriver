# ADS Driver Release Notes

## Version 3.1.0
- Added option for specifying sum read period to `AdsOpen` iocsh command. The default values remains 1 ms.
- Added the default log level flag to `USR_CXXFLAGS`. This flag is mandatory for compiling Beckhoff ADS library since commit `12ec463c`.
- Bumped included Beckhoff ADS library to commit `3824918`.
- C++14 is used by default to compile on Linux. If you want to use C++11, there is a switch in `configure/RELEASE` you can use before compiling. The difference is whether `RWLock` methods use `boost::shared_mutex` (C++14) or `pthread` implementation (C++11).
- Added support for building on Windows, courtesy of STFC UKRI. User can use either the ADS library submodule, or use the library included with TwinCAT installation.

## Version 3.0.0
Initial public release.
