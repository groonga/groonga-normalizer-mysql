# README

## Name

groonga-normalizer-mysql

## Description

Groonga-normalizer-mysql is a Groonga plugin. It provides MySQL
compatible normalizers and a custom normalizers to Groonga.

Here are MySQL compatible normalizers:

* `NormalizerMySQLGeneralCI` for `utf8mb4_general_ci`
* `NormalizerMySQLUnicodeCI` for `utf8mb4_unicode_ci`
* `NormalizerMySQLUnicode520CI` for `utf8mb4_unicode_520_ci`
* `NormalizerMySQLUnicode900` (deprecated by `NormalizerMySQLUnicode`) for:
  * `utf8mb4_0900_ai_ci` (`NormalizerMySQLUnicode900`)
  * `utf8mb4_0900_as_ci` (`NormalizerMySQLUnicode900("weight_level", 2)`)
  * `utf8mb4_0900_as_cs` (`NormalizerMySQLUnicode900("weight_level", 3)`)
  * `utf8mb4_ja_0900_as_cs` (`NormalizerMySQLUnicode900("locale", "ja", "weight_level", 3)`)
  * `utf8mb4_ja_0900_as_cs_ks` (`NormalizerMySQLUnicode900("locale", "ja", "weight_level", 4)`)
* `NormalizerMySQLUnicode` for:
  * `utf8mb4_0900_ai_ci` (`NormalizerMySQLUnicode("version", "9.0.0")`)
  * `utf8mb4_0900_as_ci` (`NormalizerMySQLUnicode("version", "9.0.0", "accent_sensitive", true)`)
  * `utf8mb4_0900_as_cs` (`NormalizerMySQLUnicode("version", "9.0.0", "accent_sensitive", true, "case_sensitive", true)`)
  * `utf8mb4_ja_0900_as_cs` (`NormalizerMySQLUnicode("version", "9.0.0", "accent_sensitive", true, "case_sensitive", true, "locale", "ja")`)
  * `utf8mb4_ja_0900_as_cs_ks` (`NormalizerMySQLUnicode("version", "9.0.0", "accent_sensitive", true, "case_sensitive", true, "locale", "ja", "kana_sensitive", true)`)
  * `utf8mb4_uca1400_ai_ci` (`NormalizerMySQLUnicode("version", "14.0.0")`)
  * `utf8mb4_uca1400_ai_cs` (`NormalizerMySQLUnicode("version", "14.0.0", "case_sensitive", true)`)
  * `utf8mb4_uca1400_as_ci` (`NormalizerMySQLUnicode("version", "14.0.0", "accent_sensitive", true)`)
  * `utf8mb4_uca1400_as_cs` (`NormalizerMySQLUnicode("version", "14.0.0", "accent_sensitive", true, "case_sensistive", true)`)

Here are custom normalizers:

* `NormalizerMySQLUnicodeCIExceptKanaCIKanaWithVoicedSoundMark`
   * It's based on `NormalizerMySQLUnicodeCI`
* `NormalizerMySQLUnicode520CIExceptKanaCIKanaWithVoicedSoundMark`
   * It's based on `NormalizerMySQLUnicode520CI`

They are self-descriptive name but long. They are variant normalizers
of `NormalizerMySQLUnicodeCI` and `NormalizerMySQLUnicode520CI`. They
have different behaviors. The followings are the different
behaviors. They describes with
`NormalizerMySQLUnicodeCIExceptKanaCIKanaWithVoicedSoundMark` but they
are true for
`NormalizerMySQLUnicode520CIExceptKanaCIKanaWithVoicedSoundMark`.

* `NormalizerMySQLUnicodeCI` normalizes all small Hiragana such as `ぁ`,
  `っ` to Hiragana such as `あ`, `つ`.
  `NormalizerMySQLUnicodeCIExceptKanaCIKanaWithVoicedSoundMark`
  doesn't normalize `ぁ` to `あ` nor `っ` to `つ`. `ぁ` and `あ` are
  different characters. `っ` and `つ` are also different characters.
  This behavior is described by `ExceptKanaCI` in the long name.  This
  following behaviors ared described by
  `ExceptKanaWithVoicedSoundMark` in the long name.
