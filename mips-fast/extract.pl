#!/usr/bin/perl

@register_numbers = qw(r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 r12 r13 r14 r15 r16 r17 r18 r19 r20 r21 r22 r23 r24 r25 r26 r27 r28 r29 r30 r31);

@register_names = qw ($0 $at $v0 $v1 $a0 $a1 $a2 $a3 $t0 $t1 $t2 $t3 $t4 $t5 $t6 $t7 $s0 $s1 $s2 $s3 $s4 $s5 $s6 $s7 $t8 $t9 $k0 $k1 $gp $sp $s8 $ra);


#
# Location of mips-cc.pl, and libraries
#
#
my $cc = $ENV{'HOME'} . "/Sim/cpus/sync/test/mips-cc.pl";
my $objdump = "mips-sgi-irix5-objdump";
my $convert = $ENV{'HOME'} . "/Sim/cpus/sync/simple/convert";

$start = 0;
$flag = 0;

if ($ARGV[0] =~ /\-num/) {
  $num = 1;
} else {
  $num = 0;
  die ("usage: extract.pl [-num] <file_name>\n") unless (@ARGV == 1);
}
$name = $ARGV[$num];

die ("usage: extract.pl [-num] <file_name>\n") unless (@ARGV < 3);
@name_parts = split (/\./, $name);
open (LIST, ">$name_parts[0].break");

@name_split = split (/\//, $name);
$directory = $name;
$directory =~ s/$name_split[@name_split-1]//;

$count = 0;
$value = 0;

#
# compile to .o
#
die ("File $name does not exist!\n") unless (-e $name);
`$cc -c $name`;

#
# generate linked binary
#
die ("File $name_parts[0].o does no exist!\n") unless (-e "$name_parts[0].o");
`$cc $name_parts[0].o -o $name_parts[0] -k`;

#
# Generate .list file for boot code
#
die ("File ${directory}boot.image does not exist!\n") unless (-e "${directory}boot.image");
`$convert ${directory}boot.image > $name_parts[0].list`;
print LIST "00000000:48\n";  # length of boot code


$start = 0;
$count = 0;
$value = 0;

open (OUTPUT, ">>$name_parts[0].list") || die ("File $name_parts[0].list can not be opened!");

#
# dump .init section
#
#die ("File $name_parts[0].ld does not exist!\n") unless (-e "$name_parts[0].ld");
#`$objdump -j .init -D $name_parts[0].ld > $name_parts[0].out`;
#open (INPUT, "$name_parts[0].out") || die "Can't open $name_parts[0].out!\n";
#load_data (1);
#close (INPUT);
#`rm -f $name_parts[0].out`;


#
# Dump .text section
#
`$objdump --disassemble-zeroes -j .text -D $name_parts[0].ld > $name_parts[0].out`;

$start = 0;
$count = 0;
$value = 0;
open (INPUT, "$name_parts[0].out");
load_data (0);
close (INPUT);
`rm -f $name_parts[0].out $name_parts[0].ld $name_parts[0].o`;
close (LIST);
close (OUTPUT);

if ($num == 1) {
  for ($i = 0; $i < @register_names; $i++) {
    #print "$register_names[$i] $register_numbers[$i]\n";
    `sed /\\$register_names[$i]/s//$register_numbers[$i]/g $name_parts[0].list > $name_parts[0].list2`;
    `rm $name_parts[0].list`;
    `mv $name_parts[0].list2 $name_parts[0].list`;
  }
}

sub load_data {
  my($hack) = @_;
  while (<INPUT>) {
#    if (/__[i]*start\>/) {
#      $start = 1;
#    }
    if (/main/) {
      $start = 1;
    }
    if (/\:/) {
      @array = split(/\:/);
      if (($array[0] =~ /\<.+\>/) && ($start == 1)) { 
	$flag = 1;
	@temp = split;
	$temp[1] =~ s/\://;
	$data = $temp[1];
	$addr = $temp[0];
	if ($count == 0) {
	  for ($i=0; $i<8; $i++) {
	    $value += get_value(substr($addr,$i,1)) * (pow(7-$i));
	  }
	  print LIST "$addr:";
	} else {
	  for ($i=0; $i<8; $i++) {
	    $value += get_value(substr($addr,$i,1)) * (pow(7-$i));
	  }	
	  print LIST "$count\n$addr:";
	}
	$count = 0;
      } elsif ($start == 1) {
	$count++;
	@temp = split;
	if ($flag == 1) {
	  print OUTPUT "$data:$temp[0]$temp[1]:$temp[2]:$temp[3] $temp[4]\n";
	  $flag = 0;
	} else {
	  print OUTPUT ":$temp[0]$temp[1]:$temp[2]:$temp[3] $temp[4]\n";
	}
      }
    }
  }
  if ($hack == 1) {
    $count++;
  }
  print LIST "$count\n";
}
	
sub get_value {
  my ($value) = @_;
  my $temp;

  if ($value =~ /a/) {
    return 10;
  } elsif ($value =~ /b/) {
    return 11;
  } elsif ($value =~ /c/) {
    return 12;
  } elsif ($value =~ /d/) {
    return 13;
  } elsif ($value =~ /e/) {
    return 14;
  } elsif ($value =~ /f/) {
    return 15;
  }
  return $value;
}

sub pow {
  my ($value) = @_;
  my ($i,$result);

  $result = 1;
  for ($i=0; $i<$value; $i++) {
    $result *= 16;
  }

  return $result;
}
