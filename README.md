## Introduction

This repository holds Python tools for reading NetFlow data. They are
lightweight wrappers on top of the [flow-tools](https://code.google.com/p/flow-tools/)
and [flowd](https://code.google.com/p/flowd/) binaries. They should be
considered experimental.

These tools are written with Python 3 in mind, but I will keep them working
with Python 2.7 as well.

I test the most with NetFlow v5. If you have samples of other NetFlow logs that
you can share, please send them to me.


## Requirements

You will need:

* The `flow-tools` package installed (get it from your distribution or from [the source](https://code.google.com/p/flow-tools/))
  and the `flow-export` binary on your path
* The `flowd` package installed (get it from your distribution or from [the source](https://code.google.com/p/flowd/))
  and the `flowd-reader` binary on your path.
* (Python 2.7 only) The `subprocess32` module installed (get it from [PyPi](https://pypi.python.org/pypi/subprocess32/))


## Usage

Call one of these functions to a NetFlow log to get an iterator over the flows
in the log:

* Use `NetFlowLog` to have the module try to guess which reader to use
* Use `FlowToolsLog` for logs that `flow-tools` can read
* Use `FlowdLog` for logs that `flowd` can reader

The iterator will return a data structure that exposes these attributes:

* `first` - A naive `datetime.datetime` object. UTC is assumed.
* `last` - A naive `datetime.datetime` object. UTC is assumed.
* `srcaddr` - IP address as a string
* `srcaddr_raw` - IP address as an integer
* `dstaddr` - IP address as a string
* `dstaddr_raw` - IP address as an integer
* `srcport` - Port number in decimal
* `dstport` - Port number in decimal
* `prot` - Protocol number in decimal, e.g. `6` for TCP and `17` for UDP
* `dOctets` - Number of octets
* `dPkts` - Number of packets
* `tcp_flags` - Integer with the TCP flags 


```python
from py3flowtools import FlowToolsLog

parser = FlowToolsLog('flowtools.bin')
for line in parser:
    print(line.dOctets, sep='\t')
```

## TODO list

* More unit tests
* IPv6 support
* More thorough documentation
* Support for gzip-compressed logs

I started with the idea of updating the [pyflowtools](http://code.google.com/p/pyflowtools/)
to support Python 3. However, I decided I wanted to use a more permissive
license and develop more rapidly.
