#!/usr/bin/perl
# gen_synthetic.pl - synthetic SAXS curves with analytically known Rg for Guinier search validation
#
# usage: gen_synthetic.pl [outdir]
# writes outdir/*.dat and outdir/truth.csv ( name, rg_true, i0_true, notes )

use strict;
use warnings;
use Math::Trig;

my $outdir = shift // "synth";
mkdir $outdir unless -d $outdir;

srand( 20260914 );

# ---------------------------------------------------------------- form factors

sub p_sphere {
   my ( $q, $R ) = @_;
   my $x = $q * $R;
   return 1 if $x < 1e-6;
   my $f = 3 * ( sin( $x ) - $x * cos( $x ) ) / ( $x ** 3 );
   return $f * $f;
}

# ellipsoid of revolution, semi-axes a, a, nu*a ; orientation averaged
sub p_ellipsoid {
   my ( $q, $a, $nu ) = @_;
   my $n   = 400;
   my $sum = 0;
   for my $k ( 0 .. $n - 1 ) {
      my $th = ( $k + .5 ) * ( pi / 2 ) / $n;
      my $r  = $a * sqrt( sin( $th ) ** 2 + $nu * $nu * cos( $th ) ** 2 );
      $sum += p_sphere( $q, $r ) * sin( $th );
   }
   return $sum * ( pi / 2 ) / $n;
}

# Bessel J1, Numerical Recipes polynomial approximation
sub bessj1 {
   my $x  = shift;
   my $ax = abs( $x );
   if ( $ax < 8 ) {
      my $y = $x * $x;
      my $a1 = $x * ( 72362614232.0 + $y * ( -7895059235.0 + $y * ( 242396853.1 + $y * ( -2972611.439 + $y * ( 15704.48260 + $y * ( -30.16036606 ) ) ) ) ) );
      my $a2 = 144725228442.0 + $y * ( 2300535178.0 + $y * ( 18583304.74 + $y * ( 99447.43394 + $y * ( 376.9991397 + $y ) ) ) );
      return $a1 / $a2;
   }
   my $z  = 8 / $ax;
   my $y  = $z * $z;
   my $xx = $ax - 2.356194491;
   my $a1 = 1.0 + $y * ( 0.183105e-2 + $y * ( -0.3516396496e-4 + $y * ( 0.2457520174e-5 + $y * ( -0.240337019e-6 ) ) ) );
   my $a2 = 0.04687499995 + $y * ( -0.2002690873e-3 + $y * ( 0.8449199096e-5 + $y * ( -0.88228987e-6 + $y * 0.105787412e-6 ) ) );
   my $ans = sqrt( 0.636619772 / $ax ) * ( cos( $xx ) * $a1 - $z * sin( $xx ) * $a2 );
   return $x < 0 ? -$ans : $ans;
}

# cylinder radius R length L, orientation averaged
sub p_cylinder {
   my ( $q, $R, $L ) = @_;
   my $n   = 400;
   my $sum = 0;
   for my $k ( 0 .. $n - 1 ) {
      my $al = ( $k + .5 ) * ( pi / 2 ) / $n;
      my $u  = $q * $R * sin( $al );
      my $v  = $q * $L * cos( $al ) / 2;
      my $fu = $u < 1e-6 ? 1 : 2 * bessj1( $u ) / $u;
      my $fv = $v < 1e-6 ? 1 : sin( $v ) / $v;
      $sum += ( $fu * $fv ) ** 2 * sin( $al );
   }
   return $sum * ( pi / 2 ) / $n;
}

# ---------------------------------------------------------------- helpers

sub gauss {
   my ( $u1, $u2 ) = ( rand(), rand() );
   $u1 = 1e-12 if $u1 < 1e-12;
   return sqrt( -2 * log( $u1 ) ) * cos( 2 * pi * $u2 );
}

sub grid {
   my ( $qmin, $qmax, $n ) = @_;
   return map { $qmin + ( $qmax - $qmin ) * $_ / ( $n - 1 ) } 0 .. $n - 1;
}

my @truth;

