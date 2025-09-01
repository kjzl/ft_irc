# how the tester works

Usage:

Run with debug output:
```bash
DEBUG=1 ruby irc_tester_fixed.rb
```

Run normally:
```bash
ruby irc_tester_fixed.rb
```

The idea is to recompile with DTESTMEM=n
with n being the iteration count of the loop that runs the program so and so many times, letting a single specific alloc fail.

## RUBY
The /.../ syntax in Ruby is a regular expression literal 

### ruby default value

default to a value by OR (||)
      client_id = client_id_map[step[:client]] || step[:client] # use clients from map or else the ones from procedure

this means it will use the value from the map if it exists, otherwise it will use the value from the procedure

default to a value by assignment (=)
  def client_received?(client_id, pattern, timeout = 3)

when defining a method, you can assign a default value to a parameter by using the assignment operator (=). If the caller does not provide a value for that parameter, the default value will be used.

### rubys implicit return
In Ruby, the value of the last expression evaluated in a method is what gets returned if you don't use an explicit return statement.

```C++
def execute_step(step, client_id)
  command = step[:command]
  expected = step[:expect]
  timeout = step[:timeout] || 3

  send_command(client_id, command)

  return true unless expected

  if expected.is_a?(Array)
    expected.all? { |pattern| client_received?(client_id, pattern, timeout) }
  else
    client_received?(client_id, expected, timeout)
  end
end
```
