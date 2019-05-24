# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Added initial Mother Bird & Baby Bird applications
- Added documentation for repo, Mother Bird & Baby Bird
- Only allows watering within an approved time frame
  - Default time frame is 6AM - 6PM CDT
  - Sending message from Particle `updateSettings` will update hours
  - Time frame (default or customized) is saved to EEPROM
  - On startup, approved time frame is set based on values in EEPROM

---



[Unreleased]: https://github.com/michaeljolley/vscode-twitch-themer/compare/8d59aa8...HEAD
