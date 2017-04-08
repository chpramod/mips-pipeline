# .bashrc

# User specific aliases and functions

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

export CPU=X86
export ARCH=x86
export SIMDIR=~/Ksim
export FLASH_TOOLS=$SIMDIR/Tools
export TWINEDIR=$SIMDIR/Tools
export FLASH_PERL_EXEC=/usr/bin/perl
export PATH=$PATH:$SIMDIR/Tools/bin/$CPU:$SIMDIR/Tools/bin