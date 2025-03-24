# -*- ruby -*-
#
# Copyright (C) 2024-2025  Sutou Kouhei <kou@clear-code.com>
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

namespace :release do
  namespace :version do
    desc "Update versions for a new release"
    task :update do
      cd("packages") do
        ruby("-S", "rake", "version:update")
      end
      sh("git",
         "add",
         "packages/debian/changelog",
         "packages/yum/groonga-normalizer-mysql.spec.in")
      sh("git", "commit", "-m", "packages: update versions")
      sh("git", "push")
    end
  end

  desc "Tag"
  task :tag do
    latest_news = "doc/text/news.md"
    latest_release_note = File.read(latest_news).split(/^## /)[1]
    latest_release_note_version = latest_release_note.lines.first[/[\d.]+/]
    if latest_release_note_version != version
      raise "release note isn't written"
    end

    changelog = "packages/debian/changelog"
    case File.readlines(changelog)[0]
    when /\((.+)-1\)/
      package_version = $1
      unless package_version == version
        raise "package version isn't updated: #{package_version}"
      end
    else
      raise "failed to detect deb package version: #{changelog}"
    end

    sh("git",
       "tag",
       "v#{version}",
       "-a",
       "-m",
       "groonga-normalizer-mysql #{version}!!!")
    sh("git", "push", "origin", "v#{version}")
  end
end

namespace :dev do
  namespace :version do
    desc "Bump version for new development"
    task :bump do
      new_version = ENV["NEW_VERSION"]
      raise "NEW_VERSION environment variable is missing" if new_version.nil?
      cmake_lists_txt_content = File.read("CMakeLists.txt")
      cmake_lists_txt_content.gsub!(/VERSION ".+?"/) do
        "VERSION \"#{new_version}\""
      end
      File.write("CMakeLists.txt", cmake_lists_txt_content)
      sh("git", "add", "CMakeLists.txt")
      sh("git", "commit", "-m", "Bump version")
      sh("git", "push")
    end
  end
end

desc "Release"
task release: [
  "release:version:update",
  "release:tag",
  "dev:version:bump"
]

namespace :table do
  namespace :generate do
    desc "Generate uca1400 tables"
    task :uca1400 do
      mariadb_build_dir = ENV["MARIADB_BUILD_DIR"]
      if mariadb_build_dir.nil?
        raise "MARIADB_BUILD_DIR environment variable is missing"
      end
      [
        [false, false, "_ai_ci"],
        [false, true, "_ai_cs"],
        [true, false, "_as_ci"],
        [true, true, "_as_cs"],
      ].each do |use_secondary_level, use_tertiary_level, suffix|
        command_line = [
          "tool/generate_uca1400_table.rb",
          "--suffix=#{suffix}",
        ]
        command_line << "--use-secondary-level" if use_secondary_level
        command_line << "--use-tertiary-level" if use_tertiary_level
        command_line << "#{mariadb_build_dir}/strings/ctype-uca1400data.h"
        ruby(*command_line,
             out: "normalizers/mysql_unicode_1400#{suffix}_table.h")
      end
    end
  end
end
