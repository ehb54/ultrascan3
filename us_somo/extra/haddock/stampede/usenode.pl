#!/usr/bin/perl

$nl = $ENV{'SLURM_NODELIST'} || die "$0: SLURM_NODELIST must be defined\n";

$arg = shift || die "usage $0 offset";

# e.g. env SLURM_NODELIST="c559-[803-804,901-904],c560-[001-004,101,402-404,501-502]" ~/bin/usenode.pl
# e.g. env SLURM_NODELIST="c557-902"  ~/bin/usenode.pl
# e.g. env SLURM_NODELIST="c557-[902,904]"  ~/bin/usenode.pl
# e.g. env SLURM_NODELIST="c557-[902-904]"  ~/bin/usenode.pl

# separate ranges into comma

while ( length( $nl ) )
{
    die "$0: unrecognized token $nl\n" if  $nl !~ /^(c\d+)-/;
    $nodebase = $1;
    $nl =~ s/^${nodebase}-//;
    print "nl st0 $nl\n";
    if ( $nl =~ /^\[/ )
    {
        print "got one\n";
        $nl =~ s/^\[//;
        $do_break = 0;
        while ( $nl !~ /^\]/ && !$do_break )
        {
            print "brace loop\n";
            if ( $nl =~ /^(\d+),/ )
            {
                $u = $1;
                print "single comma $u\n";
                push @n, "${nodebase}-$u";
                $nl =~ s/^$u,//;
                next;
            }
            if ( $nl =~ /^(\d+)-(\d+),/ )
            {
                $start = $1;
                $end   = $2;
                print "start $start end $end\n";
                for ( $i = $start; $i <= $end; ++$i )
                {
                    $ui = '0'x( length( $start ) - length( $i ) ) . $i;
                    push @n, "${nodebase}-${ui}";
                }
                $nl =~ s/^${start}-${end},//;
                next;
            }
            if ( $nl =~ /^(\d+)-(\d+)\]/ )
            {
                $start = $1;
                $end   = $2;
                print "start $start end $end\n";
                for ( $i = $start; $i <= $end; ++$i )
                {
                    $ui = '0'x( length( $start ) - length( $i ) ) . $i;
                    push @n, "${nodebase}-${ui}";
                }
                $nl =~ s/^${start}-${end}\]//;
                $do_break = 1;
            } 
            if ( !$do_break && $nl =~ /^(\d+)\]/ )
            {
                $u = $1;
                print "single close ] $u\n";
                push @n, "${nodebase}-$u";
                $nl =~ s/^$u\]//;
                print "nl is now <$nl>\n";
                $do_break = 1;
            }
            print "no loop\n";
            die "$0: error: unknown token $nl\n" if !$do_break;
        }
    }
    if ( $nl =~ /^(\d+)/ )
    {
        push @n, "${nodebase}-$1";
        $nl =~ s/^$1//;
        print "nl st1 $nl\n";
    }
    if ( $nl =~ /^,/ )
    {
        $nl =~ s/^,//;
        print "nl st2 $nl\n";
    }
}

for ( $i = 0; $i < @n; $i++ )
{
    print "$i: $n[ $i ]\n";
}

print $n[ ( $arg - 1 ) % @n ] . "\n";
