#!/usr/bin/env ruby
require 'open3'

Dir.chdir('..') do
  puts "Make IRC server..."
  stdout, stderr, status = Open3.capture3("make")

  puts "Make result stdout: #{stdout}"
  puts "Make result stderr: #{stderr}" if stderr.length > 0
  puts "Make exit status: #{status.exitstatus}"
end


Dir.chdir('..') do
  system("make") or raise "Failed to compile ircserv!"
end
