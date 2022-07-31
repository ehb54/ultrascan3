#!/usr/bin/perl

$notes = "usage: $0 csv

fixes csv, outputs to fixed.csv

checks counts in lines and pads with zeros
"
    ;

$if = shift || die $notes;

die "$if does not exist\n" if !-e $if;
die "$if is not readable\n" if !-r $if;

open IN, $if || die "$if $!\n";
@l = <IN>;
close IN;
grep chomp, @l;

## pass 1 - validate lines & find max r count

$h = $l[0];
@hf = split ',', $h;
$hrcount = @hf - 6;
$matchcount = @hf - 2;

print "hrcount $hrcount\n";

push @out, "$h\n";

for ( my $i = 1; $i < @l; ++$i ) {
    my $l = $l[$i];
    if ( $l =~ /^$/ ) {
        push @out, "$l\n";
        next;
    }
        
    my @f = split ',', $l;

    if ( @f > $matchcount ) {
        die "line " . ( $i + 1 ) . " has a greater count " . (scalar @f) . " than the header $matchcount\n";
    }
    if ( @f < $matchcount ) {
        my $padc = $matchcount - @f;
        warn "line " . ( $i + 1 ) . " will be padded with $padc zeros\n";
        for ( my $j = 0; $j < $padc; ++$j ) {
            push @f, "0";
        }
    }
    my $nl = join ',', @f;
    push @out, "$nl\n";
}    


$of = "fixed.csv";
open OUT, ">$of";
print OUT join '', @out;
close OUT;
print "$of\n";

