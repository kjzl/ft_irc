#!/usr/bin/env ruby
require 'socket'

HOST = "localhost"
PORT = 6667

# ---- Alice ----
def alice_spammer
  Thread.new do
    sock = TCPSocket.new(HOST, PORT)
    puts "[Alice] Connected"

    sock.puts "PASS password"
    sock.puts "NICK alice"
    sock.puts "USER Alice 0 * :Alice"
    sleep 1 # wait for registration

    stop = false

    # Reader thread: watches for ERR_NOSUCHNICK or disconnect
    reader = Thread.new do
      begin
        while (line = sock.gets)
          puts "[Alice <<] #{line.strip}"
          if line.include?(" 401 ") # ERR_NOSUCHNICK
            puts "[Alice] Saw NOSUCHNICK, stopping spam"
            stop = true
            break
          end
        end
      rescue IOError, Errno::ECONNRESET
        puts "[Alice] Reader disconnected"
      end
    end

    # Writer loop: spams until stop flag is set
    begin
      loop do
        break if stop
        msg = "A" * 400
        sock.puts "PRIVMSG bob :#{msg}"
        sleep 0.001
      end
    rescue Errno::EPIPE, Errno::ECONNRESET
      puts "[Alice] Writer disconnected"
    end

    reader.join
    sock.close unless sock.closed?
    puts "[Alice] Closed socket, exiting"
  end
end

# ---- Bob ----
def bob_slow
  Thread.new do
    sock = TCPSocket.new(HOST, PORT)
    puts "[Bob] Connected"

    # Limit the kernel receive buffer to 4 KB
    sock.setsockopt(Socket::SOL_SOCKET, Socket::SO_RCVBUF, 4096)

    sock.puts "PASS password"
    sock.puts "NICK bob"
    sock.puts "USER Bob 0 * :Bob"

    begin
      loop do
        # Wait until socket is readable or timeout
        ready_socks, = IO.select([sock], nil, nil, 1)
        next unless ready_socks

        if sock.eof? # peer closed the connection
          puts "[Bob] Disconnected by server (EOF detected)"
          break
        end

        data = sock.read_nonblock(4096, exception: false)
        next if data.nil? || data == :wait_readable

        puts "[Bob] got #{data.bytesize} bytes (processing slowly)"
        sleep 0.1
      end
    rescue Errno::ECONNRESET
      puts "[Bob] Disconnected by server (RST)"
    end

    sock.close unless sock.closed?
    puts "[Bob] Closed socket, exiting"
  end
end

# ---- Main ----
threads = []
threads << alice_spammer
threads << bob_slow

threads.each(&:join)
puts "[Main] Test finished"
