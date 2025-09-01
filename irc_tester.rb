#!/usr/bin/env ruby
require 'open3'
require 'thread'

TIMEOUT = 1.5 # seconds

  # IRC response codes
  module IRC
    # Registration responses
    RPL_WELCOME = /1/
    RPL_YOURHOST = /2/
    RPL_CREATED = /3/
    RPL_MYINFO = /4/

    RPL_BOUNCE = /5/
    RPL_USERHOST = /302/
    RPL_ISON = /303/
    RPL_AWAY = /301/
    RPL_UNAWAY = /305/
    RPL_NOWAWAY = /306/
    RPL_WHOISUSER = /311/
    RPL_WHOISSERVER = /312/
    RPL_WHOISOPERATOR = /313/
    RPL_WHOISIDLE = /317/
    RPL_ENDOFWHOIS = /318/
    RPL_WHOISCHANNELS = /319/
    RPL_LISTSTART = /321/
    RPL_LIST = /322/
    RPL_LISTEND = /323/
    RPL_CHANNELMODEIS = /324/
    RPL_NOTOPIC = /331/
    RPL_TOPIC = /332/
    RPL_INVITING = /341/
    RPL_SUMMONING = /342/
    RPL_VERSION = /351/
    RPL_WHOREPLY = /352/
    RPL_ENDOFWHO = /315/
    RPL_NAMREPLY = /353/
    RPL_ENDOFNAMES = /366/
    RPL_MOTD = /372/
    RPL_ENDOFMOTD = /376/
    RPL_TIME = /391/
    RPL_HELPSTART = /704/
    RPL_HELPTXT = /705/
    RPL_ENDOFHELP = /706/
    RPL_LOGGEDIN = /900/
    RPL_LOGGEDOUT = /901/
    RPL_SASLSUCCESS = /903/

    # Errors
    ERR_UNKNOWNERROR = /400/
    ERR_NOSUCHNICK = /401/
    ERR_NOSUCHSERVER = /402/
    ERR_NOSUCHCHANNEL = /403/
    ERR_CANNOTSENDTOCHAN = /404/
    ERR_TOOMANYCHANNELS = /405/
    ERR_WASNOSUCHNICK = /406/
    ERR_NOORIGIN = /409/
    ERR_NORECIPIENT = /411/
    ERR_NOTEXTTOSEND = /412/
    ERR_INPUTTOOLONG = /417/
    ERR_UNKNOWNCOMMAND = /421/
    ERR_NOMOTD = /422/
    ERR_NONICKNAMEGIVEN = /431/
    ERR_ERRONEUSNICKNAME = /432/
    ERR_NICKNAMEINUSE = /433/
    ERR_NICKCOLLISION = /436/
    ERR_USERNOTINCHANNEL = /441/
    ERR_NOTONCHANNEL = /442/
    ERR_USERONCHANNEL = /443/
    ERR_NOTREGISTERED = /451/
    ERR_NEEDMOREPARAMS = /461/
    ERR_ALREADYREGISTERED = /462/
    ERR_PASSWDMISMATCH = /464/
    ERR_YOUREBANNEDCREEP = /465/
    ERR_CHANNELISFULL = /471/
    ERR_UNKNOWNMODE = /472/
    ERR_INVITEONLYCHAN = /473/
    ERR_BANNEDFROMCHAN = /474/
    ERR_BADCHANNELKEY = /475/
    ERR_BADCHANMASK = /476/
    ERR_NOPRIVILEGES = /481/
    ERR_CHANOPRIVSNEEDED = /482/
    ERR_CANTKILLSERVER = /483/
    ERR_NOOPERHOST = /491/
    ERR_UMODEUNKNOWNFLAG = /501/
    ERR_USERSDONTMATCH = /502/
    ERR_HELPNOTFOUND = /524/
    ERR_INVALIDKEY = /525/
    ERR_NOPRIVS = /723/
    ERR_NICKLOCKED = /902/
    ERR_SASLFAIL = /904/
    ERR_SASLTOOLONG = /905/
    ERR_SASLABORTED = /906/
    ERR_SASLALREADY = /907/
    ERR_STARTTLS = /691/
    ERR_INVALIDMODEPARAM = /696/
  end

