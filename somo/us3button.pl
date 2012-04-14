#!/usr/bin/perl

# $debug++;

$dir = shift || die "usage: $0 develop directory\n";
die "usage: $0 develop directory\n" if $dir =~ /^-/ || !-d $dir;

$us3 = $ENV{'us3'} || die "can't find env variable us3\n";

while ( $ARGV[ 0 ] =~ /^-/ )
{
    $arg = shift;
    
    if ( $arg =~ /-d/ )
    {
        $debug++;
        next;
    }

    if ( $arg =~ /-c/ )
    {
        $commit++;
        print "commit\n";
        next;
    }

    die "usage: $0 { args } { file }
 valid options:
  -d debug
  -c change code ( otherwise, just leaves new code in file-us3conv )
";
}

@exclude = (
            "$dir/include/us_hydrodyn_pdb_parsing.h",
            "$dir/include/us_hydrodyn_sas_options_sans.h",
            "$dir/include/us_hydrodyn_sas_options_hydration.h",
            "$dir/include/us_hydrodyn_pdb_visualization.h",
            "$dir/include/us_hydrodyn_sas_options_bead_model.h",
            "$dir/include/us_hydrodyn_bd_options.h",
            "$dir/include/us_hydrodyn_advanced_config.h",
            "$dir/include/us_hydrodyn_save.h",
#            "$dir/include/us_hydrodyn_saxs.h",
            "$dir/include/us_hydrodyn_dmd_options.h",
            "$dir/include/us_hydrodyn_batch.h",
            "$dir/include/us_hydrodyn_anaflex_options.h",
            "$dir/include/us_hydrodyn.h",
            "$dir/include/us_hydrodyn_sas_options_saxs.h",
            "$dir/include/us_hydrodyn_grid.h",
            "$dir/include/us_hydrodyn_addresidue.h",
            "$dir/include/us_hydrodyn_saxs_buffer.h",
            "$dir/include/us_hydrodyn_hydro.h",
            "$dir/include/us_hydrodyn_misc.h",
            "$dir/include/us_hydrodyn_asa.h",
            "$dir/include/us_hydrodyn_bd.h",
            "$dir/include/us_hydrodyn_sas_options_experimental.h",
            "$dir/include/us_hydrodyn_sas_options_guinier.h",
            "$dir/include/us_hydrodyn_sas_options_misc.h",
            "$dir/include/us_hydrodyn_comparative.h",
            "$dir/include/us_hydrodyn_sas_options_curve.h",
            "$dir/include/us_hydrodyn_bead_output.h",
            "$dir/src/us_hydrodyn_asa.cpp",
            "$dir/src/us_hydrodyn_saxs_buffer.cpp",
            "$dir/src/us_hydrodyn_bead_output.cpp",
            "$dir/src/us_hydrodyn_pdb_visualization.cpp",
            "$dir/src/us_hydrodyn_pdb_parsing.cpp",
            "$dir/src/us_hydrodyn_hydro.cpp",
            "$dir/src/us_hydrodyn_batch.cpp",
            "$dir/src/us_hydrodyn_grid.cpp",
            "$dir/src/us_hydrodyn_bd_options.cpp",
            "$dir/src/us_hydrodyn_addresidue.cpp",
#            "$dir/src/us_hydrodyn_saxs.cpp"
            );
for ( $i = 0; $i < @exclude; $i++ )
{
    $excl{ $exclude[ $i ] }++;
}

# find all files with q3button

@files = `find $dir -name 'us*.cpp' -o -name 'us*.h' | xargs grep -li q3button`;
grep chomp, @files;

