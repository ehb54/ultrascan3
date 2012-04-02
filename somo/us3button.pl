#!/usr/bin/perl

# $debug++;

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
        next;
    }

    die "usage: $0 { args } { file }
 valid options:
  -d debug
  -c change code ( otherwise, just leaves new code in file-us3conv )
";
}

@exclude = (
            'develop/include/us_hydrodyn_pdb_parsing.h',
            'develop/include/us_hydrodyn_sas_options_sans.h',
            'develop/include/us_hydrodyn_sas_options_hydration.h',
            'develop/include/us_hydrodyn_pdb_visualization.h',
            'develop/include/us_hydrodyn_sas_options_bead_model.h',
            'develop/include/us_hydrodyn_bd_options.h',
            'develop/include/us_hydrodyn_advanced_config.h',
            'develop/include/us_hydrodyn_save.h',
#            'develop/include/us_hydrodyn_saxs.h',
            'develop/include/us_hydrodyn_dmd_options.h',
            'develop/include/us_hydrodyn_batch.h',
            'develop/include/us_hydrodyn_anaflex_options.h',
            'develop/include/us_hydrodyn.h',
            'develop/include/us_hydrodyn_sas_options_saxs.h',
            'develop/include/us_hydrodyn_grid.h',
            'develop/include/us_hydrodyn_addresidue.h',
            'develop/include/us_hydrodyn_saxs_buffer.h',
            'develop/include/us_hydrodyn_hydro.h',
            'develop/include/us_hydrodyn_misc.h',
            'develop/include/us_hydrodyn_asa.h',
            'develop/include/us_hydrodyn_bd.h',
            'develop/include/us_hydrodyn_sas_options_experimental.h',
            'develop/include/us_hydrodyn_sas_options_guinier.h',
            'develop/include/us_hydrodyn_sas_options_misc.h',
            'develop/include/us_hydrodyn_comparative.h',
            'develop/include/us_hydrodyn_sas_options_curve.h',
            'develop/include/us_hydrodyn_bead_output.h',
            'develop/src/us_hydrodyn_asa.cpp',
            'develop/src/us_hydrodyn_saxs_buffer.cpp',
            'develop/src/us_hydrodyn_bead_output.cpp',
            'develop/src/us_hydrodyn_pdb_visualization.cpp',
            'develop/src/us_hydrodyn_pdb_parsing.cpp',
            'develop/src/us_hydrodyn_hydro.cpp',
            'develop/src/us_hydrodyn_batch.cpp',
            'develop/src/us_hydrodyn_grid.cpp',
            'develop/src/us_hydrodyn_bd_options.cpp',
            'develop/src/us_hydrodyn_addresidue.cpp',
#            'develop/src/us_hydrodyn_saxs.cpp'
            );
for ( $i = 0; $i < @exclude; $i++ )
{
    $excl{ $exclude[ $i ] }++;
}

# find all files with q3button

@files = `find develop -name 'us*.cpp' -o -name 'us*.h' | xargs grep -li q3button`;
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
