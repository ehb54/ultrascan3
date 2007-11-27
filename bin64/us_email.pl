#!/usr/bin/perl

use MIME::Lite;

$file = shift;
open(FILE, $file);

$textfile = shift;
open(TEXTFILE, $textfile);

$email = <TEXTFILE>;
chomp $email;

$subject = <TEXTFILE>;
chomp $subject;

while(<TEXTFILE>) {
    $textmsg .= $_;
}
close TEXTFILE;

$msg = MIME::Lite->new(From    => 'gridcontrol@ultrascan.uthscsa.edu',
		       To      =>  $email,
		       Subject =>  $subject,
		       Type    => 'multipart/mixed');

$msg->attach(Type     => 'TEXT',
	     Data     => $textmsg);

while($thisfile = <FILE>) {
    chomp $thisfile;
    $msg->attach(Type     => 'BINARY',
		 Path     => $thisfile, 
		 Filename => $thisfile);
}
close FILE;
$msg->send('smtp', 'biochem.uthscsa.edu');
#$msg->send();

# cleanup
__END__
open(FILE, $file);
while($thisfile = <FILE>) {
    chomp $thisfile;
    unlink $thisfile;
}
#unlink $textfile;
#unlink $file;