class IrcservTester
  attr_reader :server_stdout, :server_stderr
  def initialize(port: 6667, password: "password")
    @port = port
    @password = password
    @server_stdout = ""
    @server_stderr = ""
    @clients = {}
    @server_pid = nil
    @procedures = {}
    @server_running = false
  end

  def start_server
    return true if @server_running
    puts "Starting IRC server on port #{@port} with password '#{@password}'..."
    system("make") or raise "Failed to compile ircserv!"
    @server_thread = Thread.new do
      Open3.popen3("./ircserv #{@port} #{@password}") do |stdin, stdout, stderr, wait_thread|
        @server_pid = wait_thread.pid
        @server_running = true
        # keep reading to not overflow internal buffer for stdin and stderr
        stdout_thread = Thread.new do
          begin
            stdout.each_line do |line|
              @server_stdout << line; # saving into internal log var
              puts "[SERVER OUT] #{line.chomp}" if ENV['DEBUG'] # printing out
            end
          rescue IOError => e
            puts "[SERVER OUT THREAD] Closed: #{e.message}" if @server_running && ENV['DEBUG']
          end
        end

        stderr_thread = Thread.new do
          begin
            stderr.each_line do |line|
              @server_stderr << line;
              puts "[SERVER ERR] #{line.chomp}" if ENV['DEBUG']
            end
          rescue IOError => e
            puts "[SERVER OUT THREAD] Closed: #{e.message}" if @server_running && ENV['DEBUG']
          end
        end

        exit_status = wait_thread.value
        puts "[SERVER EXIT] status: #{exit_status}" if ENV['DEBUG']
      end
    end
    # wait time for server to start up
    sleep(1)
    puts "Server started with PID: #{@server_pid}"
    return true
  rescue => e
    puts "Failed to start server: #{e.message}"
    @server_running = false
    return false
  end

  # connect new client
  def connect_client(client_id)
    return false if @clients[client_id]
    puts "Connecting client #{client_id} to ircserv with netcat..."
    stdin, stdout, stderr, wait_thread = Open3.popen3("nc localhost #{@port}")
    client = {
      id: client_id,
      stdin: stdin,
      stdout: stdout,
      stderr: stderr,
      pid: wait_thread.pid,
      responses: [],
      response_mutex: Mutex.new
    }
    # reading responses inside seperate thread
    Thread.new do
      begin
        stdout.each_line(chomp: true) do |line|
          client[:response_mutex].synchronize do 
            client[:responses] << line
          end
          puts "[CLIENT #{client_id} IN] #{line}" if ENV['DEBUG']
        end
      rescue IOError => e
        puts "[CLIENT #{client_id} IN THREAD] Closed: #{e.message}" if @server_running && ENV['DEBUG']
      end
    end

    @clients[client_id] = client
    puts "Client #{client_id} connected with PID: #{client[:pid]}"
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
      Process.kill("TERM", client[:pid]) rescue nil
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
    puts "[CLIENT #{client_id} OUT] #{command}"
    client[:stdin].puts(command)
    sleep(0.1) #time for server to process
    return true
  end

