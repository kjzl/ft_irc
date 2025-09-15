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
  @bot_pid = nil
  @bot_running = false
  @bot_stdout = ""
  @bot_stderr = ""
  end

  def start_server
    return true if @server_running
    Dir.chdir('..') do
      puts "Make IRC server..."
      system("make") or raise "Failed to compile ircserv!"
      puts "Starting IRC server on port #{@port} with password '#{@password}'..."
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
  end

  def start_bot
    return true if @bot_running
    puts "Starting PollBot..."
    Dir.chdir('..') do
      puts "Make ircbot..."
      system("make ircbot") or raise "Failed to compile ircbot!"
      @bot_thread = Thread.new do
        Open3.popen3("./ircbot") do |stdin, stdout, stderr, wait_thread|
          @bot_pid = wait_thread.pid
          @bot_running = true
          # consume stdout/stderr to avoid blocking
          Thread.new do
            begin
              stdout.each_line do |line|
                @bot_stdout << line
                puts "[BOT OUT] #{line.chomp}" if ENV['DEBUG']
              end
            rescue IOError => e
              puts "[BOT OUT THREAD] Closed: #{e.message}" if @bot_running && ENV['DEBUG']
            end
          end
          Thread.new do
            begin
              stderr.each_line do |line|
                @bot_stderr << line
                puts "[BOT ERR] #{line.chomp}" if ENV['DEBUG']
              end
            rescue IOError => e
              puts "[BOT ERR THREAD] Closed: #{e.message}" if @bot_running && ENV['DEBUG']
            end
          end
          wait_thread.value # wait for bot to exit
        end
      end
      sleep(1)
      puts "PollBot started with PID: #{@bot_pid}"
      return true
    rescue => e
      puts "Failed to start bot: #{e.message}"
      @bot_running = false
      return false
    end
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
    # check connection is alive
    begin
      puts "[CLIENT #{client_id} OUT] #{command}"
      client[:stdin].puts(command)
      sleep(0.1) #time for server to process
      return true
    rescue Errno::EPIPE, IOError => e
      puts "Connection to #{client_id} broken: #{e.message}"
      @clients.delete(client_id)
      return false
    end
  end

