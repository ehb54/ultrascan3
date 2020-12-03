#!/usr/bin/perl

$qt = "/opt/qt-5.14.2";
$qwt = "/opt/qt-5.14.2-qwt-6.1.5";

$notes = "usage: $0 sysname ultrascandir
sysname is something like ubuntu20
ultrascandir is the place ultrascan was compiled
$qt must exist or change \$qt in this code
$qwt must exist or change \$qwt in this code
";

$sn = shift || die $notes;
$us = shift || die $notes;

die "$us does not exist\n" if !-e $us;
die "$us is not a directory\n" if !-d $us;

die "$qt does not exist\n" if !-e $qt;

$rfile = "$us/programs/us/us_revision.h";
die "$rfile does not exist\n" if !-e $rfile;
$cmd = "grep REVISION $rfile | awk -F\\\" '{ print \$2 }'";
# print "$cmd\n";
$rev = `$cmd`;
chomp $rev;
print "revision is $rev\n";

$dd = "us3-$sn-64-4.0.$rev";
print "$dd\n";

die "$dd exists, please remove it first
rm -fr $dd
" if -e $dd;

`mkdir $dd`;

$basedir = `pwd`;
chomp $basedir;

chdir $dd;

@rsync =
    (
     "$us/bin"        , "bin"
     ,"$us/etc"       , "etc"
     ,"$us/lib"       , "lib"
     ,"$us/somo/doc"  , "somo/doc"
     ,"$us/somo/demo" , "somo/demo"
     ,"$qt/lib"       , "lib"
     ,"$qwt/lib"      , "lib"
     ,"$qt/plugins"   , "plugins"
    );

@lddfind =
    (
     "bin"
    );

    
print "working in " . `pwd`;

sub docmd {
    my $cmd   = shift;
    my $debug = shift;
    print "$cmd\n";
    print `$cmd` if !$debug;
}

for ( $i = 0; $i < @rsync; $i += 2 ) {
    $s = $rsync[$i];
    $d = $rsync[$i+1];
    if ( $d =~ /\// ) {
        $md = $d;
        $md =~ s/\/[^\/]+$//;
        docmd( "mkdir $md" ) if !-d $md;
    }
    
    docmd( "rsync -av $s/* $d/" );
    
}

# clean up ~'s
docmd( "find . -name \"*~\" | xargs rm" );

# find system libraries needed
sub addsyslibs {
    $pwd = `pwd`;
    chomp $pwd;

    my @tocheck =
        (
         "bin",
         "lib"
        );

    my %s;
    my %d;
    my %c;

    my $added = 0;

    for my $cdir ( @tocheck ) {
        my @f = `cd $cdir && find . -type f`;
        grep chomp, @f;
        for my $f ( @f ) {
            $cmd = "(env LD_LIBRARY_PATH=$pwd/lib ldd $cdir/$f | grep -v $dd | grep '=>' | awk '{ print \$3 }' ) 2> /dev/null";
            print "$cmd\n";
            my @addlib = `$cmd`;
            grep chomp, @addlib;
            for my $addlib ( @addlib ) {
                $s{ $addlib }++;
            }
        }
        for my $s ( keys %s ) {
            next if $s =~ /^libc./;
            $cmd = "cp $s lib/";
            print "$cmd\n";
            print `$cmd`;
            ++$added;
        }
    }
    return $added;
}


while( addsyslibs() ) {};

# add custom files

## us.sh
$f = "bin/us.sh";
$ftxt = <<__EOD;
#!/bin/bash

DIR="\$( cd "\$( dirname "\${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
    
exec env PATH=\$DIR:\$PATH LD_LIBRARY_PATH=\$DIR/../lib:$LD_LIBRARY_PATH \$DIR/us
__EOD

open OUT, ">$f";
print OUT $ftxt;
close OUT;
docmd( "chmod a+rx $f" );

## us_somo.sh
$f = "bin/us_somo.sh";
$ftxt = <<__EOD;
#!/bin/bash

DIR="\$( cd "\$( dirname "\${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
    
exec env PATH=\$DIR:\$PATH LD_LIBRARY_PATH=\$DIR/../lib:$LD_LIBRARY_PATH \$DIR/us3_somo
__EOD

open OUT, ">$f";
print OUT $ftxt;
close OUT;
docmd( "chmod a+rx $f" );
    
## us_env.sh
$f = "bin/us_env.sh";
$ftxt = <<__EOD;
#!/bin/bash

DIR="\$( cd "\$( dirname "\${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
    
export PATH=\$DIR:\$PATH LD_LIBRARY_PATH=\$DIR/../lib:$LD_LIBRARY_PATH
__EOD
    
open OUT, ">$f";
print OUT $ftxt;
close OUT;
docmd( "chmod a+rx $f" );

# make tar

chdir $basedir;
docmd( "tar Jcf $dd.tar.xz $dd" );
    


