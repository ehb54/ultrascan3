#!/usr/bin/perl

$notes = "usage: $0 list-of-base-dirs
makes haddock-scores.txt in current directory as union of all file.nam_haddock-score's
";

$hr = $ENV{ 'hr' } || die "$0: environment variable hr must be defined\n";

die "$0: directory $hr does not exist\n" if !-d $hr;

@stages = ( "1", "1/water" );

$bd = `pwd`;
chomp $bd;

die $notes if !@ARGV;

$ff = "file.nam_haddock-score";
while ( $fb = shift )
{
    $b = "$hr$fb/run1/structures/it";

    for( $i = 0; $i < @stages; ++$i )
    {
        $d = "$b$stages[$i]";
        die "$d does not exist or is not a directory\n" if !-d $d;
#        chdir $d || die "$0: can't change to directory $d\n";

        $f = "$d/$ff";
        die "$f does not exist\n" if !-e $f;

        open IN, "$f" || die;
        @l = <IN>;
        close IN;
        shift @l;
        $out .= join '', @l;
    }
}

$f = "haddock-scores.txt";
print ">$f\n";
open OUT, ">$f" || die;
print OUT $out;
close OUT;
