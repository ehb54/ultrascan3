#!/usr/bin/perl

$notes = "usage: $0 
make propri.tgz which includes specific us3 versions of pro & pri files
new programs and edits will be overwritten on next regen of us3 from us2 unless they are in this file
";

$us3 = $ENV{'us3'} || die "can't find env variable us3\n";
#$us3somo = $ENV{'us3somo'} || die "can't find env variable us3somo\n";

print "us3 is $us3\n";
#print "us3somo is $us3somo\n";

@files = (
    "cmdline.pri",
    "generic.pri",
    "libus_somo.pro",
    "local.pri",
    "local.pri.template",
    "local.pri.template.linux",
    "local.pri.template.osx",
    "local.pri.template.win32mingw",
    "uname.pri",
    "us3_config/",
    "us3_config/us3_config.pro",
    "us3_hydrodyn/",
    "us3_hydrodyn/us3_hydrodyn.pro",
    "us_admin/",
    "us_admin/us_admin.pro",
    "us_cluster_server_t/",
    "us_cluster_server_t/us_cluster_server_t.pro",
    "us_config/",
    "us_config/us_config.pro",
    "us_hydrodyn/",
    "us_hydrodyn/us_hydrodyn.pro",
    "us_license/",
    "us_license/us_license.pro",
    "us_saxs_cmds_mpi/",
    "us_saxs_cmds_mpi/us_saxs_cmds_mpi.pro",
    "us_saxs_cmds_mpi_cuda/",
    "us_saxs_cmds_mpi_cuda/us_saxs_cmds_mpi_cuda.pro",
    "us_saxs_cmds_t/",
    "us_saxs_cmds_t/us_saxs_cmds_t.pro",
    "us_somo.pro",
    );

sub docmd {
    my $cmd = $_[0];
    my $trial = $_[1];
    print "$cmd\n";
    my $res;
    if ( !$trial ) {
        $res = `$cmd`;
        print $res;
    } else {
        print "not run -- trial\n";
    }
    chomp $res;
    $res;
}


docmd( "tar zcf propri.tgz " . ( join " ", @files ) );
$uniq = docmd( 'date +\'%Y%m%d%H%M%S\'' );
docmd( "cp $us3/somo/arc/propri.tgz $us3/somo/arc/propri-${uniq}.tgz" );
docmd( "cp propri.tgz $us3/somo/arc/propri.tgz" );


