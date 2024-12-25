#!/bin/bash
#
# Copyright (C) 2024  Sutou Kouhei <kou@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; version 2
# of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free
# Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
# MA 02110-1301, USA

set -exu

echo "debconf debconf/frontend select Noninteractive" | debconf-set-selections

apt update
apt install -V -y lsb-release wget

distribution=$(lsb_release --id --short | tr 'A-Z' 'a-z')
code_name=$(lsb_release --codename --short)
case "${distribution}" in
  debian)
    component=main
    ;;
  ubuntu)
    component=universe
    ;;
esac
architecture=$(dpkg --print-architecture)

wget https://apache.jfrog.io/artifactory/arrow/${distribution}/apache-arrow-apt-source-latest-${code_name}.deb
apt install -V -y ./apache-arrow-apt-source-latest-${code_name}.deb

wget https://packages.groonga.org/${distribution}/groonga-apt-source-latest-${code_name}.deb
apt install -V -y ./groonga-apt-source-latest-${code_name}.deb
apt update

repositories_dir=/host/packages/apt/repositories
apt install -V -y \
  ${repositories_dir}/${distribution}/pool/${code_name}/${component}/*/*/*_${architecture}.deb

# There are some problems for running arm64 tests:
#   * Too long test time because of QEMU
if [ "${architecture}" == "arm64" ]; then
  exit
fi

cp -a /host/test /test
cd /test

apt install -V -y \
  gcc \
  groonga-bin \
  make \
  ruby-dev \
  tzdata
MAKEFLAGS=-j$(nproc) gem install grntest

grntest_options=()
grntest_options+=(--base-directory=.)
grntest_options+=(--n-retries=2)
grntest_options+=(--reporter=mark)
grntest_options+=(suite)
grntest "${grntest_options[@]}"
