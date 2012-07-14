#!/usr/bin/perl

# script to convert string from http://mrdoob.com/projects/voxels/ into our c structure and back

use strict;
use Data::Dumper;

# colors as defined on web page
my @colors = (0xDF1F1F, 0xDFAF1F, 0x80DF1F, 0x1FDF50, 0x1FDFDF, 0x1F4FDF, 0x7F1FDF, 0xDF1FAF, 0xEFEFEF, 0x303030);

if(@ARGV < 2) {
    &usage();
    exit(-1);
}

# check arguments
if($ARGV[0] eq 'decode') {
    &decode($ARGV[1], $ARGV[2]);
}
elsif($ARGV[0] eq 'encode') {
    if(-e $ARGV[1]) {
	print 'TODO: implement' . "\n";
	#&encode($ARGV[1]);
    }
    else {
	print 'Encode Error' . "\n";
	print 'File ' . $ARGV[0] . ' does not exist.' . "\n";
	exit(-1);
    }
}
else {
    &usage();
    exit(-1);
}
exit(0);

# create c struct of voxels
# pass a string generated on webpage and produce a c struct
# similar to buildFromHash() function
sub decode {
    my $str = shift(@_);
    my $arrayName = shift(@_);

    print '/* ' . $str . ' */' . "\n";
    # print structure container
    print 'const pos_t ' . (($arrayName) ? $arrayName : 'vox') . '[] = {' . "\n";

    my @data = strToArray($str);
    my $i = 0;
    my $l = @data;

    my %current = ('x', 0,
		   'y', 0,
		   'z', 0,
		   'c', 0);

    while($i < $l) {
	# convert to binary string
	my $code = $data[$i++];
	if(($code >> 3) & 0x1) {
	    $current{'x'} += $data[$i++] - 32;
	}
	if(($code >> 2) & 0x1) {
	    $current{'y'} += $data[$i++] - 32;
	}
	if(($code >> 1) & 0x1) {
	    $current{'z'} += $data[$i++] - 32;
	}
	if($code & 0x1) {
	    $current{'c'} += $data[$i++] - 32;
	}
	if(($code >> 4) & 0x1) {
	    print "\t" . '{';
	    print $current{'x'} . ', ';
	    print $current{'y'} . ', ';
	    print $current{'z'} . ', ';
	    print '{';
	    print (($colors[$current{'c'}] >> 16) & 0xff);
	    print ', ';
	    print (($colors[$current{'c'}] >> 8) & 0xff);
	    print ', ';
	    print ($colors[$current{'c'}] & 0xff);
	    print ', ';
	    print '0';
	    printf("} /* #%06x */", $colors[$current{'c'}]);
	    print'},' . "\n";
	}
    }
    print '};' . "\n";
    return;
}

sub strToArray {
    my @ret;
    # convert values in $input to values in $str
    my @str = split(//, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
    my @input = split(//, $_[0]);
    foreach (@input) {
	# find position of $_ in @str
	push (@ret, &arrayPos($_, @str));
    }
    return @ret;
}

sub arrayPos {
    my $element = shift(@_);
    my @array = @_;
    for(my $i=0;$i<@array;$i++) {
	if($array[$i] eq $element) {
	    return $i;
	}
    }
    return -1;
}

# create mrdoob string of voxels
sub encode {

}

# print usage details
sub usage {
    print 'Usage: ' . "\n";
    print 'perl ' . $0 . ' [encode|decode] [inputFile|string arrayName]' . "\n";
    print '- encode takes c struct file and outputs string to stdout' ."\n";
    print '- decode takes string and outputs c struct to stdout' . "\n";
}