# write a curve; opts: noise (relative at I0), sdcol (0/1), sdscale (reported sd = true sd * sdscale),
# qscale (multiply written q), sep (separator), header lines, transform (code ref on I), fmt
sub emit {
   my ( $name, $rg, $i0, $qs, $model, %o ) = @_;
   my $noise   = $o{ noise }   // 0;
   my $sdcol   = $o{ sdcol }   // 1;
   my $sdscale = $o{ sdscale } // 1;
   my $qscale  = $o{ qscale }  // 1;
   my $sep     = $o{ sep }     // "\t";
   my @header  = @{ $o{ header } // [ "# $name  Rg_true $rg  I0_true $i0" ] };
   my $trans   = $o{ transform };
   my $notes   = $o{ notes } // "";

   open my $fh, ">", "$outdir/$name.dat" or die;
   print $fh "$_\n" for @header;
   for my $q ( @$qs ) {
      my $I  = $i0 * $model->( $q );
      $I = $trans->( $q, $I ) if $trans;
      my $sd = $noise * $i0 * sqrt( abs( $I ) / $i0 + 1e-4 );
      my $Iobs = $I + ( $noise ? $sd * gauss() : 0 );
      my @cols = ( sprintf( "%.6e", $q * $qscale ), sprintf( "%.6e", $Iobs ) );
      push @cols, sprintf( "%.6e", $sd * $sdscale ) if $sdcol;
      print $fh join( $sep, @cols ), "\n";
   }
   close $fh;
   push @truth, [ "$name.dat", $rg, $i0, $notes ];
}

# ---------------------------------------------------------------- cases

my $R30   = 30;               my $rg30  = sqrt( 3 / 5 ) * $R30;      # 23.238
my $R100  = 100;              my $rg100 = sqrt( 3 / 5 ) * $R100;     # 77.46
my ( $ea, $enu ) = ( 20, 3 ); my $rgel  = sqrt( ( 2 * $ea ** 2 + ( $enu * $ea ) ** 2 ) / 5 );  # 29.66
my ( $cR, $cL )  = ( 20, 200 ); my $rgcy = sqrt( $cR ** 2 / 2 + $cL ** 2 / 12 );               # 59.44

my $sph30  = sub { p_sphere( $_[ 0 ], $R30 ) };
my $sph100 = sub { p_sphere( $_[ 0 ], $R100 ) };
my $ell    = sub { p_ellipsoid( $_[ 0 ], $ea, $enu ) };
my $cyl    = sub { p_cylinder( $_[ 0 ], $cR, $cL ) };

my @g500  = grid( 0,     0.5, 501 );
my @g005  = grid( 0.005, 0.5, 496 );
my @g60   = grid( 0,     0.3, 61 );
my @g2000 = grid( 0.002, 0.5, 2000 );
my @gbig  = grid( 0.008, 0.3, 293 );

# clean references
emit( "sphere30_clean_nosd",    $rg30, 1000, \@g500, $sph30, noise => 0,    sdcol => 0, notes => "no noise, 2 columns" );
emit( "sphere30_clean_sd",      $rg30, 1000, \@g500, $sph30, noise => 0.005, notes => "0.5% noise, correct sd" );
emit( "sphere30_noise2",        $rg30, 1000, \@g500, $sph30, noise => 0.02,  notes => "2% noise" );
emit( "sphere30_noise5",        $rg30, 1000, \@g500, $sph30, noise => 0.05,  notes => "5% noise" );
emit( "sphere30_noise10",       $rg30, 1000, \@g500, $sph30, noise => 0.10,  notes => "10% noise" );
emit( "sphere30_sd_underest",   $rg30, 1000, \@g500, $sph30, noise => 0.02, sdscale => 0.3, notes => "sd column 3x too small -> chi2_red >> 1" );
emit( "sphere30_sd_overest",    $rg30, 1000, \@g500, $sph30, noise => 0.02, sdscale => 3,   notes => "sd column 3x too large" );
emit( "ellipsoid_noise2",       $rgel, 1000, \@g500, $ell,   noise => 0.02,  notes => "prolate ellipsoid nu=3" );
emit( "cylinder_noise2",        $rgcy, 1000, \@g005, $cyl,   noise => 0.02,  notes => "cylinder R20 L200, q starts at 0.005 (qRg 0.30)" );
emit( "sphere100_truncated",    $rg100, 1000, \@gbig, $sph100, noise => 0.01, notes => "q starts at 0.008 (qRg 0.62), only ~9 points to qRg 1.3 at dq 0.001" );
emit( "sphere30_coarse60",      $rg30, 1000, \@g60,  $sph30, noise => 0.02,  notes => "61 points 0..0.3" );
emit( "sphere30_fine2000",      $rg30, 1000, \@g2000, $sph30, noise => 0.02, notes => "2000 points 0.002..0.5" );

# pathological
my $agg = sub { my $q = shift; 0.95 * p_sphere( $q, $R30 ) + 0.05 * p_sphere( $q, 150 ) };
emit( "sphere30_aggregated",    $rg30, 1000, \@g500, $agg,   noise => 0.02,  notes => "5% (by I0) of R=150 spheres added; Rg_true is the monomer" );
my $rep = sub { my $q = shift; p_sphere( $q, $R30 ) * ( 1 - 0.3 * exp( -( $q * 60 ) ** 2 ) ) };
emit( "sphere30_repulsive",     $rg30, 1000, \@g500, $rep,   noise => 0.02,  notes => "structure factor 1-0.3exp(-(60q)^2); Rg_true is the form factor" );
emit( "sphere30_negtail",       $rg30, 1000, \@g500, $sph30, noise => 0.02, transform => sub { $_[ 1 ] - 0.002 * 1000 }, notes => "0.2% I0 over-subtracted: high-q goes negative" );
emit( "sphere30_zerosd",        $rg30, 1000, \@g500, $sph30, noise => 0.02, transform => undef, sdcol => 1, sdscale => 1, notes => "some sd zeroed (post-processed)" );
emit( "sphere30_nm",            $rg30, 1000, \@g500, $sph30, noise => 0.02, qscale => 10, notes => "q in nm^-1, needs qscale 0.1" );
emit( "sphere30_csv",           $rg30, 1000, \@g500, $sph30, noise => 0.02, sep => ",", header => [ "q,I,sd" ], notes => "comma separated with text header" );
emit( "sphere30_atsasheader",   $rg30, 1000, \@g500, $sph30, noise => 0.02, header => [ "Sample description: synthetic sphere", "Sample:   c= 1.0 mg/ml", "" ], notes => "multi-line text header" );
emit( "rising_nonsense",        0,     1000, \@g500, sub { 1 + 5 * $_[ 0 ] ** 2 }, noise => 0.01, notes => "increasing intensity: must fail" );
emit( "sphere30_toofew",        $rg30, 1000, [ grid( 0, 0.05, 6 ) ], $sph30, noise => 0.01, notes => "6 points only: must fail with minpts 10" );


# Debye Gaussian chain ( IDP-like ), Rg exact
my $rgd = 30;
my $debye = sub { my $q = shift; my $x = ( $q * $rgd ) ** 2; $x < 1e-8 ? 1 : 2 * ( exp( -$x ) - 1 + $x ) / ( $x * $x ) };
emit( "debye_rg30_noise2",      $rgd,  1000, \@g500, $debye, noise => 0.02,  notes => "Gaussian chain (Debye), Guinier valid only to qRg~1" );

# post-process: zero every 7th sd in sphere30_zerosd
{
   my $f = "$outdir/sphere30_zerosd.dat";
   open my $in, "<", $f or die; my @l = <$in>; close $in;
   open my $out, ">", $f or die;
   my $k = 0;
   for ( @l ) {
      if ( /^\s*[\d.]/ ) { my @c = split /\t/; $c[ 2 ] = "0.000000e+00\n" if ( $k++ % 7 ) == 3; $_ = join "\t", @c; }
      print $out $_;
   }
   close $out;
}

open my $t, ">", "$outdir/truth.csv" or die;
print $t "name,rg_true,i0_true,notes\n";
print $t join( ",", $_->[ 0 ], sprintf( "%.4f", $_->[ 1 ] ), $_->[ 2 ], "\"$_->[ 3 ]\"" ), "\n" for @truth;
close $t;
print scalar( @truth ), " curves written to $outdir\n";
