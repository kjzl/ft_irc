#!/usr/bin/env ruby
require 'open3'
require 'timeout'

class IrcservTester
  attr_reader :server_stdout, :server_stderr
  def initialize(port: 6667, password: "password")
    @port = port
    @password = password
    @server_stdout = "" # honeslty, this does nothing rn, might remove it.
    @server_stderr = ""
    @clients = {}
    @server_pid = nil
  end
  def start_server
    puts "Starting IRC server on port #{@port} with password '#{@password}'..."
    system("make") or raise "Failed to compile ircserv!"
    @server_thread = Thread.new do
      Open3.popen3("./ircserv, #{@port}, #{@password}") do |stdin, stdout, stderr, wait_thread|
        @server_pid = wait_thread.pid
        # keep reading to not overflow internal buffer for stdin and stderr
        stdout_thread = Thread.new do
          stdout.each_line {
            |line| @server_stdout << line; # saving into internal log var
            puts "[SERVER OUT] #{line.chomp}" # printing out
          }
        end
        stderr_thread = Thread.new do
          stderr.each_line {
            |line| @server_stderr << line;
            puts "[SERVER ERR] #{line.chomp}"
          }
        end
        exit_status = wait_thread.value
        puts "[SERVER EXIT] status: #{exit_status}"
      end
    end
    # wait time for server to start up
    sleep(1)
    puts "Server started with PID: #{@server_pid}"
    return true
  rescue => e
    puts "Failed to start server: #{e.message}"
    return false
  end

  # connect new client
  def connect_client(client_id)
    return false if @clients[client_id]
    puts "Connecting client #{client_id} to ircserv with netcat..."
    stdin, stdout, stderr, wait_thread = Open3.popen3("nc localhost, #{@port}")
    client =
    {
      id: client_id,
      stdin: stdin,
      stdout: stdout,
      stderr: stderr,
      pid: wait_thread.pid,
      responses: []
    }
    # reading responses inside seperate thread
      Thread.new do
        stdout.each_line(chomp: true) do |line|
          client[:responses] << line
          puts "[CLIENT #{client_id} IN] #{line}"
        end
      end

      @clients[client_id] = client
      puts "Client #{client_id} connected witd PID: #{client[:pid]}"
      return true
  rescue => e
    puts "Failed to connect client #{client_id}: #{e.message}"
    return false
  end

  # send command from specific client
  def send_command(client_id, command)
    client = @clients[client_id]
    return false unless client
    puts "CLIENT #{client_id} OUT] #{command}"
    client[:stdin].puts(command)
    sleep(0.1) #time for server to processs
    return true
  end

# sees if cleint received a message pattern
  def client_received?(client_id, pattern, timeout = 3)
    client = @clients[client_id]
    return false unless client
    Timeout.timeout(timeout) do
      loop do
        if client[:responses].any? { |response| response.match?(pattern) }
          return true
        end
        sleep(0.1)
      end
    end
  rescue Timeout::Error
    puts "Timeout waiting for pattern: #{pattern} from client #{client_id}"
    puts "Last responses: #{client[:responses].last(3)}"
    return false
  end

  def run_test_case(test_case)
    puts "\nRunning test case: #{test_case[:name]}"
    # setup all clients needed for this
    test_case[:clients].each do |client_id|
      connect_client(client_id) unless @clients[client_id]
    end

    #exec test case steps
    test_case[:steps].each do |step|
      client_id = step[:client]
      command = step[:command]
      expected = step[:expect]
      timeout = step[:timeout]

      send_command(client_id, command)
      if expected
        if expected.is_a?(Array)
          success = expected.all? {|pattern| client_received?(client_id, pattern, timeout)}
        else
          success = client_received?(client_id, pattern, timeout)
        end

        unless success
          puts "❌ Test case '#{test_case[:name]}' failed at step: #{command}"
          return false
        end
      end
    end
    puts "✅ Test case '#{test_case[:name]}' passed!"
    return true
  end
