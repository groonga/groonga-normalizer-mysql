# News

## 1.2.4: 2024-12-25

### Improvements

  * Dropped support for Debian GNU/Linux bullseye.

  * Dropped support for CentOS 7.

  * Added support for Amazon Linux 2023.

## 1.2.3: 2023-11-01

### Fixes

  * Added a file that required for source archive.

## 1.2.2: 2023-10-17

### Improvements

  * Update how to install on macOS. [GitHub #29][Patched by askdkc]
  * Added support for Debian 12 (bookworm).
  * Dropped support for Amazon Linux 2.
  * Added support for Debian trixie.

### Thanks

 * askdkc

## 1.2.1: 2022-12-09

### Improvements

  * Added support for AlmaLinux 8 for ARM64.
  * Added support for AlmaLinux 9 for ARM64.

## 1.2.0: 2022-11-28

### Improvements

  * Added support for building with not under groonga/plugins/

    For example, this feature supports the following structure.

    ```
    mariadb/
      extra/
        groonga/
        groonga-normalizer-mysql/
    ```

## 1.1.9: 2022-10-29

### Improvements

  * Added support for AlmaLinux 9.
  * Added support for Amazon Linux 2.
  * Added support for Ubuntu 22.04 (jammy).
  * Dropped support for CentOS 8.
  * Dropped support for Debian GNU/Linux 10 (buster).
  * Dropped support for Ubuntu 21.10 (impish).

## 1.1.8: 2021-12-29

### Fixes

  * Fixed a bug that out of space for highlight when we use NormalizerMySQLUnicodeCI
    in normalizer on Mroonga and we execute highlight_full() to a text include full-width spaces.
    [GitHub #19][Patched by Takashi Hashida]

## 1.1.5: 2021-11-16

### Improvements

  * Added support for AlmaLinux 8.
  * Added support for Debian GNU/Linux 11 (bullseye)
  * Added support for Debian GNU/Linux 10 (buster) on ARM64.
  * Added support for Ubuntu 20.04 (focal)
  * Added support for Ubuntu 21.04 (hirsute)
  * Added support for Ubuntu 21.10 (impish)
  * Dropped support for CentOS 6.
  * Dropped support for Debian GNU/Linux 9 (stretch)
  * Dropped support for Ubuntu 16.04 (xenial)
  * Dropped support for Ubuntu 19.04 (disco)
  * Dropped support for Ubuntu 19.10 (eoan)

## 1.1.4: 2019-04-03

### Improvements

  * Added support for install pdb with MSVC and CMake build. [GitHub#7]

### Fixes

  * Fixed collation names related to
    NormalizerMySQLUnicode900 (900 -> 0900) in README.md [GitHub#6]

## 1.1.3: 2018-07-18

### Fixes

  * Fixed kana voiced sound mark related conversions for
    `utf8mb4_ja_0900_as_cs`.

## 1.1.2: 2018-07-17

### Improvements

  * Added a new normalizer `NormalizerMySQLUnicode900` for
    `utf8mb4_0900_ai_ci`, `utf8mb4_0900_as_ci`, `utf8mb4_0900_as_cs`,
    `utf8mb4_ja_0900_as_cs` and `utf8mb4_ja_0900_as_cs_ks`.
  * Added support for Debian GNU/Linux stretch.
  * Added support for Ubuntu 18.04.
  * Dropped support for Ubuntu 15.10.
  * Dropped support for Debian GNU/Linux jessie.
  * Dropped support for CentOS 5.

## 1.1.1: 2016-04-29

### Improvements

  * Supported Ubuntu 15.10 and Ubuntu 16.04
  * Dropped Debian 7.0

### Fixes

  * Fixed to install license information when cmake is used.

## 1.1.0: 2015-05-29

### Fixes

  * Fixed a bug that full-width space isn't treated as blank character.
    [groonga-dev,03215] [Reported by Shota Mitsui]

### Thanks

  * Shota Mitsui

## 1.0.9: 2015-03-29

### Improvements

  * Added `NormalizerMySQLUnicode520CI`
  * Added `NormalizerMySQLUnicode520CIExceptKanaCIKanaWithVoicedSoundMark`

## 1.0.8: 2015-02-10

### Fixes

  * Fix registering error when you build with configure.
    [GitHub#3][Reported by Kazuhiko]

### Thanks

  * Kazuhiko
