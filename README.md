This repository holds Python tools for interacting with NetFlow data. They are
lightweight wrappers on top of the [flow-tools](https://code.google.com/p/flow-tools/)
and [flowd](https://code.google.com/p/flowd/) binaries.

You will need:

* The `flow-tools` package installed (get it from your distribution or from [the source](https://code.google.com/p/flow-tools/))
  and the `flow-export` binary on your path
* The `flowd` package installed (get it from your distribution or from [the source](https://code.google.com/p/flowd/))
  and the `flowd-reader` binary on your path.
* (Python 2.7 only) The `subprocess32` module installed (get it from [PyPi](https://pypi.python.org/pypi/subprocess32/))

Call either `py3flowtools.FlowSet` or `py3flowtools.FlowLog` with the path to a
NetFlow log to get an iterator over the flows in the log:

* Use `FlowSet` for logs that `flow-tools` can read
* Use `FlowLog` for logs that `flowd` can reader

The iterator will return a data structure that exposes these attributes:

* `first` - UTC timestamp
* `last` - UTC timestamp
* `srcaddr` - Source IP address as a string
* `srcaddr_raw` - Source IP address as an integer
* `dstaddr` - Destination IP address as a string
* `dstaddr_raw` - Destination IP address as an integer
* `srcport` - Source port in decimal
* `dstport` - Destination port in decimal
* `prot` - Protocol number, e.g. `6` for TCP and `17` for UDP
* `dOctets` - Number of octets
* `dPkts` - Number of packets
* `tcp_flags` - Integer with the TCP flags 

TODO:


* IPv6 support
* More thorough documentation (sorry!)
* Method to guess which log format is to be used
* Support for gzip-compressed logs

I started with the idea of updating the [pyflowtools](http://code.google.com/p/pyflowtools/)
to support Python 3. However, I decided I wanted to use a more permissive
license and develop more rapidly.
