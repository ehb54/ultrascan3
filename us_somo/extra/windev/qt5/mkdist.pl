#!/usr/bin/perl

$somobin = $ENV{ "SOMOBIN" } || die "environment variable SOMOBIN should be set to somo base directory\n";

# basic setup --------------------------------------------------------------------------------

$dest = "/c/dist";

# format : from directory or file, dest

@fromto = (
    "$somobin/bin"
    ,"$dest"

    ,"$somobin/somo/doc/manual/somo"
    ,"$dest/somo/doc/manual"

    ,"$somobin/somo/demo"
    ,"$dest/somo"

    ,"$somobin/COPYING"
    ,"$dest"

    ,"$somobin/LICENSE"
    ,"$dest"
    
    ,"$somobin/sounds"
    ,"$dest"
    
    ,"$somobin/etc"
    ,"$dest"

    ,"/mingw64/share/qt5/plugins/platforms/qminimal.dll"
    ,"$dest/bin/platforms"

    ,"/mingw64/share/qt5/plugins/platforms/qwindows.dll"
    ,"$dest/bin/platforms"
    );

# these will have their dependencies copied 

@ldds = (
    "$somobin/bin/us3_somo.exe"
    ,"$somobin/bin/us_admin.exe"
    ,"$somobin/bin/us3_config.exe"
    );



# end basic setup --------------------------------------------------------------------------------
# after here should not need editing

die "install rsync with 'pacman -S rsync\n" if !-e "/usr/bin/rsync";

sub mymkdir {
    my $d = shift;
    
    if ( !-d $d ) {
        warn "making directory $d since it doesn't exist\n";
        print `mkdir -p $d`;
        die "could not make directory $d\n" if !-d $dest;
    }
}

sub runcmd {
    my $c = shift;

    print "\$ $c\n";
    print `$c`;
}

mymkdir( $dest );
    
foreach my $k ( @ldds ) {
    my @extra = `ldd $k | grep -v WINDOWS | awk '{ print \$3 }' | sort -u`;
    grep chomp, @extra;
    foreach my $j ( @extra ) {
        $extrabin{ $j }++ if $j !~ /^\?/;
    }        
}

$extracmd = "rsync " . ( join ' ', keys %extrabin ) . " $dest/bin";

for ( my $i = 0; $i < @fromto; $i += 2 ) {
    my $s = $fromto[ $i ];
    my $d = $fromto[ $i + 1 ];

    if ( !-d $d ) {
        mymkdir( $d );
    }

    my $cmd = "rsync -av $s $d";
    runcmd( $cmd );
}
    
runcmd( $extracmd );

$usrev = `head -1 $somobin/develop/include/us_revision.h | awk '{ print \$4 }' | sed s/\\\"//`;
chomp $usrev;
print "now run createinstall using output->setup executable:\nus3somo-${usrev}-win64-setup.exe\n";
