# how the tester works

The idea is to recompile with DTESTMEM=n
with n being the iteration count of the loop that runs the program so and so many times, letting a single specific alloc fail.
