# PacketSerial

[![Build Status](https://travis-ci.org/bakercp/PacketSerial.svg?branch=master)](https://travis-ci.org/bakercp/PacketSerial)

An Arduino Library that facilitates packet-based serial communication using COBS or SLIP encoding.

## Features

_PacketSerial_ is an small, efficient, library that allows [Arduinos](http://www.arduino.cc/) to send and receive serial data packets (with COBS, SLIP or a user-defined encoding) that include bytes of any value (0 - 255).
A _packet_ is simply an array of bytes.

## Documentation

If you're asking _Why do I need this?_, read the [background introduction](docs/BACKGROUND.md) page. If you're ready to get started, check out the [getting started](docs/GETTING_STARTED.md) page. You can also learn a lot by reading through the [examples](./examples/). Finally, if you're interested in learning more about how the code works, read the comments in the [source code](./src/) and check out the [API documentation](https://bakercp.github.com/PacketSerial).

## Support

If you're looking for help, read the [support](docs/SUPPORT.md) page.

## Compatibility

### Requrements

- [Arduino IDE](https://www.arduino.cc/en/main/software) (version 1.5+)

### Other Platforms

This project has been used successfully with [openFrameworks](https://openframeworks.cc/) using the [ofxSerial](https://github.com/bakercp/ofxSerial) addon. In particular, see the `ofx::IO::PacketSerial` object. Additionally this project has been used with Python using the [PySerial](https://pythonhosted.org/pyserial/index.html) package. In particular, check out the [COBS](https://pythonhosted.org/cobs/) (see [this discussion](https://github.com/bakercp/PacketSerial/issues/10)) and [SLIP](https://pypi.python.org/pypi/sliplib/0.0.1) packages.

Ultimately, any library that correctly implements a COBS or SLIP encoding scheme should be compatible with this project.

### Continuous Integration

Continuous integration tests are carried out on a variety of common Arduino platforms. See [this script](https://raw.githubusercontent.com/adafruit/travis-ci-arduino/master/install.sh) for a list.

## Licensing

This project is licensed under the [MIT License](LICENSE.md).

## Project Management

### Repository

[https://github.com/bakercp/PacketSerial](https://github.com/bakercp/PacketSerial)

### Contributing

If you'd like to contribute to this project, please check out the [Code of Conduct](docs/CODE_OF_CONDUCT.md) and the [contributing](docs/CONTRIBUTING.md) guide.

### Versioning

This project uses [Semantic Versioning](http://semver.org/spec/v2.0.0.html). You can check out recent changes in the [changelog](CHANGELOG.md).
