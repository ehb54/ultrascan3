#!/usr/bin/perl

$debug++;
$exec++;
$part1++;
# $part2++;

$| = 1;

sub cmd {
    print "$_[0]\n" if $debug;
    $result = `$_[0]` if $exec;
    print $result;
}

$us3 = $ENV{'us3'} || die "can't find env variable us3\n";
$us2sdev = $ENV{'us2sdev'} || die "can't find env variable us2dev\n";

print "us3 is $us3\n";
print "us2sdev is $us2sdev\n";

chdir "$us3/somo/develop" || die "can't change to dir $us3/somo/develop $!\n";

if ( $part1 ) {
    cmd("rm -fr * > /dev/null");
    
    cmd("cp -r $us2sdev/* .");
    cmd("rm -fr src/saveit include.old include/saveit us*/Makefile 2> /dev/null");
    cmd("cp -r ../arc/* .");
    cmd('qt3to4 -alwaysoverwrite `find . -name "*.pri"`');
    cmd('qt3to4 -alwaysoverwrite `find . -name "*.pro"`');
    cmd('qt3to4 -alwaysoverwrite `find . -name "*.h"`');
    cmd('qt3to4 -alwaysoverwrite `find . -name "*.cpp"`');
}

cmd('sed \'s/QMAKE_EXTRA_UNIX_TARGETS/QMAKE_EXTRA_TARGETS/g\' libus-somo.pro > libus-somo.pro.new; mv libus-somo.pro.new libus-somo.pro');
cmd('perl ../postproccpp.pl');

if ( $part2 ) {
    cmd('qmake libus_somo.pro');
    cmd('rm qmake_image_collection.cpp');
    cmd('make -j1 src/obj/qmake_image_collection.o');
    cmd('make -j4');
    cmd('qmake us_somo.pro');
    cmd('make -j4');
}



