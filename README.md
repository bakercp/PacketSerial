# PacketSerial

> An Arduino Library that facilitates packet-based serial communication using COBS or SLIP encoding.

## Features

_PacketSerial_ is an small, efficient, library that allows [Arduinos](http://www.arduino.cc/) to send and receive serial data packets (with COBS, SLIP or a user-defined encoding) that include bytes of any value (0 - 255). 
A _packet_ is simply an array of bytes.

## Documentation

### Background

See the [background introduction](docs/BACKGROUND.md) page.

### Getting Started

See the [getting started](docs/GETTING_STARTED.md) page.

### Examples

Find the examples [here](examples).

### API

See the API [documentation](https://bakercp.github.com/PacketSerial).

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

[![Build Status](https://travis-ci.org/bakercp/PacketSerial.svg?branch=master)](https://travis-ci.org/bakercp/PacketSerial)

## Versioning

This project uses [Semantic Versioning](http://semver.org/spec/v2.0.0.html), although strict adherence will only come into effect at version 1.0.0.

## Changelog

See the [Changelog](CHANGELOG.md).

## Code of Conduct

See the [Code of Conduct](docs/CODE_OF_CONDUCT.md).

## Contributing

See [Contributing](docs/CONTRIBUTING.md) guide.

## Licensing

See the [License](LICENSE.md).
