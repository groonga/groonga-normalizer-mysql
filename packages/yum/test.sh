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

os=$(cut -d: -f4 /etc/system-release-cpe)
case ${os} in
  amazon)
    os=amazon-linux
    version=$(cut -d: -f6 /etc/system-release-cpe)
    ;;
  *) # For AlmaLinux
    version=$(cut -d: -f5 /etc/system-release-cpe | sed -e 's/\.[0-9]$//')
    ;;
esac

case ${os} in
  amazon-linux)
    DNF="dnf"
    ${DNF} install -y \
      https://apache.jfrog.io/artifactory/arrow/amazon-linux/${version}/apache-arrow-release-latest.rpm
    ;;
  *)
    case ${version} in
      8)
        DNF="dnf --enablerepo=powertools"
        ;;
      *)
        DNF="dnf --enablerepo=crb"
        ${DNF} install -y \
          https://apache.jfrog.io/artifactory/arrow/${os}/${version}/apache-arrow-release-latest.rpm
        ;;
    esac
    ;;
esac

${DNF} install -y \
  https://packages.groonga.org/${os}/${version}/groonga-release-latest.noarch.rpm

repositories_dir=/host/packages/yum/repositories
${DNF} install -y \
  ${repositories_dir}/${os}/${version}/$(arch)/Packages/*.rpm

# There are some problems for running aarch64 tests:
#   * Too long test time because of QEMU
if [ $(arch) == "aarch64" ]; then
  exit
fi

cp -a /host/test /test
cd /test

case ${version} in
  8)
    ${DNF} module disable -y ruby
    ${DNF} module enable -y ruby:3.1
    ${DNF} install -y ruby-devel
    ;;
  *)
    ${DNF} install -y ruby-devel
    ;;
esac

${DNF} install -y \
       gcc \
       groonga \
       make
if [ ${os} != "amazon-linux" ]; then
    ${DNF} install -y redhat-rpm-config
fi
MAKEFLAGS=-j$(nproc) gem install grntest

grntest_options=()
grntest_options+=(--base-directory=.)
grntest_options+=(--n-retries=2)
grntest_options+=(--reporter=mark)
grntest_options+=(suite)
grntest "${grntest_options[@]}"