* `NormalizerMySQLUnicode` normalizes all Hiragana with voiced sound
  mark such as `が` to Hiragana without voiced sound mark such as `か`.
  `NormalizerMySQLUnicodeCIExceptKanaCIKanaWithVoicedSoundMark` doesn't
  normalize `が` to `か`. `が` and `か` are different characters.
* `NormalizerMySQLUnicode` normalizes all Hiragana with semi-voiced sound
  mark such as `ぱ` to Hiragana without semi-voiced sound mark such as `は`.
  `NormalizerMySQLUnicodeCIExceptKanaCIKanaWithVoicedSoundMark` doesn't
  normalize `ぱ` to `は`. `ぱ` and `は` are different characters.
* `NormalizerMySQLUnicode` normalizes all Katakana with voiced sound
  mark such as `ガ` to Katakana without voiced sound mark such as `カ`.
  `NormalizerMySQLUnicodeCIExceptKanaCIKanaWithVoicedSoundMark` doesn't
  normalize `ガ` to `カ`. `ガ` and `カ` are different characters.
* `NormalizerMySQLUnicode` normalizes all Katakana with semi-voiced sound
  mark such as `パ` to Hiragana without semi-voiced sound mark such as `ハ`.
  `NormalizerMySQLUnicodeCIExceptKanaCIKanaWithVoicedSoundMark` doesn't
  normalize `パ` to `ハ`. `パ` and `ハ` are different characters.
* `NormalizerMySQLUnicode` normalizes all halfwidth Katakana with
  voiced sound mark such as `ｶﾞ` to halfwidth Katakana without voiced
  sound mark such as `ｶ`.
  `NormalizerMySQLUnicodeCIExceptKanaCIKanaWithVoicedSoundMark`
  normalizes all halfwidth Katakana with voided sound mark such as `ｶﾞ`
  to fullwidth Katakana with voiced sound mark such as `ガ`.

`NormalizerMySQLUnicodeCIExceptKanaCIKanaWithVoicedSoundMark` and
`NormalizerMySQLUnicode520CIExceptKanaCIKanaWithVoicedSoundMark`
are MySQL incompatible normalizers but they are useful for Japanese
text. For example, `ふらつく` and `ブラック` has different
means. `NormalizerMySQLUnicodeCI` identifies `ふらつく` with `ブラック`
but `NormalizerMySQLUnicodeCIExceptKanaCIKanaWithVoicedSoundMark`
doesn't identify them.

## Install

### Debian GNU/Linux

