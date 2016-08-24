LIBMSR {#mainpage}
==================

Welcome to Libmsr, a friendly (well, friendlier) interface to many of the
model-specific registers in Intel processors. Now with PCI configuration
register support for some Intel hardware.

version 0.3.0


Last Update
-----------
24 August 2016


Webpages
--------
http://software.llnl.gov/libmsr <br>
https://github.com/llnl/libmsr


Overview
--------

Libmsr provides an interface to accessing the model-specific registers (MSRs)
on Intel platforms, which provide privileged functionality for monitoring and
controlling various CPU features.


Installation
------------

Installation is simple. You will need [CMAKE](http://www.cmake.org) version 2.8
or higher and GCC. The old installation method is deprecated, you MUST use the
script. In most cases, the installation is as follows:

	install.sh [ /path/to/install ] [ -f arch_model ]

The install script can take two (optional) arguments. The first argument is the
install directory. If a null or empty string is specified, then a BUILD/
directory is automatically created in the top-level directory (i.e, where
install.sh is launched from).

The second argument can explicitly tell the auto-configuration tool to use the
header file of a specific architecture. To do this, simply add the following
flag when executing the install script: "-f[architecture number]", where the
architecture number is in hexadecimal.

Example:

I have an Ivy Bridge client processor which the auto-configuration tool does not
detect. So, I will force the auto-configuration tool to use the Ivy Bridge
header file defined in platform_headers/.

	install.sh ~/build/libmsr -f3E

Currently supported architectures are Intel Xeon v1-3 (Sandy Bridge, Ivy
Bridge, and Haswell server processors). The library technically supports all
processors based on these architectures, but some features may be missing from
client products. Using the wrong header file is likely to cause problems.

Supported Architectures:

	2D (Sandy Bridge)
	3E (Ivy Bridge)
	3F (Haswell)

If you are unsure of your architecture number, check the "model" field in `lscpu`
or `/proc/cpuinfo` (note that it won't be in hexadecimal).


Notes
-----

This software depends on the files `/dev/cpu/*/msr` being present with R/W
permissions. Recent kernels require additional capabilities. We have found it
easier to use our own [MSR-SAFE](https://github.com/LLNL/msr-safe) kernel
module, but running as root (or going through the bother of adding the
capabilities to the binaries) is another option.

If you need PCI configuration register (CSR) support in Libmsr, you MUST have
CSR-SAFE installed. This code is not currently on Github -- you will need to
request it.

Call `msr_init()` before using any of the APIs.

For sample code, see libmsr_test.c in the test/ directory.

Our most up-to-date documentation for Libmsr use is generated as part of the
install script using Doxygen. There are also some useful PDF files in the
documentation/ directory.

If you wish to use Libmsr on LLNL's Cab system, you will need to apply a patch
to gain MSR access. At this time, the patch will need to be requested as it is
not on Github.

Contributing
------------

Code formatting can be automated using astyle with the following parameters:

    astyle --style=allman --indent=spaces=4 -y -S -C -N <src_file>


Contact
-------

Barry Rountree, Project Lead, <rountree@llnl.gov> <br>
Kathleen Shoga, Developer, <shoga1@llnl.gov> <br>
Scott Walker, Developer, <walker91@llnl.gov> <br>
Lauren Morita, Developer, <morita4@llnl.gov> <br>
Stephanie Labasan, Developer, <labasan1@llnl.gov>

Please feel free to contact the developers with any questions or feature
requests.
