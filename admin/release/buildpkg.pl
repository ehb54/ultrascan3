#!/usr/bin/perl

## n.b. when adding a new image, it's best to pull the base image, esp. if multiples might be selected
### otherwise, buildpkg might appear to hang waiting for a selection which never is displayed
### e.g.
#### STEP 1/70: FROM rockylinux:8.7
#### ? Please select an image: 
####   ▸ container-registry.oracle.com/rockylinux:8.7
####     docker.io/library/rockylinux:8.7
####
### pulling from docker.io seems to be a working choice
##

my %supported =
    (
     "ubuntu:18.04"  => "python"
     ,"ubuntu:20.04" => "python"
     ,"ubuntu:22.04" => "python3"
     ,"redhat:8.7"   => "python3"
    );

$snames = join( "\n", keys %supported );

$notes = "usage: $0 cores name

where cores is the number of cores to use for parallel compilation

where name one of

$snames

what it does:

builds a container using podman if available of if not tries docker
the building of the container install qt from source etc, until ultrascan is built & packaged
then, the package is extracted from the container.

"
    ;

$cores = shift || die $notes;
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
    . " && unbuffer $docker build -t $name --build-arg image=$bimage --build-arg apt_python_version=$supported{$image} --build-arg parallel_compile=$cores ."
    . " && id=\$($docker create $name)"
    . " && $docker cp \$id:lastpkgname ../last.$name"
    . " && $docker cp \$id:`cat ../last.$name` .."
    . " && $docker rm -v \$id"
    . " && echo package `cat ../last.$name` created"
    ;

$|=1;

print "-"x80 . "\n";
print "building release package for $bimage using $cores cores\n";
print "-"x80 . "\n";
print "$cmd\n";
print "-"x80 . "\n";
print `$cmd\n`;
print "-"x80 . "\n";


