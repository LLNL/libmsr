libmsr
====================

Welcome to libmsr, a friendly (well, friendlier) interface to many
of the model-specific registers in Intel processors. Now with PCI
configuration register support for some Intel hardware.

version 0.2.0


Installation
---------------------

Installation is simple. You will need [cmake](http://www.cmake.org)
version 2.8 or higher and GCC. The old installation method is deprecated,
you MUST use the script. In most cases, the installation is as follows:

	install.sh /path/to/install

The install script can take 2 arguments. The first must be the install directory.
The second can tell the auto configuration tool to use the header files for a specific architecture.
To do this simply put "-f[hexadecimal architecture number]".
Currently supported architectures are Xeon v1-3 (Sandy Bridge, Ivy Bridge, and Haswell processors for servers).
The library technically supports all  processors from these architecture buts some features will be missing from client chips.
Using the wrong header files is likely to cause problems.

Example: 
I have a client Ivy Bridge processor which the autoconfiguration tool does not detect.
So I will force the autoconfiguration tool to use the Ivy Bridge header files.

	install.sh ~/libmsr -f3E

Architectures:
	
	2D (Sandy Bridge)
	3E (Ivy Bridge)
	3F (Haswell)

If you are unsure of your architecture number check the "model" field in lscpu or /proc/cpuinfo (note that it wont be in hexadecimal).


Notes
----------------------

This software depends on the files `/dev/cpu/*/msr` being present with
r/w permissions.  Recent kernels require additional capabilities.  We
have found it easier to use our own [MSR-SAFE](https://github.com/LLNL/msr-safe) kernel module, but
running as root (or going through the bother of additing the
capabilities to the binaries) are other options.

If you need the PCI configuration register (CSR) support in Libmsr, you MUST have CSR-SAFE installed.
This code is not currently on github, you'll have to ask the author for it.

Call `msr_init()` before using any of the APIs.

For sample code, see libmsr_test.c in the test folder.

Our most up-to-date documentation for libmsr use is the pdf files in the documentation folder, however there
is some additional useful information in API as well.



Authors
---------------------
Please feel free to contact the authors with questions, bugs, and feature requests.

  * Scott Walker (walker91@llnl.gov) Lead Developer
  * Barry Rountreee (rountree@llnl.gov) Project Leader
  * Kathleen Shoga (shoga1@llnl.gov) Developer
  * Lauren Morita (morita4@llnl.gov) Developer
