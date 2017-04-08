This design assignment is meant to be an introduction to the CS422 MIPS
integer processor. The processor simulator that you will use
implements the MIPS-I integer instruction subset and only four floating-point
instructions to make the system call code work (lwc1, swc1, mtc1, and mfc1). 
The simulator can simulate any integer program, but not the programs that 
involve floating-point operations. 

The assignment has two parts. In the first part, you will install the simulator
and get familiarized with it. The current distribution has some unimplemented
instructions. The first part of the assignment will guide you through
implementation of these. This is expected to help you get accustomed with the
internals of the simulator. In the second part of the assignment, you will
pipeline the MIPS processor.

INSTALLATION
-------------
Please follow these steps to install the simulator.

1. Log on to mainakc4.cse.iitk.ac.in using the account information mailed to you
by the instructor.

2. Copy ~mainakch/cs422.zip into your home space. Unzip it using 

unzip cs422.zip.

This will put all simulator code under a new directory named Ksim. The Ksim/Bench
directory has the application codes that we will run on the simulated processor.
There are 14 application programs under Ksim/Bench/testcode. Please do not
modify any of these. If you need to modify anything for debugging purpose,
please make sure to save the original one because the performance of your
design will be evaluated on these 14 test programs. The Ksim/Tools
directory has several scripts required for compiling the application programs
and the simulator. You should not modify anything here. The Ksim/lib directory has
the simulator library code. You should not modify anything here. The Ksim/cpus
directory has the main processor simulator. The Ksim/cpus/common directory
has the system call interface code and various other utilities such as a cache
interface. You should not modify anything here. You will mostly confine yourself 
to the Ksim/cpus/sync/mips-fast directory. This directory currently implements 
an unpipelined five-cycle MIPS integer processor. All the processor states are 
declared in mips.h. The comments should be self-explanatory. The mips.cc file 
implements the instruction fetcher (see the MainLoop method). The exec_helper.cc file
implements the decoder functionality and the EX and MEM operations. The
decode.{h,cc}, executor.{h,cc}, memory.{h,cc}, and wb.{h,cc} implement the
decoder/register fetch, execution, memory, and writeback phases of instruction
processing (see the MainLoop method of each .cc file). The top-level file is
main.cc.

Each MainLoop method has a common characteristic: it wakes up on the positive
edge of the clock (accomplished with a macro AWAIT_P_PHI0), samples the inputs,
and does its work on the negative edge of the clock (accomplished with a macro 
AWAIT_P_PHI1). Since this is an unpipelined design, every stage may not have
something to do on every cycle. As a result, a two-phase handshake guarantees
that if a stage has nothing to do on a positive edge, it just sleeps for a
cycle~(accomplished with a macro PAUSE(x) for sleeping x cycles). The
handshake is accomplished with the following signals.

_insValid	(Output of FETCH and input to DECODE)
_decodeValid	(Output of DECODE and input to EX)
_execValid	(Output of EX and input to MEM)
_memValid 	(Output of MEM and input to WB)
_insDone	(Output of WB and input to FETCH)

Note that the implemented protocol of sampling all the signals on the
positive edge and doing all the work on the negative edge is only an artifact
due to lack of a realistic flip-flop implementation. In reality, the work in a
stage would start right after the signals are sampled on the positive edge.
Your first task is to understand how different stages pass information to the
adjacent stages and what each stage accomplishes.

3. Setting up the environment: Include the following lines in your .bashrc. If
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

4. Compiling the simulator: Move to ~/Ksim/cpus/sync/mips-fast/ and type `gmake clobber clean'
followed by `gmake'.

> cd ~/Ksim/cpus/sync/mips-fast/
> gmake clobber clean
> gmake

If everything goes fine, you should see an executable named mipc in the current directory.
This is the simulator binary.

5. Compiling the test programs: Move to ~/Ksim/Bench/lib and type gmake. 

> cd ~/Ksim/Bench/lib
> gmake

Move to the individual test program directories in ~/Ksim/Bench/testcode and type gmake to compile each.
If everything goes fine, in each directory you should see a .image and a .ld file. The .image
file contains the binary image of the application and is the input to the simulator, while the .ld file 
can be used to generate a readable disassembly of the executable:

mips-sgi-irix5-objdump -D foo.ld > foo.dis

The leftmost column of each non-empty line in foo.dis is the PC of the
instruction listed in that line. This is very useful for debugging.

6. Running a program on the simulator: The simulator takes as input the name
of the .image file. It should be run from the ~/Ksim/cpus/sync/mips-fast/
directory. For example, to run the program in the ~/Ksim/Bench/testcode/asm-sim/
directory, you should say the following.