for ( $i = 0; $i < @files; $i++ )
{
    if ( $excl{ $files[ $i ] } )
    {
        print "excluding $files[ $i ]\n";
        next;
    }

    $fin = $files[ $i ];
    $f = $fin;
    open IN, $f || die "$0: $f $!\n";
    print "processing $f\n";

    $fout = "$files[ $i ]-u3conv";
    $f = $fout;
    open OUT, ">$f" || die "$0: $f $!\n";

    $hmode = $fin =~ /\.h$/;

    undef $bg_pos_defined;
    undef %bg_names;
    undef $add_bg_setAllEnabled;
    undef $add_bg_setAllMinimumHeight;

# precheck
    while ( <IN> )
    {
        if ( /(\s+bg_\w+\s*->\s*)setEnabled\(\s*(\w+)\s*\)/ )
        {
            $add_bg_setAllEnabled++;
        }
        if ( /(\s+bg_\w+\s*->\s*)setMinimumHeight\(\s*(\w+)\s*\)/ )
        {
            $add_bg_setAllMinimumHeight++;
        }
        last if $add_bg_setAllEnabled && $add_bg_setAllMinimumHeight;
    }
    close IN;
    $f = $fin;
    open IN, $f || die "$0: $f $!\n";

    if ( $add_bg_setAllEnabled || $add_bg_setAllMinimumHeight )
    {
        print OUT '
#include <QAbstractButton>
#include <QButtonGroup>
'          ;
    }

    if ( $add_bg_setAllEnabled )
    {
        print OUT '
static void bg_setAllEnabled( QButtonGroup *bg, bool enabled )
{
    QList < QAbstractButton * > buttons = bg->buttons();
    for ( QList < QAbstractButton * >::iterator it = buttons.begin();
          it != buttons.end();
          it++ )
    {
        (*it)->setEnabled( enabled );
    }
}
'       ;
    }

    if ( $add_bg_setAllMinimumHeight )
    {
        print OUT '
static void bg_setAllMinimumHeight( QButtonGroup *bg, int height )
{
    QList < QAbstractButton * > buttons = bg->buttons();
    for ( QList < QAbstractButton * >::iterator it = buttons.begin();
          it != buttons.end();
          it++ )
    {
        (*it)->setMinimumHeight( height );
    }
}
'       ;
    }


    while ( <IN> )
    {
        if ( $hmode )
        {
            s/q3buttongroup\.h/qbuttongroup.h/;
            s/Q3ButtonGroup/QButtonGroup/;
            print OUT $_;
            next;
        } 

        if ( /(^\s+)(bg_\w+)\s*=\s*new\s+Q3ButtonGroup/ )
        {
            $spaces  = $1;
            $bg_name = $2;
            $bg_names{ $bg_name }++;
            print OUT "$spaces$bg_name = new QButtonGroup( this );\n";
            if ( !$bg_pos_defined )
            {
                $bg_pos_defined++;
                print OUT "${spaces}int bg_pos = 0;\n";
            } else {
                print OUT "${spaces}bg_pos = 0;\n";
            }
            next;
        }            

        s/(${bg_name}\s*->\s*set)RadioButtonExclusive/${1}Exclusive/;
        s/(${bg_name}\s*->\s*)insert\(\s*(\w+)\s*\)/${1}addButton( ${2}, bg_pos++ )/;
        s/clicked/buttonClicked/ if /connect\s*\(\s*${bg_name}\s*,/;

        s/(\s+bg_\w+\s*->\s*)setButton\(\s*(.+)\s*\)\s*;/${1}button( ${2} )->setChecked( true );/;

        s/(\s+)(bg_\w+)\s*->\s*setEnabled\(\s*(\w+)\s*\)/${1}bg_setAllEnabled( ${2}, ${3} )/;

        s/(\s+)(bg_\w+)\s*->\s*setMinimumHeight\(\s*(\w+)\s*\)/${1}bg_setAllMinimumHeight( ${2}, ${3} )/;

        if ( /(\s+)(\w+)\s*=\s*new\s+QCheckBox\s*\(\s*(bg_\w+)\s*\)/ )
        {
           print OUT "$1$2 = new QCheckBox( 0 );
$1$3->addButton( $2, bg_pos++ );
"            ;
           next;
        }

        print OUT $_;
    }
    close IN;

    close OUT;
    if ( $commit )
    {
        `mv $fout $fin`;
        print "updated $fin\n";
    } else {
        print "created $fout\n";
    }
}
