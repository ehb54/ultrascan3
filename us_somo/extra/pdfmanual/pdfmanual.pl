#!/usr/bin/perl

$dpi = shift || 300;
$dpi = 50 if $dpi < 50;

$maxw = 10000;

$debug++;
 $doit++;

$us = `cd ../.. && pwd`;
chomp $us;

print "using docs in $us\n";

sub docmd { 
    my $cmd = $_[0];
    print "$cmd\n" if $debug;
    print `$cmd` if $doit;
}

$tdir = "$us/extra/pdfmanual/tmp";
$odir = "$us/extra/pdfmanual/out";

docmd( "rm -r $tdir $odir 2> /dev/null" );
docmd( "mkdir -p $tdir $odir 2> /dev/null" );
docmd( "cp -R $us/somo/doc/manual/somo/. $tdir/" );

@pdfs = ( "somo_hplc" );

@is = `cd $tdir; identify *.png *jpg`;
grep chomp, @is;
if ( $maxw < 5000 ) {
    foreach $i ( @is ) {
        @isf = split ' ', $i;
        $f = $isf[ 0 ];
        $f =~ s/\[\d+\]$//;
        $s = $isf[ 2 ];
        $s =~ s/x\d+$//;
        if ( $s > $maxw ) {
            docmd( "convert $tdir/$f -resize $maxw $tdir/$f" );
#            docmd( "convert $tdir/$f -units PixelsPerInch -density 120 $tdir/$f" );
        }
    }
}

foreach $i ( @pdfs ) {
    die "$0: Error: $i.txt does not exist\n" if !-e "$i.txt";
    @files = `cat $i.txt`;
    grep chomp, @files;

    foreach $f ( @files ) {
        die "$0: Error: $i file $f  does not exist\n" if !-e "$tdir/$f";
        open IN, "$tdir/$f";
        @l = <IN>;
        close IN;
        $l = join '', @l;
        $l =~ s/\<h1\>.*\<\/h1>//sg;
        open OUT, ">$tdir/$f";
        print OUT $l;
        close OUT;
    }
#    docmd( "cd $tdir; wkhtmltopdf --image-dpi $dpi toc " . ( join ' ', @files ) . " $odir/$i.pdf" );
    docmd( "cd $tdir; wkhtmltopdf --page-size A3 --footer-center \"Page [page] of [toPage]\"  toc " . ( join ' ', @files ) . " $odir/$i.pdf" );
}



# wkhtmltopdf toc somo_saxs_hplc.html saxs_hplc_Gaussian_theory.html somo_saxs_hplc_skewedGauss.html somo_saxs_hplc_svd.html saxs_hplc_options.html saxs_hplc_ciq.html saxs_hplc_dctr.html saxs_hplc_nth.html saxs_hplc_p3d.html somo_saxs_hplc_fit.html somo_saxs_hplc_linear_baselines.html somo_saxs_hplc_movie.html somo_hplc.pdf
