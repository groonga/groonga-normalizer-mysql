# README

## Name

groonga-normalizer-mysql

## Description

Groonga-normalizer-mysql is a groonga plugin. It provides MySQL
compatible normalizers to groonga. They are `NormalizerMySQLGeneralCI`
and `NormalizerMySQLUnicodeCI`. `NormalizerMySQLGeneralCI` corresponds
to `utf8mb4_general_ci`.  `NormalizerMySQLUnicodeCI` corresponds to
`utf8mb4_unicode_ci`.

## Install

### Debian GNU/Linux

[Add apt-line for the groonga deb package repository](http://groonga.org/docs/install/debian.html)
and install `groonga-normalizer-mysql` package:

    % sudo aptitude -V -D -y install groonga-normalizer-mysql

### Ubuntu

[Add apt-line for the groonga deb package repository](http://groonga.org/docs/install/ubuntu.html)
and install `groonga-normalizer-mysql` package:

    % sudo aptitude -V -D -y install groonga-normalizer-mysql


### CentOS

Install `groonga-repository` package:

    % sudo rpm -ivh http://packages.groonga.org/centos/groonga-release-1.1.0-1.noarch.rpm
    % sudo yum makecache

Then install `groonga-normalizer-mysql` package:

    % sudo yum install -y groonga-normalizer-mysql

### Fedora

Install `groonga-repository` package:

    % sudo rpm -ivh http://packages.groonga.org/fedora/groonga-release-1.1.0-1.noarch.rpm
    % sudo yum makecache

Then install `groonga-normalizer-mysql` package:

    % sudo yum install -y groonga-normalizer-mysql

### Windows

You need to build from source. Here are build instructions.

#### Build system

Install the following build tools:

* [Microsoft Visual Studio 2010 Express](http://www.microsoft.com/japan/msdn/vstudio/express/): 2012 isn't tested yet.
* [CMake](http://www.cmake.org/)

#### Build groonga

Download the latest groonga source from [packages.groonga.org](http://packages.groonga.org/source/groonga/). Source file name is formatted as `groonga-X.Y.Z.zip`.

Extract the source and move to the source folder:

    > cd ...\groonga-X.Y.Z
    groonga-X.Y.Z>

Run CMake. Here is a command line to install groonga to `C:\groonga` folder:

    groonga-X.Y.Z> cmake . -G "Visual Studio 10 Win64" -DCMAKE_INSTALL_PREFIX=C:\groonga

Build by Visual C++ 2010 Express:

    groonga-X.Y.Z> msbuild groonga.sln /p:Configuration=Release

Install by Visual C++ 2010 Express:

    groonga-X.Y.Z> msbuild groonga.sln /p:Configuration=Release /t:Install

#### Build groonga-normalizer-mysql

Download the latest groonga-normalizer-mysql source from [packages.groonga.org](http://packages.groonga.org/source/groonga-normalizer-mysql/). Source file name is formatted as `groonga-normalizer-X.Y.Z.zip`.

Extract the source and move to the source folder:

    > cd ...\groonga-normalizer-mysql-X.Y.Z
    groonga-normalizer-mysql-X.Y.Z>

IMPORTANT!!!: Set `PKG_CONFIG_PATH` environment variable:

    groonga-normalizer-mysql-X.Y.Z> set PKG_CONFIG_PATH=C:\groongalocal\lib\pkgconfig

Run CMake. Here is a command line to install groonga to `C:\groonga` folder:

    groonga-normalizer-mysql-X.Y.Z> cmake . -G "Visual Studio 10 Win64" -DCMAKE_INSTALL_PREFIX=C:\groonga

Build by Visual C++ 2010 Express:

    groonga-normalizer-mysql-X.Y.Z> msbuild groonga-normalizer-mysql.sln /p:Configuration=Release

Install by Visual C++ 2010 Express:

    groonga-normalizer-mysql-X.Y.Z> msbuild groonga-normalizer-mysql.sln /p:Configuration=Release /t:Install

## Usage

First, you need to register `normalizers/mysql` plugin:

    groonga> register normalizers/mysql

Then, you can use `NormalizerMySQLGeneralCI` and
`NormalizerMySQLUnicodeCI` as normalizers:

    groonga> table_create Lexicon TABLE_PAT_KEY --default_tokenizer TokenBigram --normalizer NormalizerMySQLGeneralCI

## Dependencies

* groonga >= 3.0.3

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
this program is derived work of
`MYSQL_SOURCE/strings/ctype-utf8.c`. This program is the same license
as `MYSQL_SOURCE/strings/ctype-utf8.c` and it is licensed under LGPLv2
only.
