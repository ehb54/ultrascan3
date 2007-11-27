<?php

/*  Database Information - Required!!  */
/* -- Configure the Variables Below --*/
$dbhost = 'localhost';
$dbusername = 'ultrascan';
$dbpasswd = '241deal';
$database_name = 'US_References';

/* Database Stuff, do not modify below this line */

$connection = mysql_pconnect("$dbhost","$dbusername","$dbpasswd") 
	or die ("Couldn't connect to server.");
	
$db = mysql_select_db("$database_name", $connection)
	or die("Couldn't select database.");

// Define post fields into simple variables
$authors = $_POST['authors'];
$title = $_POST['title'];
$year = $_POST['year'];
$journal = $_POST['journal'];
$volume = $_POST['volume'];
$pages = $_POST['pages'];
$weblink = $_POST['weblink'];
$protein = $_POST['protein'];
$DNA = $_POST['DNA'];
$synthetic = $_POST['synthetic'];
$glyco = $_POST['glyco'];
$membrane = $_POST['membrane'];
$assemblies = $_POST['assemblies'];
$finite = $_POST['finite'];
$vhw = $_POST['vhw'];
$equil = $_POST['equil'];
$wavelength = $_POST['wavelength'];

/* Let's strip some slashes in case the user entered
any escaped characters. */
$authors = stripslashes($authors);
$title = stripslashes($title);
$year = stripslashes($year);
$journal = stripslashes($journal);
$volume = stripslashes($volume);
$pages = stripslashes($pages);
$weblink = stripslashes($weblink);

if (!$protein) {$protein = "0";}
if (!$DNA) {$DNA = "0";}
if (!$synthetic) {$synthetic = "0";}
if (!$glyco) {$glyco = "0";}
if (!$membrane) {$membrane = "0";}
if (!$assemblies) {$assemblies = "0";}
if (!$finite) {$finite = "0";}
if (!$vhw) {$vhw = "0";}
if (!$equil) {$equil = "0";}
if (!$wavelength) {$wavelength = "0";}


/* Do some error checking on the form posted fields */

if((!$authors) || 
   (!$title) ||
   (!$year) ||
   (!$journal) ||
   (!$pages) ||
   (!$volume))
{
	echo '<font color=red><b>Error: The following information is required, please enter it below:</b></font> <p>';
	if(!$authors)
	{
		echo "Please provide the names of all authors...<br>";
	}
	if(!$title)
	{
		echo "Please provide a title for your reference...<br>";
	}
	if(!$year)
	{
		echo "Please enter the year of your publication...<br>";
	}
	if(!$journal)
	{
		echo "Please provide the name of the journal...<br>";
	}
	if(!$pages)
	{
		echo "Please provide the page numbers for your article...<br>";
	}
	if(!$volume)
	{
		echo "Please provide the journal volume number...<br>";
	}
	include 'ultrascan-refs.html'; // Show the form again!
	exit(); // if the error checking has failed, we'll exit the script!
}
	
// Enter info into the Database.
$query="INSERT INTO us_reference (authors, title, year, journal, volume, pages, weblink, protein, DNA, synthetic, glyco, membrane, assemblies, finite, vhw, equil, wavelength) VALUES ('$authors', '$title', '$year', '$journal', '$volume', '$pages', '$weblink', '$protein', '$DNA', '$synthetic', '$glyco', '$membrane', '$assemblies', '$finite', '$vhw', '$equil', '$wavelength')";
$sql = mysql_query($query) or die (mysql_error());

echo "Thank you for your help!<p>Your reference has been included in the
database!<p> If you would like to search the database, please visit the 
UltraScan Reference Database Search Form:<br>
<a href=http://www.ultrascan.uthscsa.edu/search-refs.html>
http://www.ultrascan.uthscsa.edu/search-refs.html</a><p>";

	$subject = "UltraScan Reference Submitted";
	$message = "The following reference has been submitted:";

$reply_mail = "demeler@biochem.uthscsa.edu";
	
$message = $message . "\n" . $authors . "\n" . $title . "\n" . $year . "\n" . $journal . "\n" . $volume . ":" . $pages . "\n" . $weblink;

mail($reply_mail, $subject, $message, "From: UltraScan Reference Submission <demeler@biochem.uthscsa.edu>\nX-Mailer: PHP/" . phpversion());

echo "<a href=http://www.ultrascan.uthscsa.edu/ultrascan-refs.html>
Enter another reference</a><p>";
echo "<a href=http://www.ultrascan.uthscsa.edu/>
Return to the UltraScan Home Page</a>";
?>
