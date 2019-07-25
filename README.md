# PacketSerial

> An Arduino Library that facilitates packet-based serial communication using COBS or SLIP encoding.

## Features

_PacketSerial_ is an small, efficient, library that allows [Arduinos](http://www.arduino.cc/) to send and receive serial data packets (with COBS, SLIP or a user-defined encoding) that include bytes of any value (0 - 255). 
A _packet_ is simply an array of bytes.

## Documentation

If you're asking _Why do I need this?_, read the [background introduction](docs/BACKGROUND.md) page. If you're ready to get started check out the [getting started](docs/GETTING_STARTED.md) page. You can also learn a lot by reading through the [examples](examples). Finally, if you're interested in learning more about how the code works, read the comments in the [source](src) and check out the [API documentation](https://bakercp.github.com/PacketSerial).

- [Background](docs/BACKGROUND.md)
- [Getting Started](docs/GETTING_STARTED.md)
- [Examples](examples)
- [API Documentation](https://bakercp.github.com/PacketSerial)
- [Source Code](src)

## Support

See the [support](docs/SUPPORT.md) page.

## Compatibility

### Build Status

[![Build Status](https://travis-ci.org/bakercp/PacketSerial.svg?branch=master)](https://travis-ci.org/bakercp/PacketSerial)

### Other Platforms

- openFrameworks (C/C++ on macOS, Windows, Linux, Android, etc).
  - [ofxSerial](https://github.com/bakercp/ofxSerial) addon.
    - See the `ofx::IO::PacketSerial` object which is directly compatible with this library.

- Python
  - [PySerial](https://pythonhosted.org/pyserial/index.html)
    - [COBS](https://pythonhosted.org/cobs/) (see [this discussion](https://github.com/bakercp/PacketSerial/issues/10) for more).
    - [SLIP](https://pypi.python.org/pypi/sliplib/0.0.1)

_Any library that correctly implements a COBS or SLIP encoding scheme should be compatible._

## Project Information

- [Changelog](CHANGELOG.md)
- [Code of Conduct](docs/CODE_OF_CONDUCT.md).
- [Contributing](docs/CONTRIBUTING.md) guide.
- [License](LICENSE.md).
