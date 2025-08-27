#!/usr/bin/env ruby
require 'open3'

class IrcservTester
  attr_reader :s_stdout, :s_stderr
  def initialize(port: 6667, password: "password")
    @port = port
    @password = password
    @s_stdout = ""
    @s_stderr = ""
    @n_stdout = ""
  end
  def start_server
    puts "Starting IRC server on port #{@port} with password '#{@password}'..."
    system("make") or raise "Failed to compile ircserv!"
    @s_thread = Thread.new do
      Open3.popen3("./ircserv #{@port} #{@password}") do |stdin, stdout, stderr, wait_thread|
        @server_pid = wait_thread.pid
    end
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
