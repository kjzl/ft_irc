#!/usr/bin/env ruby
require 'open3'
require 'timeout'
require 'thread'

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

  def disconnect_client(client_id)
    client = @clients[client_id]
    return false unless client
    puts "Disconnecting client #{client_id}"
    begin
      # kill the client process
      Process.kill("TERM", client[:pid]) rescue nill
      # close streams (as they were opened with assignment in popen3)
      client[:stdin].close rescue nil
      client[:stdout].close rescue nil
      client[:stderr].close rescue nil
      @clients.delete(client_id)
      return true
    rescue => e
      puts "Failed to disconnect client #{client_id}: #{e.message}"
      return false
    end
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

  # puts steps into common steps under a name
  def define_procedure(name, steps)
    @common_steps[name] = steps
    puts "Defined procedure '#{name}' with #{steps.length} steps"
  end

  def execute_step(step, client_id)
    command = step[:command]
    expected = step[:expect]
    timeout = step[:timeout]
    send_command(client_id, command)
    return true unless expected
    if expected.is_a?(Array)
      expected.all? {|pattern| client_received?(client_id, pattern, timeout)}
    else
      client_received?(client_id, expected, timeout)
    end
  end

  def run_procedure(name, client_id_map = {})
    return false unless @common_steps[name]
    puts "Running procedure '#{name}'..."
    steps = @common_steps[name]
    steps.each do |step|
      client_id = client_id_map[step[:client]] || step[:client] # use clients from map or else the ones from procedure
      success = execute_step(step, client_id)
      unless success
        puts "❌ Procedure '#{name}' failed at step: #{step[:command]}"
        return false
      end
    end
    # puts "✓ Procedure '#{name}' completed successfully"
    return true
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

      success = execute_step(step, client_id)
      unless success
        puts "  ❌ Test case '#{test_case[:name]}' failed at step: #{command}"
        test_case[:clients].each do |client_id|
          disconnect_client(client_id) # disconnect client on failure
        return false
      end
    end
    puts "✅ Test case '#{test_case[:name]}' passed!"
    test_case[:clients].each do |client_id|
      disconnect_client(client_id)
    return true
  end

  def run_test_suite(test_cases)
    retults = []
    test_cases.each do |test_case|
      result = run_test_case(test_case)
      results << { test_case: test_case[:name], passed: result }
    end

    puts "\n------------------------------"
    puts "Test Summary:"
    puts "------------------------------"
    total = results.count
    passed = results.count { |case| r[:passed] }
    puts "#{passed}/#{total} test cases passed."
    failed = results.reject { |case| r[:passed] }
    if failed.any?
      puts "\nFailed test cases:"
      failed.each { |case| puts " - #{case[:test_case]}"" }
    end
    return results
  end

  def cleanup
    puts "Cleaning up.."
    @clients.keys.each do |client_id|
      dicsconnect_client(client_id)
    end
    if @server_pid
      begin
        Process.kill("TERM", @server_pid) rescue nil
        puts "Server terminated"
      rescue => e
        puts "Failed to terminate server: #{e.message}"
      end
    end
    puts "Cleanup complete."
  end

  def common_procedures(tester)
    # Register a client
    tester.define_procedure(:register_client, [
      { client: :client, command: "PASS #{@password}", expect: nil },
      { client: :client, command: "NICK $nickname", expect: nil },
      { client: :client, command: "USER $nickname 0 * :Test User", expect: [
        IRC::RPL_WELCOME,
        IRC::RPL_YOURHOST,
        IRC::RPL_CREATED,
        IRC::RPL_MYINFO
      ]}
    ])
    #join a test channel
    tester.define_procedure(:join_channel, [
      { client: client, command: "JOIN $channel", expect: /JOIN /}/ }
  end

# define all test cases here
test_cases = [
  # ==========REGISTRATION TESTS==========
  # WRONG PASS or ORDER
  # Wrong password
  # only nick
  # only user
  # Unknown command
  # Pass after nick/user
  # Pass after Nick before user
  # pass after user before nick
  # pass after user/nick
  #
  # WRONG NICK (pass ok)
  # nick with space
  # nick with special chars
  # nick too long
  # nick already in use (needs 2 clients)
  # WRONG USER (Pass, nick ok)
  # 3 args
  # 2 args
  # 1 arg
  # too many args (4)
  {
    name: "Basic Registration",
    clients: [:alice],
    steps: [
      { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } }
    ]
  },
  {
    name: "Join Channel Test",
    clients: [:alice, :bob],
    steps: [
      # Register both clients
      { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
      { procedure: :register_client, client_map: { client: :bob }, variables: { nickname: "bob" } },
      
      # Join channel
      { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "test" } },
      { procedure: :join_channel, client_map: { client: :bob }, variables: { channel: "test" } },
      
      # Additional steps specific to this test
      { client: :bob, command: "", expect: /353.*alice.*bob/ }  # Check NAMES list includes both
    ]
  },
  {
    name: "PRIVMSG Test",
    clients: [:alice, :bob],
    steps: [
      # Register both clients
      { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
      { procedure: :register_client, client_map: { client: :bob }, variables: { nickname: "bob" } },
      
      # Join channel
      { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "test" } },
      { procedure: :join_channel, client_map: { client: :bob }, variables: { channel: "test" } },
      
      # Send and verify message
      { client: :alice, command: "PRIVMSG #test :Hi!", expect: nil },
      { client: :bob, command: "", expect: /PRIVMSG #test :Hi!/ }
    ]
  }
]
