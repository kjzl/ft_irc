#!/usr/bin/env ruby
require 'open3'

system("make") or raise "Failed to compile ircserv!"
timeout = 5

puts "Starting server for #{timeout} seconds"
Open3.popen3("./ircserv 6667 password") do |stdin, stdout, stderr, wait_thread|
  server_pid = wait_thread.pid
  puts "Server started with PID: #{server_pid}"
  # exit_status = wait_thread.value
  sleep(5)
  Process.kill("TERM", server_pid)
  puts "Server terminated"
end
