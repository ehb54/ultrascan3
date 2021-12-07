#!/usr/bin/perl

## user defines
$cores       = 10;
$gitroot     = "/ultrascan3t";
$codeqldb    = "us3t";
$outfile     = "codeqlout.txt";

## somo paths

$somoroot    = "$gitroot/us_somo/develop";
$somorootcli = "$gitroot/us_somo/develop/us_saxs_cmds_t";

## end user defines


$notes = "usage: $0 sourcefile {cli}

sourcefile is a file with the .cpp files listed

options 
cli    -  process the cli version (e.g. in $somorootcli)

";

$sources = shift || die $notes;
$usecli  = shift;
if ( length( $usecli ) ) {
    die $notes if $usecli ne 'cli';
    $cli = 1;
} else {
    $cli = 0;
}

$cwd = `pwd`;
chomp $cwd;

$useroot = $cli ? $somorootcli : $somoroot;

$codeqlcmd1  = "/codeql/codeql/codeql database create $codeqldb --language=cpp --source-root $useroot --overwrite --command 'make -j$cores ";
$codeqlcmd2  = "/codeql/codeql/codeql database analyze $codeqldb --format=sarif-latest --output=$cwd/$outfile --threads $cores";

die "$sources does not exist\n" if !-e $sources;
die "$sources is not readable\n" if !-r $sources;

@s = `cat $sources`;
grep chomp, @s;

die "$sources is empty\n" if !@s;

$prefix = $cli ? "" : "src/obj/";

grep s/^.*\///, @s;
grep s/^/$prefix/, @s;
grep s/\.cpp$/.o/, @s;

$cmd = sprintf( "( cd $useroot && make clean 2>&1 > /dev/null && cd $gitroot && $codeqlcmd1 %s' && $codeqlcmd2 && cd $cwd )", join( ' ', @s ) );

print "$cmd\n";



