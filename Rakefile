# -*- ruby -*-
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

version = File.read("CMakeLists.txt")[/VERSION "(.*?)"/, 1]

desc "Create source archives"
task :dist do
  base_name = "groonga-normalizer-mysql-#{version}"
  ["tar.gz", "zip"].each do |format|
    sh("git",
       "archive",
       "--format=#{format}",
       "--output=#{base_name}.#{format}",
       "--prefix=#{base_name}/",
       "HEAD")
  end
end