# sees if client received a message pattern
  def client_received?(client_id, pattern, timeout = TIMEOUT)
    client = @clients[client_id]
    return false unless client
    start_time = Time.now
    while (Time.now - start_time) < timeout
      unless @clients[client_id]
        puts "Client #{client_id} was disconnected"
        return false
      end
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
    unless @clients[client_id]
      puts "Client was disconnected during wait"
    end
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
          pattern = Regexp.new(Regexp.escape(pattern)) unless pattern.is_a?(Regexp)
        end
        client_received?(client_id, pattern, timeout)
      end
    else
      if expected.is_a?(String)
        expected = substitute_variables(expected, all_variables)
        expected = Regexp.new(Regexp.escape(expected)) unless expected.is_a?(Regexp)
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
    if @bot_pid && @bot_running
      begin
        Process.kill("TERM", @bot_pid) rescue nil
        puts "Bot terminated"
      rescue => e
        puts "Failed to terminate bot: #{e.message}"
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
      { client: :client, command: "JOIN $channel", expect: [
        /.+!.+@.+ JOIN /,
      IRC::RPL_NAMREPLY,
      IRC::RPL_ENDOFNAMES
      ], timeout: 1.5 }
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
      { procedure: :register_client,
        client_map: { client: :alice },
        variables: { nickname: "alice", password: "password" } }
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
  #--------------------------------------------------
  # JOIN TESTS
  {
    name: "Join Channel Test",
    clients: [:alice, :bob],
    steps: [
      # Register both clients
      { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
      { procedure: :register_client, client_map: { client: :bob }, variables: { nickname: "bob" } },

      # Join channel
      # { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "test"} },
      # { procedure: :join_channel, client_map: { client: :bob }, variables: { channel: "test"} },
      { client: :alice, command: "JOIN #test", expect: [
      /alice!.+@.+ JOIN #test/,
      /:.+ 353 alice . #test/,
      /:.+ 366 alice #test :End of \/NAMES list/
      ], timeout: 1.5 },
      { client: :bob, command: "JOIN #test", expect: [
      /bob!.+@.+ JOIN #test/,
      /:.+ 353 bob . #test :.alice/,
      /:.+ 366 bob #test :End of \/NAMES list/
      ], timeout: 1.5 }
    ]
  },
  # Join malformed channelname
  #--------------------------------------------------
  # PRIVMSG TESTS
  # privmsg to client
  {
    name: "PRIVMSG to Client Test",
    clients: [:alice, :bob],
    steps: [
      # Register both clients
      { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
      { procedure: :register_client, client_map: { client: :bob }, variables: { nickname: "bob" } },
      # Join channel
      { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "#test"} },
      { procedure: :join_channel, client_map: { client: :bob }, variables: { channel: "#test"} },
      # Send and verify message
      { client: :alice, command: "PRIVMSG bob :Hi there!", expect: nil },
      { client: :bob, command: "", expect: /:alice!alice@.+ PRIVMSG bob :Hi there!/, timeout: 3 }
    ]
  },
  # privmsg to Channel
  {
    name: "PRIVMSG to Channel Test",
    clients: [:alice, :bob],
    steps: [
      # Register both clients
      { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
      { procedure: :register_client, client_map: { client: :bob }, variables: { nickname: "bob" } },
      # Join channel
      { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "#test"} },
      { procedure: :join_channel, client_map: { client: :bob }, variables: { channel: "#test"} },
      # Send and verify message
      { client: :alice, command: "PRIVMSG #test :Hi there!", expect: nil },
      { client: :bob, command: "", expect: /:alice!alice@.+ PRIVMSG #test :Hi there!/, timeout: 3 }
    ]
  },
  {
    name: "PRIVMSG to nonexistant Client Test",
    clients: [:alice],
    steps: [
      { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
      # Send and verify message
      { client: :alice, command: "PRIVMSG bob :Hi there!", expect: nil },
      { client: :alice, command: "", expect: /401 alice bob / }
    ]
  },
  {
    name: "PRIVMSG to nonexistant Channel Test",
    clients: [:alice],
    steps: [
      { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
      # Send and verify message
      { client: :alice, command: "PRIVMSG #bla :Hi there!", expect: /403/ }
    ]
  },
  {
    name: "PRIVMSG to existant channel, while nobody is in Channel Test",
    clients: [:alice],
    steps: [
      { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
      # Send and verify message
      { client: :alice, command: "PRIVMSG #test :Hi there!", expect: nil },
      { client: :alice, command: "", expect: /404/ }
    ]
  },
  {
    name: "PRIVMSG to Channel while not in Channel Test",
    clients: [:alice, :bob],
    steps: [
      # Register both clients
      { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
      { procedure: :register_client, client_map: { client: :bob }, variables: { nickname: "bob" } },
      # Join channel
      { procedure: :join_channel, client_map: { client: :bob }, variables: { channel: "#test"} },
      # Send and verify message
      { client: :alice, command: "PRIVMSG #test :Hi there!", expect: /404 alice #test/ }
    ]
  },
  #--------------------------------------------------
  # TOPIC TESTS
  {
    name: "TOPIC setting",
    clients: [:alice, :bob],
    steps: [
      # Register both clients
      { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
      { procedure: :register_client, client_map: { client: :bob }, variables: { nickname: "bob" } },
      # Join channel
      { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "#test"} },
      { procedure: :join_channel, client_map: { client: :bob }, variables: { channel: "#test"} },
      # Send and verify message
      { client: :alice, command: "TOPIC #test :Discuss the meaning of life", expect: [
      /:alice!alice@.+ TOPIC #test :Discuss the meaning of life/,
      /.+ TOPIC #test :Discuss the meaning of life/,
      ] },
      { client: :bob, command: "", expect: /:alice!alice@.+ TOPIC #test :Discuss the meaning of life/, timeout: 3 },
    ]
  },
  {
    name: "TOPIC displayed in join reply",
    clients: [:alice, :bob, :steve],
    steps: [
      # Register both clients
      { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
      { procedure: :register_client, client_map: { client: :bob }, variables: { nickname: "bob" } },
      { procedure: :register_client, client_map: { client: :steve }, variables: { nickname: "steve" } },
      # Join channel
      { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "#test"} },
      { procedure: :join_channel, client_map: { client: :bob }, variables: { channel: "#test"} },
      # Send and verify message
      { client: :alice, command: "TOPIC #test :Discuss the meaning of life", expect: [
      /:alice!alice@.+ TOPIC #test :Discuss the meaning of life/,
      /.+ TOPIC #test :Discuss the meaning of life/,
      ] },
      { client: :bob, command: "", expect: /:alice!alice@.+ TOPIC #test :Discuss the meaning of life/, timeout: 3 },
      { procedure: :join_channel, client_map: { client: :steve }, variables: { channel: "#test"} },
      { client: :steve, command: "", expect: /:AspenWood 332 steve #test :Discuss the meaning of life/, timeout: 3 }
    ]
  },
  # setting topic when not operator
  #--------------------------------------------------
  # QUIT TEST
  {
    name: "QUIT sends error message",
    clients: [:alice],
    steps: [
      { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
      # After QUIT we expect a numeric 5 (error-message) response before disconnect.
      { client: :alice, command: "QUIT :Client exiting", expect: /ERROR/, timeout: 1.5 }
    ]
  }
]

# Run tests
begin
  # Set DEBUG=1 for verbose output
  ENV['DEBUG'] = '1'
  if tester.start_server
    tester.start_bot
    tester.setup_common_procedures
    # Append PollBot test cases
  pollbot_tests = [
      {
        name: "PollBot Invite and Instructions",
        clients: [:alice],
        steps: [
          { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
          { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "#test" } },
          # Invite the bot and expect it to JOIN and post instructions
          { client: :alice, command: "INVITE PollBot #test", expect: nil },
          { client: :alice, command: "", expect: [
            /:PollBot!.+@.+ JOIN #test/,
            /PRIVMSG #test :Hi! I can run simple polls\./,
            /PRIVMSG #test :Start: !poll start .+\| .+\| .+/,
            /PRIVMSG #test :Close: !poll close/,
            /PRIVMSG #test :Vote \(private msg to me\): vote #channel <number>/
          ], timeout: 3 }
        ]
      },
      {
        name: "PollBot Start Poll and Vote Requires Channel",
        clients: [:alice],
        steps: [
          { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
          { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "#poll" } },
          { client: :alice, command: "INVITE PollBot #poll", expect: nil },
          { client: :alice, command: "", expect: /:PollBot!.+@.+ JOIN #poll/, timeout: 3 },
          # Start a poll
          { client: :alice, command: "PRIVMSG #poll :!poll start Best color\? | Red | Blue", expect: [
            /PRIVMSG #poll :New poll: Best color\?/,
            /PRIVMSG #poll :1\) Red/,
            /PRIVMSG #poll :2\) Blue/,
            /PRIVMSG #poll :Vote by sending me a private message: vote #channel <number>/
          ], timeout: 3 },
          # Try to vote without channel -> expect usage notice to alice
          { client: :alice, command: "PRIVMSG PollBot :vote 2", expect: /NOTICE alice :Usage: vote #channel <number>/, timeout: 2 },
          # Correct vote with channel -> expect confirmation
          { client: :alice, command: "PRIVMSG PollBot :vote #poll 2", expect: /NOTICE alice :Your vote has been recorded\./, timeout: 2 },
          # Close poll -> expect results
          { client: :alice, command: "PRIVMSG #poll :!poll close", expect: [
            /PRIVMSG #poll :Poll closed\. Results:/,
            /PRIVMSG #poll :1\) Red - 0 vote\(s\)/,
            /PRIVMSG #poll :2\) Blue - 1 vote\(s\)/
          ], timeout: 3 }
        ]
      },
      {
        name: "PollBot Invalid Option and No Open Poll",
        clients: [:alice],
        steps: [
          { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
          { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "#pollA" } },
          { client: :alice, command: "INVITE PollBot #pollA", expect: nil },
          { client: :alice, command: "", expect: /:PollBot!.+@.+ JOIN #pollA/, timeout: 3 },
          { client: :alice, command: "PRIVMSG #pollA :!poll start Choose one | One | Two", expect: [
            /PRIVMSG #pollA :New poll: Choose one/,
            /PRIVMSG #pollA :1\) One/,
            /PRIVMSG #pollA :2\) Two/
          ], timeout: 3 },
          # invalid vote id
          { client: :alice, command: "PRIVMSG PollBot :vote #pollA 5", expect: /NOTICE alice :Please send a valid option number\./, timeout: 2 },
          # close poll
          { client: :alice, command: "PRIVMSG #pollA :!poll close", expect: /PRIVMSG #pollA :Poll closed\. Results:/, timeout: 3 },
          # vote when no open poll -> no open poll notice
          { client: :alice, command: "PRIVMSG PollBot :vote #pollA 1", expect: /NOTICE alice :No open poll in that channel\./, timeout: 2 }
        ]
      },
      {
        name: "PollBot Multiple Voters and Tally",
        clients: [:alice, :bob],
        steps: [
          { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
          { procedure: :register_client, client_map: { client: :bob }, variables: { nickname: "bob" } },
          { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "#pollB" } },
          { procedure: :join_channel, client_map: { client: :bob }, variables: { channel: "#pollB" } },
          { client: :alice, command: "INVITE PollBot #pollB", expect: nil },
          { client: :alice, command: "", expect: /:PollBot!.+@.+ JOIN #pollB/, timeout: 3 },
          { client: :alice, command: "PRIVMSG #pollB :!poll start Pick | A | B | C", expect: /PRIVMSG #pollB :New poll: Pick/, timeout: 3 },
          # votes
          { client: :alice, command: "PRIVMSG PollBot :vote #pollB 2", expect: /NOTICE alice :Your vote has been recorded\./, timeout: 2 },
          { client: :bob,   command: "PRIVMSG PollBot :vote #pollB 3", expect: /NOTICE bob :Your vote has been recorded\./, timeout: 2 },
          # close and check tallies
          { client: :alice, command: "PRIVMSG #pollB :!poll close", expect: [
            /PRIVMSG #pollB :1\) A - 0 vote\(s\)/,
            /PRIVMSG #pollB :2\) B - 1 vote\(s\)/,
            /PRIVMSG #pollB :3\) C - 1 vote\(s\)/
          ], timeout: 3 }
        ]
      },
      {
        name: "PollBot Vote Update Overwrites Previous",
        clients: [:alice],
        steps: [
          { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
          { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "#pollC" } },
          { client: :alice, command: "INVITE PollBot #pollC", expect: nil },
          { client: :alice, command: "", expect: /:PollBot!.+@.+ JOIN #pollC/, timeout: 3 },
          { client: :alice, command: "PRIVMSG #pollC :!poll start Choose | Left | Right", expect: /PRIVMSG #pollC :New poll: Choose/, timeout: 3 },
          { client: :alice, command: "PRIVMSG PollBot :vote #pollC 1", expect: /NOTICE alice :Your vote has been recorded\./, timeout: 2 },
          { client: :alice, command: "PRIVMSG PollBot :vote #pollC 2", expect: /NOTICE alice :Your vote has been recorded\./, timeout: 2 },
          { client: :alice, command: "PRIVMSG #pollC :!poll close", expect: [
            /PRIVMSG #pollC :1\) Left - 0 vote\(s\)/,
            /PRIVMSG #pollC :2\) Right - 1 vote\(s\)/
          ], timeout: 3 }
        ]
      },
      {
        name: "PollBot Prevent Second Poll and Close by Non-Starter",
        clients: [:alice, :bob],
        steps: [
          { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
          { procedure: :register_client, client_map: { client: :bob }, variables: { nickname: "bob" } },
          { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "#pollD" } },
          { procedure: :join_channel, client_map: { client: :bob }, variables: { channel: "#pollD" } },
          { client: :alice, command: "INVITE PollBot #pollD", expect: nil },
          { client: :alice, command: "", expect: /:PollBot!.+@.+ JOIN #pollD/, timeout: 3 },
          { client: :alice, command: "PRIVMSG #pollD :!poll start A | Yes | No", expect: /PRIVMSG #pollD :New poll: A/, timeout: 3 },
          # bob tries to start another poll
          { client: :bob, command: "PRIVMSG #pollD :!poll start Another | 1 | 2", expect: /NOTICE #pollD :There\'s already an open poll\./, timeout: 2 },
          # bob closes the poll
          { client: :bob, command: "PRIVMSG #pollD :!poll close", expect: [
            /PRIVMSG #pollD :Poll closed\. Results:/,
            /PRIVMSG #pollD :1\) Yes - 0 vote\(s\)/,
            /PRIVMSG #pollD :2\) No - 0 vote\(s\)/
          ], timeout: 3 }
        ]
      },
      {
        name: "PollBot Ignores Channel Vote Text",
        clients: [:alice],
        steps: [
          { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
          { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "#pollE" } },
          { client: :alice, command: "INVITE PollBot #pollE", expect: nil },
          { client: :alice, command: "", expect: /:PollBot!.+@.+ JOIN #pollE/, timeout: 3 },
          { client: :alice, command: "PRIVMSG #pollE :!poll start X | Y | Z", expect: /PRIVMSG #pollE :New poll: X/, timeout: 3 },
          # In-channel 'vote' should be ignored by bot logic
          { client: :alice, command: "PRIVMSG #pollE :vote #pollE 2", expect: nil },
          # Proper private vote
          { client: :alice, command: "PRIVMSG PollBot :vote #pollE 2", expect: /NOTICE alice :Your vote has been recorded\./, timeout: 2 },
          # Results should only reflect the private vote
          { client: :alice, command: "PRIVMSG #pollE :!poll close", expect: [
            /PRIVMSG #pollE :1\) Y - 0 vote\(s\)/,
            /PRIVMSG #pollE :2\) Z - 1 vote\(s\)/
          ], timeout: 3 }
        ]
      },
      {
        name: "PollBot Malformed Vote Command",
        clients: [:alice],
        steps: [
          { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
          { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "#pollF" } },
          { client: :alice, command: "INVITE PollBot #pollF", expect: nil },
          { client: :alice, command: "", expect: /:PollBot!.+@.+ JOIN #pollF/, timeout: 3 },
          { client: :alice, command: "PRIVMSG #pollF :!poll start Q | Yes | No", expect: /PRIVMSG #pollF :New poll: Q/, timeout: 3 },
          # missing number
          { client: :alice, command: "PRIVMSG PollBot :vote #pollF", expect: /NOTICE alice :Usage: vote #channel <number>/, timeout: 2 },
          # non-numeric
          { client: :alice, command: "PRIVMSG PollBot :vote #pollF two", expect: /NOTICE alice :Please send a valid option number\./, timeout: 2 },
          # close to end
          { client: :alice, command: "PRIVMSG #pollF :!poll close", expect: /PRIVMSG #pollF :Poll closed\. Results:/, timeout: 3 }
        ]
      },
      {
        name: "PollBot Start Requires At Least Two Options",
        clients: [:alice],
        steps: [
          { procedure: :register_client, client_map: { client: :alice }, variables: { nickname: "alice" } },
          { procedure: :join_channel, client_map: { client: :alice }, variables: { channel: "#pollG" } },
          { client: :alice, command: "INVITE PollBot #pollG", expect: nil },
          { client: :alice, command: "", expect: /:PollBot!.+@.+ JOIN #pollG/, timeout: 3 },
          # one option only -> should respond with usage
          { client: :alice, command: "PRIVMSG #pollG :!poll start Need more | OnlyOne", expect: /NOTICE #pollG :Usage: !poll start <question> \| <opt1> \| <opt2>/, timeout: 3 }
        ]
      }
    ]
    tester.run_test_suite(test_cases + pollbot_tests)
  end
ensure
  tester.cleanup
end
