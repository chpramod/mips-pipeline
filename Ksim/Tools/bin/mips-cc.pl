#!/usr/bin/perl -w

$objdump="mips-sgi-irix5-objdump";
$as="mips-sgi-irix5-as";
$ld="mips-sgi-irix5-ld -no-warn-mismatch";
$mipspath="/usr/local/lib/gcc-lib/mips-sgi-irix5";
$image="$ENV{'SIMDIR'}/Tools/bin/genmipsimage.pl";
$cc="mips-sgi-irix5-gcc";


my $suifinc="$mipspath/suif-include";
my $suifbin="/usr/local/bin";
my $f77path="$mipspath/suif-lib";

use Getopt::Long;

my $opt_compile=0;
my $opt_output="";
my $verbose = 0;
my $keepfiles = 0;
my $optimize = -1;
my $asm_only = 0;
my @defines;
my @includes;
my @libdirs;
my @libraries;

my $debug = 0;
my $fortran = 0;
my $mkdeps = 0;
my @archlevel;
my $opt_fortran_link=0;

Getopt::Long::Configure("no_ignore_case");
Getopt::Long::Configure("bundling");

GetOptions(
	   "c" => \$opt_compile,
	   "O:i" => \$optimize,
	   "o=s" => \$opt_output,
	   "v" => \$verbose,
	   "S" => \$asm_only,
	   "k" => \$keepfiles,
	   "f" => \$opt_fortran_link,
	   "D=s@" => \@defines,
	   "I=s@" => \@includes,
	   "L=s@" => \@libdirs,
	   "l=s@" => \@libraries,
	   "g" => \$debug,
	   "M" => \$mkdeps,
	   "m=s@" => \@archlevel
	  );

if ($optimize >= 0) {
  $cc = $cc . " -O";
  if ($optimize > 1) {
    $cc = $cc . ($optimize);
  }
}

if ($debug) {
  $cc = $cc . " -g";
}

if ($mkdeps) {
  $cc = $cc . " -M";
}

$cc = $cc . " -m" . join (" -m", @archlevel) if $#archlevel >= 0;
$as = $as . " -m" . join (" -m", @archlevel) if $#archlevel >= 0;

# standard includes
@includes = (@includes, "$mipspath/2.7.2.3/include", "$mipspath/include", "$mipspath/include/CC");

# standard libs
@libdirs = (@libdirs, "$mipspath/lib/nonshared");


# includes
$cc = $cc . " -I" . join(" -I", @includes) if $#includes >= 0;
# defines
$cc = $cc . " -D" . join(" -D", @defines) if $#defines >= 0;

die "Usage: $0 [-c] [-k] [-O] [-v] [-o output] files...\n" unless $#ARGV >= 0;

