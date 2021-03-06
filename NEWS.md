# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.4] - 2021-08-10

### Changed

- Greatly simplify the preview code when select file for avatar.
- Just change My Account to Your Account that best reflects that it means.
- Use emblem-default to indicate logged in users instead custom drawing.

### Fixed

- Misc fixes of scan-build reports.

## [0.2.3] - 2021-07-29

### Added

- Add appdata and funding.
- Update requeriments and print build configuration options.

### Fixed

- Fix circle style on some old themes.

## [0.2.2] - 2021-07-26

### Fixed

- Reduce Glib requirements and fix memory leak.
- Don´t try to open an empty string as file path.

## [0.2.1] - 2021-07-26

### Added

- Misc improvements in the layout of the dialogs.
- Allow override the default folder to search for avatar images.
- Allow override the default avatar for new users with an custom file.
- Increase the image size from webcam to 512px wide.

### Fixed

- Fix the single instance to show the dialog in multiple executions.
- Remove unnecessary checks when load the user's image.
- Don't try to embed the dialog in Xfce settings manager.

## [0.2.0] - 2021-07-21

### Added

- Hide unlock infobar when is authorized.
- Move the button to add users below the list.
- Improve the texts of the unock info bar and buttons.
- Add an initial Man page

## [0.1.0] - 2021-07-14

### Added

- Initial public release.
