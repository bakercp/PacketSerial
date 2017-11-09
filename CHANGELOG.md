# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Removed
- Remove the `void begin(unsigned long speed, size_t port)`.

## [1.1.0] 2017-11-09
### Added
- Additional inline documentation.
- Added doxygen configuration file.
- Added CHANGELOG.md file.
- Added the `void PacketSerial_::begin(unsigned long speed, uint8_t config, size_t port)` method to avoid confusion with the standard `Serial.begin(unsigned long speed, uint8_t config)`.

### Changed
- Updated README.md, fixed errors, spelling, byte counts, etc.

### Removed
- Deprecated the `void begin(unsigned long speed, size_t port)` method because it could be confused with the standard `Serial.begin(unsigned long speed, uint8_t config)` method.

### Fixed
- Fixed Duplicated SLIP END Packet #[11](https://github.com/bakercp/PacketSerial/issues/11)
- Fix types to remove warnings in examples.
- Add `const` qualifier to the `send()` method.

### Security
- None