# sees if client received a message pattern
  def client_received?(client_id, pattern, timeout = TIMEOUT)
    client = @clients[client_id]
    return false unless client
    start_time = Time.now
    while (Time.now - start_time) < timeout
      client[:response_mutex].synchronize do
        client[:responses].each do |response|
          if response.match?(pattern)
            puts "Client #{client_id} received expected pattern: #{pattern}"
            return true
          end
        end
      end
      sleep(0.1)
    end
    puts "Timeout waiting for pattern: #{pattern} from client #{client_id}"
    client[:response_mutex].synchronize do
      puts "Last responses: #{client[:responses].last(5)}" if client[:responses].any?
    end
    return false
  end

  def substitute_variables(text, variables)
    return text unless text && variables
    result = text.dup
    variables.each do |key, value|
      result = result.gsub("$#{key}", value.to_s)
    end
    return result
  end

  # puts steps into common steps under a name
  def define_procedure(name, steps)
    @procedures[name] = steps
    puts "Defined procedure '#{name}' with #{steps.length} steps" if ENV['DEBUG']
  end

  def execute_step(step, variables = {})
    all_variables = (step[:variables] || {}).merge(variables)

    command = substitute_variables(step[:command], all_variables)
    expected = step[:expect]
    timeout = step[:timeout] || TIMEOUT
    client_id = step[:client]

    send_command(client_id, command)
    return true unless expected
    if expected.is_a?(Array)
      return expected.all? do |pattern|
        if pattern.is_a?(String)
          pattern = substitute_variables(pattern, all_variables)
          pattern = Regexp.new(Regexp.excape(pattern)) unless pattern.is_a?(Regexp)
        end
        client_received?(client_id, pattern, timeout)
      end
    else
      if expected.is_a?(String)
        expected = substitute_variables(expected, all_variables)
        expected = Regexp.new(Regexp.excape(expected)) unless expected.is_a?(Regexp)
      end
      client_received?(client_id, expected, timeout)
    end
  end

  def run_procedure(name, client_id_map = {}, variables = {})
    return false unless @procedures[name]
    puts "Running procedure '#{name}'..."
    steps = @procedures[name]
    steps.each do |step|
      # map client from procedure to step
      mapped_step = step.dup
      if client_id_map[step[:client]]
        mapped_step[:client] = client_id_map[step[:client]]
      end
      success = execute_step(mapped_step, variables)
      unless success
        puts "❌ Procedure '#{name}' failed at step: #{step[:command]}"
        return false
      end
    end
    puts "✓ Procedure '#{name}' completed successfully"
    return true
  end

  def run_test_case(test_case)
    puts "\n" + "="*50
    puts "Running test case: #{test_case[:name]}"
    puts "="*50
    # setup all clients needed for this
    test_case[:clients].each do |client_id|
      connect_client(client_id) unless @clients[client_id]
    end

    #exec test case steps
    test_case[:steps].each do |step|
      if step[:procedure]
        client_map = step[:client_map] || {} # care, proceeds with empty if not given
        variables = step[:variables] || {}
        success = run_procedure(step[:procedure], client_map, variables)
        unless success
          puts "  ❌ Test case '#{test_case[:name]}' failed at procedure: #{step[:procedure]}"
          test_case[:clients].each { |client_id| disconnect_client(client_id) } # disconnect clients on failure
          return false
        end
      else
        # regular step
        client_id = step[:client]
        success = execute_step(step)
        unless success
          puts "  ❌ Test case '#{test_case[:name]}' failed at step: #{step[:command]}"
          test_case[:clients].each { |client_id| disconnect_client(client_id) }
          return false
        end
      end
    end
    puts "✅ Test case '#{test_case[:name]}' passed!"
    test_case[:clients].each { |client_id| disconnect_client(client_id) }
    return true
  end

  def run_test_suite(test_cases)
    results = []
    test_cases.each do |test_case|
      result = run_test_case(test_case)
      results << { test_case: test_case[:name], passed: result }
    end
    puts "\n" + "="*20
    puts "TEST SUMMARY:"
    puts "="*20
    total = results.count
    passed = results.count { |result| result[:passed] }
    puts "#{passed}/#{total} test cases passed."
    failed = results.reject { |result| result[:passed] }
    if failed.any?
      puts "\nFailed test cases:"
      failed.each { |result| puts " - #{result[:test_case]}" }
    end
    return results # techically not needed but might be useful
  end

  def cleanup
    puts "Cleaning up.."
    @clients.keys.each do |client_id|
      disconnect_client(client_id)
    end
    if @server_pid && @server_running
      begin
        Process.kill("TERM", @server_pid) rescue nil
        puts "Server terminated"
      rescue => e
        puts "Failed to terminate server: #{e.message}"
      end
    end
    puts "Cleanup complete."
  end

  def setup_common_procedures
    # Register a client
    define_procedure(:register_client, [
      { client: :client, command: "PASS #{@password}", expect: nil },
      { client: :client, command: "NICK $nickname", expect: nil },
      { client: :client, command: "USER $nickname 0 * :Test User", expect: [
        IRC::RPL_WELCOME,
        IRC::RPL_YOURHOST,
        IRC::RPL_CREATED,
        IRC::RPL_MYINFO
      ], timeout: 1.5}
    ])
    #join a test channel
    define_procedure(:join_channel, [
      { client: :client, command: "JOIN $channel", expect: /.+!.+@.+ JOIN / }
    ])
  end
end

# create tester instance
tester = IrcservTester.new()
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
      { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice", password: "password" } }
    ]
  },
  {
    name: "Two Client Registration",
    clients: [:alice, :bob],
    steps: [
      { 
        procedure: :register_client, 
        client_map: { client: :alice }, 
        variables: { nickname: "alice", password: "password" } 
      },
      { 
        procedure: :register_client, 
        client_map: { client: :bob },
        variables: { nickname: "bob", password: "password" } 
      }
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
      # { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "test" } },
      # { procedure: :join_channel, client_map: { client: :bob }, variables: { channel: "test" } },
      { client: :alice, command: "JOIN #test", expect: /.+!.+@.+ JOIN / },
      { client: :bob, command: "JOIN #test", expect: /.+!.+@.+ JOIN / },
      
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
      { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "#test" } },
      { procedure: :join_channel, client_map: { client: :bob }, variables: { channel: "#test" } },
      
      # Send and verify message
      { client: :alice, command: "PRIVMSG #test :Hi!", expect: nil },
      { client: :bob, command: "", expect: /PRIVMSG #test :Hi!/ }
    ]
  }
]

# Run tests
begin
  # Set DEBUG=1 for verbose output
  # ENV['DEBUG'] = '1' 
  if tester.start_server
    tester.setup_common_procedures
    tester.run_test_suite(test_cases)
  end
ensure
  tester.cleanup
end
