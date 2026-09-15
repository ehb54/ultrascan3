#!/usr/bin/perl
# compare.pl - tabulate SOMO guinier_search (json lines) vs truth.csv vs ATSAS autorg csv
# usage: compare.pl truth.csv somo.jsonl [atsas.csv]
use strict;
use warnings;

my ( $truthf, $somof, $atsasf ) = @ARGV;
die "usage: compare.pl truth.csv somo.jsonl [atsas.csv]\n" unless $truthf && $somof;

my %truth;
open my $t, "<", $truthf or die "$truthf: $!";
<$t>;
while ( <$t> ) {
   chomp;
   my ( $name, $rg, $i0, $notes ) = /^([^,]+),([^,]+),([^,]+),"(.*)"$/ or next;
   $truth{ $name } = { rg => $rg, i0 => $i0, notes => $notes };
}
close $t;

# minimal flat json reader ( no nesting beyond the warnings array )
sub jget {
   my ( $j, $k ) = @_;
   return $1 if $j =~ /"$k":("(?:[^"\\]|\\.)*"|\[[^\]]*\]|[^,}]+)/;
   return undef;
}

my %somo;
open my $s, "<", $somof or die "$somof: $!";
while ( <$s> ) {
   chomp;
   next unless /^\{/;
   my $name = jget( $_, "name" ); $name =~ s/^"|"$//g; $name =~ s{^.*/}{};
   my %r = map { $_ => jget( $_, $_ ) } ();
   $somo{ $name } = {
      ok      => jget( $_, "ok" ),
      rg      => jget( $_, "rg" ),
      rg_sd   => jget( $_, "rg_sd" ),
      i0      => jget( $_, "i0" ),
      first   => jget( $_, "first" ),
      last    => jget( $_, "last" ),
      npts    => jget( $_, "npts" ),
      qrgmin  => jget( $_, "qrgmin" ),
      qrgmax  => jget( $_, "qrgmax" ),
      quality => jget( $_, "quality" ),
      lowq_z  => jget( $_, "lowq_z" ),
      agg     => jget( $_, "aggregation" ),
      rep     => jget( $_, "repulsion" ),
      sd_used => jget( $_, "sd_used" ),
      spread  => jget( $_, "rg_spread_sd" ),
      err     => jget( $_, "errormsg" ),
      warn    => jget( $_, "warnings" ),
   };
}
close $s;

my %atsas;
if ( $atsasf && -f $atsasf ) {
   open my $a, "<", $atsasf or die;
   <$a>;
   while ( <$a> ) {
      chomp;
      my @c = split /,/;
      my $name = $c[ 0 ]; $name =~ s{^.*/}{};
      $atsas{ $name } = { rg => $c[ 1 ], sd => $c[ 2 ], i0 => $c[ 3 ], first => $c[ 5 ], last => $c[ 6 ], q => $c[ 7 ], agg => $c[ 8 ] };
   }
   close $a;
}

printf "%-26s %8s | %8s %6s %6s %7s %5s %5s %4s %5s | %8s %6s %7s %s\n",
   "name", "Rg_true", "SOMO_Rg", "sd", "err%", "qual", "pts", "qRg1", "flag", "sdw", "ATSAS_Rg", "err%", "qual", "notes";
for my $name ( sort keys %truth ) {
   my $tr = $truth{ $name };
   my $so = $somo{ $name };
   my $at = $atsas{ $name };
   my $soline;
   if ( !$so ) {
      $soline = sprintf "%-8s %6s %6s %7s %5s %5s %4s %5s", "missing", "", "", "", "", "", "", "";
   } elsif ( $so->{ ok } ne "true" ) {
      my $e = $so->{ err } // ""; $e =~ s/^"|"$//g;
      $soline = sprintf "%-51s", "FAIL: " . substr( $e, 0, 45 );
   } else {
      my $err = $tr->{ rg } > 0 ? 100 * ( $so->{ rg } - $tr->{ rg } ) / $tr->{ rg } : 0;
      my $flag = $so->{ agg } eq "true" ? "AGG" : ( $so->{ rep } eq "true" ? "REP" : "-" );
      $soline = sprintf "%8.3f %6.3f %+6.2f %7.3f %5d %5.2f %4s %5s",
         $so->{ rg }, $so->{ rg_sd }, $err, $so->{ quality }, $so->{ npts }, $so->{ qrgmax }, $flag, $so->{ sd_used } eq "true" ? "y" : "n";
   }
   my $atline;
   if ( !$at ) {
      $atline = sprintf "%-8s %6s %7s", "fail", "", "";
   } else {
      my $err = $tr->{ rg } > 0 ? 100 * ( $at->{ rg } - $tr->{ rg } ) / $tr->{ rg } : 0;
      $atline = sprintf "%8.3f %+6.2f %7.3f%s", $at->{ rg }, $err, $at->{ q }, $at->{ agg } ? "A" : "";
   }
   printf "%-26s %8.3f | %s | %s %s\n", substr( $name, 0, 26 ), $tr->{ rg }, $soline, $atline, $tr->{ notes };
}
