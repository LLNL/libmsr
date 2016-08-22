This directory contains 3 libmsr based power monitors.

* powmon - Samples and prints power consumption and allocation per socket for
systems with 2 sockets

* power_wrapper_static - Samples and prints power consumption and allocation per
socket for systems with 2 sockets after setting a power cap

* power_wrapper_dynamic - Samples and prints power consumption and allocation
per socket for systems with 2 sockets and adjusts the cap stepwise every 500
ms.

----- Building -----
These tools are built using GNU autotools. You will need to use autotools to
generate the configure script and the configure script to generate the
makefiles for your environment.

To generate the configure script:
autoreconf -if

To generate the makefiles:
./configure --prefix=<install path>

To make and install the binaries:
make install

Each monitor includes a usage string. All three monitors are wrappers around
some other process that will be executing on the node and include logic so that
only one monitor is run per node.
