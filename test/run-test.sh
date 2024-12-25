#!/bin/bash
#
# Copyright (C) 2013-2024  Sutou Kouhei <kou@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

set -e

SOURCE_TEST_DIR="$(dirname $0)"
if [ -z "${BUILD_DIR}" ]; then
  BUILD_DIR="$(cd "${SOURCE_TEST_DIR}/.." && pwd)"
fi

if [ -f "${BUILD_DIR}/CMakeCache.txt" ]; then
  cmake --build "${BUILD_DIR}"
fi

GRN_PLUGINS_DIR="${BUILD_DIR}"
export GRN_PLUGINS_DIR

case $(uname) in
  Darwin)
    DYLD_LIBRARY_PATH="${BUILD_DIR}/lib/:${DYLD_LIBRARY_PATH}"
    export DYLD_LIBRARY_PATH
    ;;
  *)
    :
    ;;
esac

if ! type grntest > /dev/null; then
  gem install grntest
fi

have_targets="false"
use_gdb="false"
next_argument_is_long_option_value="false"
for argument in "$@"; do
  case "$argument" in
    --*=*)
    ;;
    --keep-database|--no-*|--version|--help)
    # no argument options
    ;;
    --gdb)
      # no argument options
      use_gdb="true"
      ;;
    --*)
      next_argument_is_long_option_value="true"
      continue
      ;;
    -*)
      ;;
    *)
      if test "$next_argument_is_long_option_value" != "true"; then
	have_targets="true"
      fi
      ;;
  esac
  next_argument_is_long_option_value="false"
done

grntest_options=("$@")
if [ "${use_gdb}" == "true" ]; then
  grntest_options=("--n-workers" "1" "${grntest_options[@]}")
fi
if [ "${CI}" = "true" ]; then
  grntest_options=("--reporter" "mark" "${grntest_options[@]}")
fi
if [ "${have_targets}" != "true" ]; then
  grntest_options+=("${SOURCE_TEST_DIR}/suite")
fi

grntest \
  --base-directory "${SOURCE_TEST_DIR}" \
  "${grntest_options[@]}"