die "$0: -c/-S and -o can only support one file argument" if (($opt_compile == 1 || $asm_only == 1) && $#ARGV != 0 && $opt_output ne "");

my $i;

@objlist=();

for ($i=0; $i <= $#ARGV; $i++) {
  if (($ARGV[$i] =~ /^(.+)\.[csf]$/) || ($ARGV[$i] =~ /^(.+)\.cc$/)) {
    if ($mkdeps == 0) {
      if ($opt_output ne "") {
	if ($opt_compile == 0 && $asm_only == 0) {
	  gen_obj_file ($ARGV[$i],$1 . ".o");
	  push (@objlist, $1 . ".o");
	}
	else {
	  gen_obj_file ($ARGV[$i],$opt_output);
	}
      }
      else {
	gen_obj_file ($ARGV[$i],$1 . ".o");
	push (@objlist, $1 . ".o");
      }
    }
    else {
      my_system ($cc . " $ARGV[$i]");
    }
  }
  else {
    push (@objlist, $ARGV[$i]);
  }
}

if ($opt_compile == 0 && $asm_only == 0 && $mkdeps == 0) {
  my $outname;
  if ($opt_output eq "") {
    $outname = "a.out";
  }
  else {
    $outname = $opt_output;
  }

  if ($opt_fortran_link) {
    @libraries = ("F77", "I77", @libraries);
    @libdirs = ($f77path, @libdirs);
  }

  link_obj_files ($outname . ".ld", @objlist);
  gen_image_file ($outname . ".ld", $outname . ".image");
  if ($keepfiles == 0) {
    if ($verbose) {
      printf ("unlink $outname.ld\n");
    }
    unlink($outname . ".ld");
  }
}

exit 0;

sub gen_obj_file {

  my ($file,$output) = @_;
  my $tmpfile;

  if ($verbose) {
    printf ("Generating .o for $file\n");
  }
  if ($file =~ /\.s$/) {
    my_system ("cp $file tmp.$$.s");
  }
  elsif (($file =~ /\.c$/) || ($file =~ /\.cc$/)) {
    my_system ($cc . " -o tmp.$$.s -S $file");
  }
  else {
    if ($fortran == 0) {
      @libraries = ("F77", "I77", @libraries);
      @libdirs = ($f77path, @libdirs);
    }
    $fortran = 1;
    print "Calling f2c on $file\n";
#    my_system ("$suifbin/sf2c -a -g -w -quiet -lab $file -o $file.c");
    my_system ("$suifbin/f2c -a -g -w -Nn802 $file");
    print "Compiling $file.c\n";
    my_system ($cc . " -I$suifinc " . " -o tmp.$$.s -S $file.c");
  }

  die "Could not open assembly output!" unless open(FUBAR,"<tmp.$$.s");

  $tmpfile = "tmp2.$$.s";

  die "Could not open tmp file!" unless open(FUBAR2,">$tmpfile");

  if ($verbose) {
    print ("Strip .pic from tmp.$$.s, generate $tmpfile\n");
  }

  while (<FUBAR>) {
    print FUBAR2 unless /.option pic/;
  }
  close (FUBAR);
  close (FUBAR2);

  if ($verbose) {
     print "unlink tmp.$$.s\n";
  }
  unlink "tmp.$$.s";

  if ($asm_only) {
    if ($output =~ /(.*)\.o$/) {
      $output = $1 . ".s";
    }
    my_system ("cp $tmpfile $output");
  }
  else {
    my_system ($as . " -non_shared $tmpfile -o $output");
  }

  if ($verbose) {
    print "unlink $tmpfile\n";
  }
  unlink $tmpfile;
}


sub link_obj_files {
  my $outfile = $_[0];

  shift;

  my @file = @_;
  my $linkoptdir;
  my $linkoptlib;

  if ($verbose) {
    printf ("Linking $outfile <- @file\n");
  }
  
  $linkoptdir = "";
  $linkoptlib = "";

  $linkoptdir = $linkoptdir . " -L" . join(" -L", @libdirs) if $#libdirs >= 0;
  $linkoptlib = $linkoptlib . " -l" . join(" -l", @libraries) if $#libraries >= 0;
  $linkoptlib = $linkoptlib . " -lm -lgcc -lc -lgcc";

  my_system ("$ld -N -non_shared -o $outfile  $mipspath/lib/nonshared/crt1.o $linkoptdir @file $linkoptlib $mipspath/lib/nonshared/crtn.o 2> .link.out");
  

  if ( ! -f $outfile ) {
    print ("Linking failed. Please examine [.link.out]\n");
    exit (1);
  }
}


sub gen_image_file {

  my ($infile, $outfile) = @_;
  my $entry_point;
  my $tmpfile;


  if ($verbose) {
    printf ("Generating image for $infile -> $outfile\n");
  }

  my_system ("$image $infile > $outfile");

  #
  # Determine entry point by looking at the executable
  #

  open (FUBAR, "$objdump -h $infile|");

  $entry_point=0;

  while (<FUBAR>) {
    if (/.text/) {
      /.text\s+[0-9a-f]+\s+([0-9a-f]+)\s+/;
      $entry_point=hex($1);
      last;
    }
  }
  close(FUBAR);


  $tmpfile = "tmp.image.$$";
  if ($verbose) {
    printf ("Entry point for main program: 0x%08x\n", $entry_point);
    printf ("Generate $tmpfile from boot.s with entry point...\n");
  }
  if ( ! -f "boot.s") {
    die "boot.s missing" unless 
      open(FUBAR, "<" . $ENV{'SIMDIR'} . "/Tools/bin/boot.s");
  }
  else {
    die "boot.s missing" unless open(FUBAR, "<boot.s");
  }
  open(FUBAR2,">$tmpfile");

  while (<FUBAR>) {
    s/ENTRY_POINT/$entry_point/g;
    print FUBAR2;
  }
  close(FUBAR);
  close(FUBAR2);

  if ($verbose) {
    printf ("Assembling boot code...\n");
  }

  my_system ($as . " $tmpfile -o boot.preld.o");
  my_system ($ld . " -e __boot_code boot.preld.o -Ttext 0x1fc00000 -T " .
		$ENV{'SIMDIR'} . "/Tools/bin/boot.ldscript" .
		" -o boot.o");
  unlink ("boot.preld.o");

  if ($verbose) {
    print ("unlink $tmpfile\n");
  }
  unlink ($tmpfile);

  my_system ($image . " boot.o .text > boot.image");
  my_system ("cat boot.image >> $outfile");
}



sub my_system {
  my $cmd = $_[0];

#  if ($verbose) {
#    printf ("$cmd \n");
#  }
  system ($cmd);
}
