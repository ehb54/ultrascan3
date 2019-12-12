#!/usr/bin/perl

$notes =
    "usage: $0 count files {file2 {...}}
creates rank lists by haddock score, saxs-chi score and combined score
leave count zero to keep all of them
";

$count = shift || die $notes;
die $notes if ! scalar @ARGV;

while ( $f = shift )
{
    undef %v;
    die "$0: $f does not exist\n" if !-e $f;
    open IN, $f || die "$0: error opening $f $!\n";

    @l = <IN>;

    close IN;

    ( $h  ) = $l[ 41 ] =~ /^REMARK haddock (\S+)$/;
    ( $s  ) = $l[ 42 ] =~ /^REMARK saxs-chi (\S+)$/;
    ( $hs ) = $l[ 43 ] =~ /^REMARK haddock\+saxs (\S+)$/;

    chomp $h;
    chomp $s;
    chomp $hs;

    push @h , "$h : $f";
    push @s , "$s : $f";
    push @hs, "$hs : $f";
}

@h  = sort { $a <=> $b } @h;
@s  = sort { $a <=> $b } @s;
@hs = sort { $a <=> $b } @hs;

print "\n" . '-'x80 . "\n";

print "h:\n";
print join "\n", @h;

print "\n" . '-'x80 . "\n";

print "s:\n";
print join "\n", @s;

print "\n" . '-'x80 . "\n";
print "hs:\n";
print join "\n", @hs;
print "\n";

print "\n" . '-'x80 . "\n";

grep s/^.*: //, @h;
grep s/^.*: //, @s;
grep s/^.*: //, @hs;


@h  = @h [ 0..($count - 1) ] if $count;
@s  = @s [ 0..($count - 1) ] if $count;
@hs = @hs[ 0..($count - 1) ] if $count;

for ( $i = 0; $i < @h; ++$i )
{
    $m{ $h[ $i ] }++;
}

for ( $i = 0; $i < @s; ++$i )
{
    $m{ $s[ $i ] }++;
}

for ( $i = 0; $i < @hs; ++$i )
{
    $m{ $hs[ $i ] }++;
}


foreach $k ( keys %m )
{
    push @manifest, $k;
}

@manifest = sort { $a cmp $b } @manifest;

$f = "haddock_score.list";
print ">$f\n";
open OUT, ">$f" || die "$0: error creating $f $!\n";
print OUT join "\n", @h;
print OUT "\n";
close OUT;

$f = "saxs_score.list";
print ">$f\n";
open OUT, ">$f" || die "$0: error creating $f $!\n";
print OUT join "\n", @s;
print OUT "\n";
close OUT;

$f = "haddock_saxs_score.list";
print ">$f\n";
open OUT, ">$f" || die "$0: error creating $f $!\n";
print OUT join "\n", @hs;
print OUT "\n";
close OUT;

$f = "manifest.list";
print ">$f\n";
open OUT, ">$f" || die "$0: error creating $f $!\n";
print OUT join "\n", @manifest;
print OUT "\n";
close OUT;


    
