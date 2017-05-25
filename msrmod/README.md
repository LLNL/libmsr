v0.0 alpha
This tool is in alpha and has not been fully tested. Additionally, updates will
be sparse.

EXAMPLES
========
To set a package-level power limit on socket 0 (add -v for a verbose print out):

    ./msrmod -s package -c 0 -a 100 -b 1 -e 120 -f 3 (-v)

To restore default power limits on socket 1:

    ./msrmod -s default -c 1

To see current package-level and dram-level power limits:

    ./msrmod -p rapl

To explicitly write a new hex value into register MSR_PKG_POWER_LIMIT 0x610 on
CPU 3 (i.e., socket 0):

    ./msrmod -w 610 -t 3 -d 7845000158320