> cd ~/Ksim/cpus/sync/mips-fast/
> ./mipc ~/Ksim/Bench/testcode/asm-sim/example

Note that the .image extension is dropped from
~/Ksim/Bench/testcode/asm-sim/example.image because that is the default
extension. On successful completion of the simulation, the expected output
should be printed and a statistics file named mipc.log is generated in
Ksim/cpus/sync/mips-fast/ directory. Take a look at this file, which tells
you the number of instructions executed, number of cycles needed to execute
them, and many other useful things. Currently, the simulator won't be able
to execute any of the 14 programs to completion because some of the execution
functions in exec_helper.cc are left unspecified for you to fill in.

Always compile and run the simulator on mainakc4.cse.iitk.ac.in. Use the account 
information mailed to you by the instructor.

PART I [20 points]
------------------
1. Run each of the 14 programs. Whenever a simulation stops, it should tell
you what you need to fill in to proceed further. There are eight unspecified
execution functions in exec_helper.cc. Fill them in as you stumble upon them
while executing the test programs. Hint: See how similar instructions are
implemented. Take help from the 32-bit MIPS specification in C posted on the
course home. [10 points]

2. Once you have all 14 programs running, create a new C program that
generates the lwl, lwr, swl, and swr instructions. Run it on the simulator
and see if it executes correctly. To create a new test program, you need to
first make a new directory in ~/Ksim/Bench/testcode/. Call it Subreg. Copy
Makefile from ~/Ksim/Bench/testcode/hello and replace all instances of hello
by subreg. Then you write subreg.c in ~/Ksim/Bench/testcode/Subreg/. This
file should contain your program. Typing gmake in this directory should
create subreg.image and subreg.ld. Disassemble subreg.ld to see if it
generates the lwl, lwr, swl, and swr instructions. [5 points]

3. Now you have 15 test programs, including subreg.c. Run all of them and
record the total number of instructions simulated, percentage of loads
simulated (including the syscall loads), percentage of stores simulated 
(including the syscall stores), and percentage of conditional branches for each.
You will have to compute these from mipc.log after each simulation. Prepare
a table to report these statistics. Particularly, note the differences
between ifib and rfib (iterative and recursive Fibonacci), and ifactorial and 
factorial (iterative and recursive factorial). [5 points]

TIPS: If you are stuck in this part, please take help from the instructor
as soon as possible because the second part of the assignment is more important and
will take time to complete.

PART II [80 points]
-------------------
In this part of the assignment, you will pipeline the CS422 MIPS integer processor.
Your design will be evaluated based on the following criteria.

1. Correctness: Your design should be able to execute all the 15 programs correctly
giving outputs (not statistics) exactly same as the unpipelined design.
2. Performance: Speedup achieved compared to the unpipelined design for each of the 15 programs.
3. Cleanliness of the design: Reflects your thought-process and the level of
your understanding of the concepts.

In the following, I will discuss some tips to help you complete the assignment.

TIPS
----

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

WHAT TO SUBMIT
--------------

Prepare a table with the statistics from exercise 3 in PART I.

Prepare a second table that shows, for each of the 15 programs, how the CPI
progressively dropped as you made progress through PART II. The first column should report
the CPI of a design with complete interlock logic, but no bypass paths.
This design should also have two delay slots for branches, one of which should
be implemented as a stall cycle in the interlock. In the next column, show the
CPI as you remove the branch interlock cycle. In the subsequent columns,
show the CPI as you implement one data bypass path at a time. At the end, 
incorporate a column that reports the percentage of stall cycles arising from 
the load-delay interlock in the final design with all bypass paths enabled. 
For the programs that do not have any load-delay interlock stalls, if you do 
not see a CPI of 1.0, try to explain what might have happened. Prepare a brief 
writeup along with this table to summarize all that you have done to get to 
the final design and anything interesting that you would like me to know and 
appreciate.

Send by email to cs422spring2017@gmail.com the following with subject line [CS422 HW3].

A. The write-up including the aforementioned two tables and any other explanations.
We will accept only PDF files.
B. subreg.c
C. A zip of your mips-fast directory. Make sure to execute gmake clobber clean
before you zip the directory. Not doing this would include all object and
executable files in your zip and unnecessarily inflate the size of your 
submission. If you do not execute gmake clobber clean, you will lose points.

> cd ~/Ksim/cpus/sync/mips-fast/
> gmake clobber clean
> cd ..
> zip -r HW3_MYGROUPNO.zip mips-fast/

Replace MYGROUPNO by your group number. Please note that we will accept only a .zip file and no other
types of compressed archive.
