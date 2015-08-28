libmsr
====================

Welcome to libmsr, a friendly (well, friendlier) interface to several
of the model-specific registers in Intel processors.


Installation
---------------------

Installation is simple.  You will need [cmake](http://www.cmake.org)
version 2.8 or higher.

    mkdir BUILD && cd BUILD
    cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install
    make
    make install

Notes
----------------------

This software depends on the files `/dev/cpu/*/msr` being present with
r/w permissions.  Recent kernels require additional capabilities.  We
have found it easier to use our own msr-safe kernel module, but
running as root (or going through the bother of additing the
capabilities to the binaries) are other options.

Call `msr_init()` before using any of the APIs.

For sample code, see libmsr_test.c in the test folder.

Our most up-to-date documentation for libmsr use is the pdf files in the documentation folder, however there
is some additional useful information in API as well.

Please feel free to contact the authors with questions and suggestions.

Authors
---------------------
  * Scott Walker (walker91@llnl.gov)
  * Kathleen Shoga (shoga1@llnl.gov)
  * Barry Rountreee (rountree@llnl.gov)
  * Lauren Morita (morita4@llnl.gov)
