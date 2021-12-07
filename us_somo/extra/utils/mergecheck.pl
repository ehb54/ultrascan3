#!/usr/bin/perl

$notes = "usage: $0 sourcedir destdir
compare 2 source trees
list:
files in source not present in dest
diffs
skips files ending in ~ or #

best way to run this (four runs)
1. do the mkdirs cmds
2. do the new files cmds
3. check the remaining diffs and cp if ok
4. check fhe final result (should be empty)

Afterwards:
cd destdir && svn diff
if ok: svn commit
";

$sd = shift || die $notes;
$dd = shift || die $notes;

die "$sd is not a directory\n" if !-d $sd;
die "$dd is not a directory\n" if !-d $dd;

@sf = `cd $sd && find * -type f | grep -v '\~' | grep -v '\#\$'`;
@df = `cd $dd && find * -type f | grep -v '\~' | grep -v '\#\$'`;

grep chomp, @sf;
grep chomp, @df;

for my $f ( @sf ) {
    $sfm{ $f }++;
}

for my $f ( @df ) {
    $dfm{ $f }++;
}

for my $f ( @sf ) {
    push @snotd, $f if !$dfm{ $f };
}

for my $f ( @df ) {
    push @dnots, $f if !$sfm{ $f };
}

$lbreak = '-'x80 . "\n";
$dlbreak = '='x100 . "\n";

for my $f ( @sf ) {
    if ( $dfm{ $f } ) {
        my $cmd = "diff $sd/$f $dd/$f";
        print "cmd --> '$cmd'\n" if $debug;
        my $diffs = `diff $sd/$f $dd/$f`;
        if ( length( $diffs ) ) {
            push @cpdfiles, $f;
            $difflog .= "$f
$diffs
$lbreak;
"                ;
        }
    }
}

if ( @snotd ) {
    print "Files present in $sd but not in $dd:\n";
    print join "\n", @snotd;
    print "\n";
    push @cpnfiles, @snotd;
    print $dlbreak;
    $todo++;
}

if ( $difflog ) {
    $todo++;
}

if ( $todo ) {
    for my $f ( @cpnfiles ) {
        my $d = $f;
        $d =~ s/\/[^\/]*$//;
        if ( !-d "$dd/$d" ) {
            $mkdirs{ "$dd/$d" }++;
        }
    }

    if ( keys %mkdirs ) {
        print "some mkdirs might fail due to depth issues:\n";

        for my $d ( keys %mkdirs ) {
            print "mkdir -p $d\n";
        }
        print $dlbreak;
    }

    if ( @cpnfiles ) {
        print "New files to copy:\n";
        for my $f ( @cpnfiles ) {
            print "cp $sd/$f $dd/$f\n";
        }
        print $dlbreak;
    }
    if ( @cpnfiles ) {
        print "If you wish to svn add:\n";
        print "(cd $dd && svn add ";
        print join ' ', @cpnfiles;
        print ")\n";
        print $dlbreak;
    }

    if ( $difflog ) {
        print "diff commands (for manual diff checks):\n";
        for my $f ( @cpdfiles ) {
            print "diff $sd/$f $dd/$f\n";
        }
        print $dlbreak;
        print "Diffs:\n$difflog$dlbreak";
        print "WARNING: make sure diffs are MERGEABLE-OK (diffs above) before running this:\n";
        for my $f ( @cpdfiles ) {
            print "cp $sd/$f $dd/$f\n";
        }
        print $dlbreak;
    }
    print "N.B.:Work through the steps above one at a time\n";
} else {
    print "${dlbreak}NICE: There no differences comparing files in source to destination.\n";
    if ( @dnots ) {
        print $lbreak;
        print "However: the following files are present in the destination and not in the source:\n";
        print join "\n", @dnots;
        print "\n$dlbreak";
    }
}

    
