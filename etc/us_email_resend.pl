#!/usr/bin/perl

# user editable section

$wwwbase = "/var/www/html/ultrascan";
# $execute++;

# end user editable

$us = $ENV{'ULTRASCAN'} || die "$0: The environment variable ULTRASCAN must be set.  Terminating\n";

$file = shift || die "usage: $0 filename
where filename has been touched to the start date/time
";

$pwd = `pwd`;
chomp $pwd;
$file = "$pwd/$file" if !($file =~ /^\//);

die "$0 ERROR: file $file $!\n" if !-e $file;

chdir $wwwbase || die "$0 ERROR: can't change to $wwwbase. $!\n";

@files = `find . -name "email_msg" -newer $file`;
grep chomp, @files;
@files = grep s/\/email_msg//, @files;
@files = grep s/^\./$wwwbase/, @files;

for ($i = 0; $i < @files; $i++) {
    $cmd = "cd $files[$i]; head -1 email_msg\n";
    print $cmd;
    print `$cmd`;
    $cmd = "cd $files[$i]; perl $us/bin64/us_email.pl email_list* email_msg\n";
    print $cmd;
    print `$cmd` if $execute;
    
}

