# Copyright (C) 2025  Sutou Kouhei <kou@clear-code.com>
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

FROM archlinux

RUN \
  pacman --sync --refresh --refresh --sysupgrade --noconfirm \
    # mecab-ipadic must have this but it doesn't have it.
    # So we install this in base environment.
    autoconf \
    binutils \
    ccache \
    debugedit \
    # mecab-git must have this but it doesn't have it.
    # So we install this in base environment.
    diffutils \
    fakeroot \
    # mecab-git must have this but it doesn't have it.
    # So we install this in base environment.
    gcc \
    git \
    # mecab-git must have this but it doesn't have it.
    # So we install this in base environment.
    make \
    sudo

RUN \
  useradd --user-group --create-home groonga-normalizer-mysql

RUN \
  echo "groonga-normalizer-mysql ALL=(ALL:ALL) NOPASSWD:ALL" | \
    EDITOR=tee visudo -f /etc/sudoers.d/groonga-normalizer-mysql

USER groonga-normalizer-mysql
WORKDIR /home/groonga-normalizer-mysql

CMD /source/ci/arch-linux/build.sh
