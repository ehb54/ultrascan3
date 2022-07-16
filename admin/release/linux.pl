#!/usr/bin/perl

$qt  = $ENV{QTDIR}     || die "environment variable QTDIR must be defined (did you \$ module swap ultrascan/gui-build )";
$qwt = $ENV{QWTDIR}    || die "environment variable QWTDIR must be defined";
$us  = $ENV{ULTRASCAN} || die "environment variable ULTRASCAN must be defined";

$notes = "usage: $0 sysname
sysname is something like ubuntu20 or Linux
ultrascandir is the place ultrascan was compiled
$qt must exist or change \$qt in this code
$qwt must exist or change \$qwt in this code
";

$sn = shift || die $notes;

die "$us does not exist\n" if !-e $us;
die "$us is not a directory\n" if !-d $us;

die "$qt does not exist\n" if !-e $qt;
die "$qt is not a directory\n" if !-d $qt;
die "$qwt does not exist\n" if !-e $qwt;
die "$qwt is not a directory\n" if !-d $qwt;

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
     "$us/bin"                  , "bin"
     ,"$us/etc"                 , "etc"
     ,"$us/lib"                 , "lib"
     ,"$us/somo/doc"            , "somo/doc"
     ,"$us/somo/demo"           , "somo/demo"
     ,"$qt/lib"                 , "lib"
     ,"$qwt/lib"                , "lib"
     ,"$qt/plugins"             , "plugins"
     ,"$us/us_somo/add_to_bin"  , "bin"
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

## cleanups

@cleanups =
    (
     "*~"
     ,"*.cmake"
     ,"*.prl"
     ,"metatypes"
     ,"pkgconfig"
     ,"cmake"
     ,"*.la"
     ,"*.a"
    );

# run cleanups
for $rm ( @cleanups ) {
    docmd( "find . -name \"$rm\" | xargs rm -r 2> /dev/null" );
}

docmd( "ln -s bin bin64" );

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
            $cmd = "(env LD_LIBRARY_PATH=$pwd/lib ldd $cdir/$f | sed -z 's/\\n\\s*=>/ =>/g' | grep -v $dd | grep '=>' | awk '{ print \$3 }' ) 2> /dev/null";
            # print "$cmd\n";
            my @addlib = `$cmd`;
            grep chomp, @addlib;
            for my $addlib ( @addlib ) {
                $s{ $addlib }++;
            }
        }
        for my $s ( keys %s ) {
            ## next if $s =~ /^libc./;
            die "missing library!\n" if $s =~ /not found/;
            my $snp = $s;
            $snp =~ s/^.*\///;
            next if -e "lib/$snp";
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

$opensslname = 'redhat';
$opensslname = 'ubuntu' if lc($sn) =~ /^ubuntu/;

$ftxt = <<__EOD;
#!/bin/bash

DIR="\$( cd "\$( dirname "\${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
    
VERSION_ID=`grep -P '^VERSION_ID=' /etc/os-release | awk -F= '{ print \$2 }' | sed 's/"//g'`
export OPENSSL_CONF=\$DIR/../etc/openssl.cnf.d/${opensslname}\${VERSION_ID}.cnf

exec env PATH=\$DIR:\$PATH LD_LIBRARY_PATH=\$DIR/../lib:$$LD_LIBRARY_PATH \$DIR/us
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
    
exec env PATH=\$DIR:\$PATH LD_LIBRARY_PATH=\$DIR/../lib:$$LD_LIBRARY_PATH \$DIR/us3_somo
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
docmd( "XZ_DEFAULTS='-T 0 --fast' tar Jcf $dd.tar.xz $dd" );
    


