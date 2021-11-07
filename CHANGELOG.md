# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.2] - 2018-01-04

### Fixed

- Radio sequences causing the game to lose focus, which resulted in players not being able to look around
- M16 not playing grenade launch sequence
- Launching M16 grenades before reload sequence is complete
- Firing using the tank no longer plays players' active selected weapon firing sound
- Missing fog as seen in the original mod

## [1.0.1] - 2018-01-03

### Fixed

- Crash when attempting to use the lighter without any weapon
- IRgun not playing correct idle sequences
- Gas ignition trigger correctly when players attempt to use their weapons on map l4m5

## [1.0.0] - 2016-01-12

### Added

- HL: Invasion source code

### Changed

- Removed outro muzzle flash effect to prevent a crash on map l5m5
- Dropped use of FMOD library in favor of Steam Half-Life's built in MP3 player

### Fixed

- Various crashes found in original mod
- Bug where saving and reloading would remove a single item of both stocked healthkit and battery
