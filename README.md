This design assignment is meant to be an introduction to the CS422 MIPS
integer processor. The processor simulator that is used
implements the MIPS-I integer instruction subset and only four floating-point
instructions to make the system call code work (lwc1, swc1, mtc1, and mfc1). 
The simulator can simulate any integer program, but not the programs that 
involve floating-point operations. 

INSTALLATION
-------------

***Setting up the environment*** 
Include the following lines in your .bashrc. If
you use some other shell environment, change these accordingly.

export CPU=X86
export ARCH=x86
export SIMDIR=~/Ksim
export FLASH_TOOLS=$SIMDIR/Tools
export TWINEDIR=$SIMDIR/Tools
export FLASH_PERL_EXEC=/usr/bin/perl
export PATH=$PATH:$SIMDIR/Tools/bin/$CPU:$SIMDIR/Tools/bin

Make sure to execute

source ~/.bashrc.

***Compiling the simulator***
Move to ~/Ksim/cpus/sync/mips-fast/ and type `gmake clobber clean'
followed by `gmake'.

> cd ~/Ksim/cpus/sync/mips-fast/
> gmake clobber clean
> gmake

If everything goes fine, you should see an executable named mipc in the current directory.
This is the simulator binary.

***Compiling the test programs***
Move to ~/Ksim/Bench/lib and type gmake. 

> cd ~/Ksim/Bench/lib
> gmake

Move to the individual test program directories in ~/Ksim/Bench/testcode and type gmake to compile each.
If everything goes fine, in each directory you should see a .image and a .ld file. The .image
file contains the binary image of the application and is the input to the simulator, while the .ld file 
can be used to generate a readable disassembly of the executable:

mips-sgi-irix5-objdump -D foo.ld > foo.dis

The leftmost column of each non-empty line in foo.dis is the PC of the
instruction listed in that line. This is very useful for debugging.

***Running a program on the simulator***
The simulator takes as input the name
of the .image file. It should be run from the ~/Ksim/cpus/sync/mips-fast/
directory. For example, to run the program in the ~/Ksim/Bench/testcode/asm-sim/
directory, you should say the following.

> cd ~/Ksim/cpus/sync/mips-fast/
> ./mipc ~/Ksim/Bench/testcode/asm-sim/example

Note that the .image extension is dropped from
~/Ksim/Bench/testcode/asm-sim/example.image because that is the default
extension. On successful completion of the simulation, the expected output
should be printed and a statistics file named mipc.log is generated in
Ksim/cpus/sync/mips-fast/ directory. 

*** SPECIFICATIONS (PROVIDED BY THE INSTRUCTOR) ***

1. The compiler always fills in the branch delay slot correctly. So, there
should not be any CPI loss due to branches provided you design the pipeline
such that the branch instructions update the PC within the positive half of
a cycle and the instruction fetch takes place during the negative half of a
cycle. Note that the current unpipelined implementation does exactly
the opposite i.e., executes all instructions during the negative half of the 
cycle (see executor.cc) and fetches instructions during the positive half of
the cycle (see mips.cc).

2. Assume that the compiler does not fill the load-delay slot with independent
instructions. You should implement a proper interlock logic in the decode stage
of the pipe.

3. System calls should execute in the WB stage, but as soon as you detect a
system call in the decoder, you should stall the fetcher and also ``nullify''
the previous instruction that has already been fetched. In other words, when
the system call finally executes, the pipe should be filled with NOPs (a zero
instruction is a NOP; you can verify this from the decoder).

4. Implement phased register read and register write so that the register
write happens in the first half of a cycle and register read happens in the
second half of the cycle. You will notice that the ID/RF stage is already
implemented to decode an instruction and read register operands during the
second half of the cycle (see decode.cc). This will obviate the need for a
bypass from the WB stage.

5. Follow these steps to get to the final pipelined design:

A. Save a working copy of the unpipelined design for your reference. This is
very helpful for debugging the pipelined design because if you want, you
can compare the sequences of executed instructions in the two designs. They
must match always.

B. Plan the contents of the pipeline registers i.e., what you
should carry forward through the pipeline.

C. You should not require most of the handshake signals of the unpipelined
design. So the first step is to get rid of them. See how the design fails to
work as soon as you do this. Then implement the pipeline register contents in
each stage. You should sample the register contents during the positive half and
work on the contents during the negative half. If you are careful, you will 
notice that some of this logic is already implemented. Remember to correctly
co-ordinate the EX and IF stages for branches so that there is exactly one
delay slot for branches.

D. At this point the design would not work because you don't have the bypass
paths. Implement a complete interlock logic in the decoder whenever you detect
a possible data hazard. This design should work and have a CPI lower than 5.0,
but much higher than 1.0 due to interlock stalls.

E. Incorporate each of the bypass paths one by one and remove the corresponding
stall logic from the decoder. You should observe that the CPI is gradually
going down. Finally, your design should show a CPI of 1.0 on programs that
have no load-delay-induced stalls (this interlock logic will remain in the
decoder even in the final design).

F. There is a MIPC_DEBUG flag in mips.h. If you uncomment that and recompile
i.e., 

gmake clobber clean; gmake

and run some program, a pipe trace will be generated in the file mipc.debug.
You can open this file in any text editor and see what happened in different
pipe stages every cycle. Each line in this file mentions an event in a certain
pipe stage. The line starts with the corresponding cycle within the angle 
brackets. This trace is very handy for debugging.
