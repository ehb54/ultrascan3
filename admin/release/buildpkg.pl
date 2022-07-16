#!/usr/bin/perl

my %supported =
    (
     "ubuntu:18.04"  => "python"
     ,"ubuntu:20.04" => "python"
     ,"ubuntu:22.04" => "python3"
     ,"redhat:8.6"   => "python3"
    );

$snames = join( "\n", keys %supported );

$notes = "usage: $0 name

where name one of

$snames

what it does:

builds a container using podman if available of if not tries docker
the building of the container install qt from source etc, until ultrascan is built & packaged
then, the package is extracted from the container.

"
    ;

$image = shift || die $notes;

# find a container manager

$docker = `which podman`;
$docker = `which docker` if !$docker;
chomp $docker;

die "no podman or docker found installed\n" if !$docker;
die "$image is not currently supported\n" if !exists $supported{$image};
     
$from = $image;
$from =~ s/:.*$//;

$name = "us3-$image";
$name =~ s/:/-/;
$bimage = $image;
$bimage =~ s/^redhat/rockylinux/;

$cmd =
    "cd $from"
    . " && unbuffer $docker build -t $name --build-arg image=$bimage --build-arg apt_python_version=$supported{$image} ."
    . " && id=\$($docker create $name)"
    . " && $docker cp \$id:lastpkgname ../last.$name"
    . " && $docker cp \$id:`cat ../last.$name` .."
    . " && $docker rm -v \$id"
    . " && echo package `cat ../last.$name` created"
    ;

$|=1;

print "$cmd\n";
print `$cmd\n`;


