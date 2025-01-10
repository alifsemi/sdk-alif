.. _zas-connection-ble-rom_abi:

ROM Code ABI
############
Certain decisions have been made when the |**Alif_BLE**| and |**Alif_LC3**| libraries have been compiled.
Things that needs to be considered with the Application Binary Interfaces(**ABI**).

Data Structures and Sizes
*************************
Libraries have been configured to use short enums. For further details please see.

* *-fshort-enums*, check `Options for GCC Code Generation`_
* *-fshort-enums*, check `Options for Clang Code Generation`_

`Arm GNU Toolchain`_ defaults to short enums but when compiling with `LLVM Embedded Toolchain for Arm`_ toolchain it defaults to word sized ones.

Threads and synchronization
***************************
The API provides means to mutual exclusion when accessing the stack from multiple threads.
An application must call the following functions if multiple threads are accessing the host layer:

* alif_ble_mutex_lock()
* alif_ble_mutex_unlock()

|**Alif_LC3**| does not offer primitives for mutual exclusion so it's up to the application developer to protect accessess on the codec.

Floating Point Support
**********************
Both, the host layer and the codec, don't use floating point arithmetics. The codec is `Helium`_ accelerated - which uses the floating point registers - but that does not affect the ABI.