[Add apt-line for the Groonga deb package repository](https://groonga.org/docs/install/debian.html)
and install `groonga-normalizer-mysql` package:

    % sudo apt-get -y install groonga-normalizer-mysql

### Ubuntu

[Add apt-line for the Groonga deb package repository](https://groonga.org/docs/install/ubuntu.html)
and install `groonga-normalizer-mysql` package:

    % sudo apt-get -y install groonga-normalizer-mysql

### AlmaLinux 8

Install `groonga-repository` package:

    % sudo dnf install -y https://packages.groonga.org/almalinux/8/groonga-release-latest.noarch.rpm

Then install `groonga-normalizer-mysql` package:

    % sudo dnf install -y --enablerepo=epel groonga-normalizer-mysql

### AlmaLinux 9

Install `groonga-repository` package:

    % sudo dnf install -y https://packages.groonga.org/almalinux/9/groonga-release-latest.noarch.rpm

Then install `groonga-normalizer-mysql` package:

    % sudo dnf install -y --enablerepo=epel groonga-normalizer-mysql

### Amazon Linux 2023

Install `groonga-repository` package:

    % sudo dnf install -y https://packages.groonga.org/amazon-linux/2023/groonga-release-latest.noarch.rpm

Then install `groonga-normalizer-mysql` package:

    % sudo dnf install -y --enablerepo=epel groonga-normalizer-mysql

### macOS - Homebrew

Install `groonga` package (which includes `groonga-normalizer-mysql`):

    % brew install groonga

### Windows

You need to build from source. Here are build instructions.

#### Build system

Install the following build tools:

* [Microsoft Visual C++](https://visualstudio.microsoft.com/vs/features/cplusplus/)
* [CMake](http://www.cmake.org/)

#### Build Groonga

Download the latest Groonga source from [GitHub releases](https://github.com/groonga/groonga/releases/). Source file name is formatted as `groonga-X.Y.Z.zip`.

Extract the source and move to the source folder:

    > cd ...\groonga-X.Y.Z
    groonga-X.Y.Z>

Run CMake. Here is a command line to install Groonga to `C:\groonga` folder:

    groonga-X.Y.Z> cmake . -G "Visual Studio 14 Win64" -DCMAKE_INSTALL_PREFIX=C:\groonga

Build:

    groonga-X.Y.Z> cmake --build . --config Release

Install:

    groonga-X.Y.Z> cmake --build . --config Release --target Install

#### Build groonga-normalizer-mysql

Download the latest groonga-normalizer-mysql source from [GitHub releases](https://github.com/groonga/groonga-normalizer-mysql/releases/). Source file name is formatted as `groonga-normalizer-X.Y.Z.zip`.

Extract the source and move to the source folder:

    > cd ...\groonga-normalizer-mysql-X.Y.Z
    groonga-normalizer-mysql-X.Y.Z>

IMPORTANT!!!: Set `PKG_CONFIG_PATH` environment variable:

    groonga-normalizer-mysql-X.Y.Z> set PKG_CONFIG_PATH=C:\groonga\local\lib\pkgconfig

Run CMake. Here is a command line to install Groonga to `C:\groonga` folder:

    groonga-normalizer-mysql-X.Y.Z> cmake . -G "Visual Studio 14 Win64" -DCMAKE_INSTALL_PREFIX=C:\groonga

Build:

    groonga-normalizer-mysql-X.Y.Z> cmake --build . --config Release

Install:

    groonga-normalizer-mysql-X.Y.Z> cmake --build . --config Release --target Install

## Usage

First, you need to register `normalizers/mysql` plugin:

    groonga> register normalizers/mysql

Then, you can use `NormalizerMySQLGeneralCI` and
`NormalizerMySQLUnicodeCI` as normalizers:

    groonga> table_create Lexicon TABLE_PAT_KEY --default_tokenizer TokenBigram --normalizer NormalizerMySQLGeneralCI

## Dependencies

* Groonga >= 8.0.4

## Mailing list

* English: [groonga-talk@lists.sourceforge.net](https://lists.sourceforge.net/lists/listinfo/groonga-talk)
* Japanese: [groonga-dev@lists.sourceforge.jp](http://lists.sourceforge.jp/mailman/listinfo/groonga-dev)

## Thanks

* Alexander Barkov \<bar@udm.net\>: The author of
  `MYSQL_SOURCE/strings/ctype-utf8.c`.
* ...

## Authors

* Kouhei Sutou \<kou@clear-code.com\>

## License

LGPLv2 only. See doc/text/lgpl-2.0.txt for details.

This program uses normalization table defined in MySQL source code. So
this program is derived work of `MYSQL_SOURCE/strings/ctype-utf8.c`,
`MYSQL_SOURCE/strings/uca900_data.h`,
`MYSQL_SOURCE/strings/uca900_ja_data.h`. This program is the same
license as them and they are licensed under LGPLv2 only.

This program also uses normalization table defined in MariaDB source
code. The table is generated from
https://www.unicode.org/Public/UCA/14.0.0/allkeys.txt . So the
normalization table is licensed under
https://www.unicode.org/copyright.html . It's compatible with LGPLv2
only. So the program can use LGPLv2 only.

## For developers

### How to release

#### Add a release note for a new release

```bash
editor doc/text/news.md
```

#### Update package versions

```bash
APACHE_ARROW_REPOSITORY=${APACHE_ARROW_REPOSITORY_PATH} \
  GROONGA_REPOSITORY=${GROOGNA_REPOSITORY_PATH} \
  rake release:version:update
```

#### Tag

```bash
rake release:tag
```

#### Publish Ubuntu packages

```bash
APACHE_ARROW_REPOSITORY=${APACHE_ARROW_REPOSITORY_PATH} \
  GROONGA_REPOSITORY=${GROOGNA_REPOSITORY_PATH} \
  LAUNCHPAD_UPLOADER_PGP_KEY=${YOUR_GPG_KEY_ID} \
  rake -C packages ubuntu:upload
```

#### Bump version for new development

```bash
rake dev:version:bump NEW_VERSION=${NEW_VERSION}
```
