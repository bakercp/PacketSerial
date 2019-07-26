# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- None

### Changed

- None

### Removed

- None

### Fixed

- None

### Security

- None

## [1.4.0] 2019-07-26

### Added

- Added `const Stream* getStream() const` and `Stream* getStream()` methods. Thanks @orgicus.

### Changed

- Updated README.

## [1.3.0] 2019-07-25

### Removed

- Remove the `void begin(unsigned long speed, size_t port)` function.
- Remove the `void begin(Stream* stream)` function.
- `while(!SerialX)` line when `CORE_TEENSY` is defined. This was leading to unexpected behavior where programs would not start until a serial connection was opened.

### Added

- Lambda function packetHandler examples and documentation.
- Add `bool overflow() const` to check for a receive buffer overflow.
- Added API documentation @ [http://bakercp.github.io/PacketSerial/](http://bakercp.github.io/PacketSerial/)

### Changed

- Updated README.md, fixed spelling error, added links, docs.
- Rewrote SLIP enum in terms of decimal notation for consistency.
- Updated documentation folder, .github structure.
- Updated CI testing.

## [1.2.0] 2017-11-09

### Added

- An additional PacketHandler pointer type that includes the sender's pointer e.g. `void onPacketReceived(const void* sender, const uint8_t* buffer, size_t size)`. Either functions can be set. Calling `setPacketHandler()` will always remove all previous function pointers.

### Removed

- Deprecated all but one basic `void begin(Stream* stream)` function. Use `void setStream(Stream* stream)` instead.
- Reverted void `PacketSerial_::begin(unsigned long speed, uint8_t config, size_t port)`.

### Changed

- Updated README.md, fixed errors, spelling and updated examples.

## [1.1.0] 2017-11-09

### Added

- Additional inline documentation.
- Added doxygen configuration file.
- Added CHANGELOG.md file.
- Added the `void PacketSerial_::begin(unsigned long speed, uint8_t config, size_t port)` method to avoid confusion with the standard `Serial.begin(unsigned long speed, uint8_t config)`.

### Changed

- Updated README.md, fixed errors, spelling, byte counts, etc.
- Updated documentation / comments in documentation for clarity.

### Removed

- Deprecated the `void begin(unsigned long speed, size_t port)` method because it could be confused with the standard `Serial.begin(unsigned long speed, uint8_t config)` method.

### Fixed

- Fixed Duplicated SLIP END Packet #[11](https://github.com/bakercp/PacketSerial/issues/11)
- Fix types to remove warnings in examples.
- Add `const` qualifier to the `send()` method.

### Security

- None
