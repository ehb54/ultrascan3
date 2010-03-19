#!/usr/bin/perl

# convert email_text into old format for us_tigre_job.pl formatting.

@_ = <>;

if ( !grep /Fit:  Meniscus:/, @_ ) {
    print @_;
    exit;
}

for ( $i = 0; $i < @_; $i++ ) {
    if ( $_[$i] =~ /^Meniscus gridpoints:\s+(\d+)/ ) {
        $gridpoints = $1;
	$gridpoints++ if 2*int($gridpoints/2) != $gridpoints;
    }

    if ( $_[$i] =~ /^Experiment .*, Cell / ) {
        $save = $_[$i];
        $save =~ s/, Cell /, cell /;
        $save =~ s/, Wavelength: /, wavelength /;
        chomp $save;
        $i++;
        next;
    }
        
    if ( !($_[$i] =~ /Fit:  Meniscus:/) ) {
        print $_[$i];
        next;
    }

    $i++;

    # meniscus reprint
    for ( $j = 0; $j < $gridpoints; $j++ ) {
        $i++;
        ( $fit, $meniscus, $rmsd, $solutes, $iter ) = $_[$i] =~ /(\d+)\s+(\S+)\s+(\S+)\s+(\d+)\s+(\d+)/;
        print "$save, meniscus $meniscus, solutes $solutes, rmsd $rmsd, iterations $iter\n";
    }
}
