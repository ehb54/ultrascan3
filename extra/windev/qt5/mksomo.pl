#!/usr/bin/perl

$somobin = $ENV{ "SOMOBIN" } || die "environment variable SOMOBIN should be set to somo base directory\n";

sub runcmd {
    my $c = shift;

    print "\$ $c\n";
    print `$c`;
}

$td = "$somobin/develop";
chdir $td  || die "can not change to $td $!\n";
$cmd = "qmake libus_somo.pro && make -j8 && qmake us_somo.pro && make -j8";
runcmd $cmd;
