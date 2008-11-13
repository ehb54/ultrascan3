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
$author = $_POST['author'];
$keyword = $_POST['keyword'];
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
$author = stripslashes($author);
$keyword = stripslashes($keyword);
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


// Query the Database.
if ($author)
{
	$a1 = str_replace('oe', 'ö', $author);
	$a2 = str_replace('ae', 'ä', $author);
	$a3 = str_replace('ue', 'ü', $author);
	$a4 = str_replace('oe', 'Ö', $author);
	$a5 = str_replace('ae', 'Ä', $author);
	$a6 = str_replace('ue', 'Ü', $author);
	$a7 = str_replace('o', 'ö', $author);
	$a8 = str_replace('a', 'ä', $author);
	$a9 = str_replace('u', 'ü', $author);
	$a10 = str_replace('o', 'Ö', $author);
	$a11 = str_replace('a', 'Ä', $author);
	$a12 = str_replace('u', 'Ü', $author);
	$query="SELECT * FROM us_reference WHERE authors LIKE '%$author%' 
															or authors LIKE '%$a1%'
															or authors LIKE '%$a2%'
															or authors LIKE '%$a3%'
															or authors LIKE '%$a4%'
															or authors LIKE '%$a5%'
															or authors LIKE '%$a6%'
															or authors LIKE '%$a7%'
															or authors LIKE '%$a8%'
															or authors LIKE '%$a9%'
															or authors LIKE '%$a10%'
															or authors LIKE '%$a11%'
															or authors LIKE '%$a12%'";
	$sql = mysql_query($query) or die (mysql_error());
	print "<table border=1 cellspacing=3 cellpadding=3>\n";
   print "<tr><td colspan=2  bgcolor=#CFCFCF>References by Author:</td></tr>\n";
   while ($line = mysql_fetch_array($sql, MYSQL_ASSOC))
   {
      print "\t<tr>\n";
		print "\t\t<td bgcolor=#CFCFCF>" . $line["RefID"]  . ":</td><td bgcolor=#FFE77C>" .  $line["authors"] . "<br>\n";
		print "<a href=\"" . $line["weblink"] . "\">";
		print $line["title"] . "<br></a>" . " (" . $line["year"] . ") ";
		print $line["journal"] . " <b>" . $line["volume"] . "</b>:" . $line["pages"] . "</a></td></tr>\n";
	}
	print "</table><p>";
}

if ($keyword)
{
	$k1 = str_replace('oe', 'ö', $keyword);
	$k2 = str_replace('ae', 'ä', $keyword);
	$k3 = str_replace('ue', 'ü', $keyword);
	$k4 = str_replace('oe', 'Ö', $keyword);
	$k5 = str_replace('ae', 'Ä', $keyword);
	$k6 = str_replace('ue', 'Ü', $keyword);
	$query="SELECT * FROM us_reference WHERE title LIKE '%$keyword%'
															or title LIKE '%$k1%'
															or title LIKE '%$k2%'
															or title LIKE '%$k3%'
															or title LIKE '%$k4%'
															or title LIKE '%$k5%'
															or title LIKE '%$k6%'";
	$sql = mysql_query($query) or die (mysql_error());
	print "<table border=1 cellspacing=3 cellpadding=3>\n";
   print "<tr><td colspan=2  bgcolor=#CFCFCF>References by Keyword:</td></tr>\n";
   while ($line = mysql_fetch_array($sql, MYSQL_ASSOC))
   {
      print "\t<tr>\n";
		print "\t\t<td bgcolor=#CFCFCF>" . $line["RefID"]  . ":</td><td bgcolor=#FFE77C>" .  $line["authors"] . "<br>\n";
		print "<a href=\"" . $line["weblink"] . "\">";
		print $line["title"] . "<br></a>" . " (" . $line["year"] . ") ";
		print $line["journal"] . " <b>" . $line["volume"] . "</b>:" . $line["pages"] . "</a></td></tr>\n";
	}
	print "</table><p>";
}

if ($protein == 1)
{
	$query="SELECT * FROM us_reference WHERE protein = '$protein' ORDER BY year";
	$sql = mysql_query($query) or die (mysql_error());
	print "<table border=1 cellspacing=3 cellpadding=3>\n";
   print "<tr><td colspan=2  bgcolor=#CFCFCF>Protein References:</td></tr>\n";
   while ($line = mysql_fetch_array($sql, MYSQL_ASSOC))
   {
      print "\t<tr>\n";
		print "\t\t<td bgcolor=#CFCFCF>" . $line["RefID"]  . ":</td><td bgcolor=#FFE77C>" .  $line["authors"] . "<br>\n";
		print "<a href=\"" . $line["weblink"] . "\">";
		print $line["title"] . "<br></a>" . " (" . $line["year"] . ") ";
		print $line["journal"] . " <b>" . $line["volume"] . "</b>:" . $line["pages"] . "</a></td></tr>\n";
	}
	print "</table><p>";
}

if ($DNA == 1)
{
	$query="SELECT * FROM us_reference WHERE DNA = '$DNA' ORDER BY year";
	$sql = mysql_query($query) or die (mysql_error());
	print "<table border=1 cellspacing=3 cellpadding=3>\n";
   print "<tr><td colspan=2  bgcolor=#CFCFCF>DNA References:</td></tr>\n";
   while ($line = mysql_fetch_array($sql, MYSQL_ASSOC))
   {
      print "\t<tr>\n";
		print "\t\t<td bgcolor=#CFCFCF>" . $line["RefID"]  . ":</td><td bgcolor=#FFE77C>" .  $line["authors"] . "<br>\n";
		print "<a href=\"" . $line["weblink"] . "\">";
		print $line["title"] . "<br></a>" . " (" . $line["year"] . ") ";
		print $line["journal"] . " <b>" . $line["volume"] . "</b>:" . $line["pages"] . "</a></td></tr>\n";
	}
	print "</table><p>";
}

if ($synthetic == 1)
{
	$query="SELECT * FROM us_reference WHERE synthetic = '$synthetic' ORDER BY year";
	$sql = mysql_query($query) or die (mysql_error());
	print "<table border=1 cellspacing=3 cellpadding=3>\n";
   print "<tr><td colspan=2  bgcolor=#CFCFCF>Synthetic Polymer References:</td></tr>\n";
   while ($line = mysql_fetch_array($sql, MYSQL_ASSOC))
   {
      print "\t<tr>\n";
		print "\t\t<td bgcolor=#CFCFCF>" . $line["RefID"]  . ":</td><td bgcolor=#FFE77C>" .  $line["authors"] . "<br>\n";
		print "<a href=\"" . $line["weblink"] . "\">";
		print $line["title"] . "<br></a>" . " (" . $line["year"] . ") ";
		print $line["journal"] . " <b>" . $line["volume"] . "</b>:" . $line["pages"] . "</a></td></tr>\n";
	}
	print "</table><p>";
}

if ($glyco == 1)
{
	$query="SELECT * FROM us_reference WHERE glyco = '$glyco' ORDER BY year";
	$sql = mysql_query($query) or die (mysql_error());
	print "<table border=1 cellspacing=3 cellpadding=3>\n";
   print "<tr><td colspan=2  bgcolor=#CFCFCF>Glycoprotein References:</td></tr>\n";
   while ($line = mysql_fetch_array($sql, MYSQL_ASSOC))
   {
      print "\t<tr>\n";
		print "\t\t<td bgcolor=#CFCFCF>" . $line["RefID"]  . ":</td><td bgcolor=#FFE77C>" .  $line["authors"] . "<br>\n";
		print "<a href=\"" . $line["weblink"] . "\">";
		print $line["title"] . "<br></a>" . " (" . $line["year"] . ") ";
		print $line["journal"] . " <b>" . $line["volume"] . "</b>:" . $line["pages"] . "</a></td></tr>\n";
	}
	print "</table><p>";
}

if ($membrane == 1)
{
	$query="SELECT * FROM us_reference WHERE membrane = '$membrane' ORDER BY year";
	$sql = mysql_query($query) or die (mysql_error());
	print "<table border=1 cellspacing=3 cellpadding=3>\n";
   print "<tr><td colspan=2  bgcolor=#CFCFCF>Membrane Protein References:</td></tr>\n";
   while ($line = mysql_fetch_array($sql, MYSQL_ASSOC))
   {
      print "\t<tr>\n";
		print "\t\t<td bgcolor=#CFCFCF>" . $line["RefID"]  . ":</td><td bgcolor=#FFE77C>" .  $line["authors"] . "<br>\n";
		print "<a href=\"" . $line["weblink"] . "\">";
		print $line["title"] . "<br></a>" . " (" . $line["year"] . ") ";
		print $line["journal"] . " <b>" . $line["volume"] . "</b>:" . $line["pages"] . "</a></td></tr>\n";
	}
	print "</table><p>";
}

if ($assemblies == 1)
{
	$query="SELECT * FROM us_reference WHERE assemblies = '$assemblies' ORDER BY year";
	$sql = mysql_query($query) or die (mysql_error());
	print "<table border=1 cellspacing=3 cellpadding=3>\n";
   print "<tr><td colspan=2  bgcolor=#CFCFCF>Assembly References:</td></tr>\n";
   while ($line = mysql_fetch_array($sql, MYSQL_ASSOC))
   {
      print "\t<tr>\n";
		print "\t\t<td bgcolor=#CFCFCF>" . $line["RefID"]  . ":</td><td bgcolor=#FFE77C>" .  $line["authors"] . "<br>\n";
		print "<a href=\"" . $line["weblink"] . "\">";
		print $line["title"] . "<br></a>" . " (" . $line["year"] . ") ";
		print $line["journal"] . " <b>" . $line["volume"] . "</b>:" . $line["pages"] . "</a></td></tr>\n";
	}
	print "</table><p>";
}

if ($vhw == 1)
{
	$query="SELECT * FROM us_reference WHERE vhw = '$vhw' ORDER BY year";
	$sql = mysql_query($query) or die (mysql_error());
	print "<table border=1 cellspacing=3 cellpadding=3>\n";
   print "<tr><td colspan=2  bgcolor=#CFCFCF>van Holde - Weischet References:</td></tr>\n";
   while ($line = mysql_fetch_array($sql, MYSQL_ASSOC))
   {
      print "\t<tr>\n";
		print "\t\t<td bgcolor=#CFCFCF>" . $line["RefID"]  . ":</td><td bgcolor=#FFE77C>" .  $line["authors"] . "<br>\n";
		print "<a href=\"" . $line["weblink"] . "\">";
		print $line["title"] . "<br></a>" . " (" . $line["year"] . ") ";
		print $line["journal"] . " <b>" . $line["volume"] . "</b>:" . $line["pages"] . "</a></td></tr>\n";
	}
	print "</table><p>";
}

if ($finite == 1)
{
	$query="SELECT * FROM us_reference WHERE finite = '$finite' ORDER BY year";
	$sql = mysql_query($query) or die (mysql_error());
	print "<table border=1 cellspacing=3 cellpadding=3>\n";
   print "<tr><td colspan=2  bgcolor=#CFCFCF>Finite Element References:</td></tr>\n";
   while ($line = mysql_fetch_array($sql, MYSQL_ASSOC))
   {
      print "\t<tr>\n";
		print "\t\t<td bgcolor=#CFCFCF>" . $line["RefID"]  . ":</td><td bgcolor=#FFE77C>" .  $line["authors"] . "<br>\n";
		print "<a href=\"" . $line["weblink"] . "\">";
		print $line["title"] . "<br></a>" . " (" . $line["year"] . ") ";
		print $line["journal"] . " <b>" . $line["volume"] . "</b>:" . $line["pages"] . "</a></td></tr>\n";
	}
	print "</table><p>";
}

if ($equil == 1)
{
	$query="SELECT * FROM us_reference WHERE equil = '$equil' ORDER BY year";
	$sql = mysql_query($query) or die (mysql_error());
	print "<table border=1 cellspacing=3 cellpadding=3>\n";
   print "<tr><td colspan=2  bgcolor=#CFCFCF>Global Equilibrium References:</td></tr>\n";
   while ($line = mysql_fetch_array($sql, MYSQL_ASSOC))
   {
      print "\t<tr>\n";
		print "\t\t<td bgcolor=#CFCFCF>" . $line["RefID"]  . ":</td><td bgcolor=#FFE77C>" .  $line["authors"] . "<br>\n";
		print "<a href=\"" . $line["weblink"] . "\">";
		print $line["title"] . "<br></a>" . " (" . $line["year"] . ") ";
		print $line["journal"] . " <b>" . $line["volume"] . "</b>:" . $line["pages"] . "</a></td></tr>\n";
	}
	print "</table><p>";
}

if ($wavelength == 1)
{
	$query="SELECT * FROM us_reference WHERE wavelength = '$wavelength' ORDER BY year";
	$sql = mysql_query($query) or die (mysql_error());
	print "<table border=1 cellspacing=3 cellpadding=3>\n";
   print "<tr><td colspan=2  bgcolor=#CFCFCF>Extinction Spectrum Analysis References:</td></tr>\n";
	$i = 0;
   while ($line = mysql_fetch_array($sql, MYSQL_ASSOC))
   {
		$i = $i+1;
      print "\t<tr>\n";
		print "\t\t<td bgcolor=#CFCFCF>" . $line["RefID"]  . ":</td><td bgcolor=#FFE77C>" .  $line["authors"] . "<br>\n";
		print "<a href=\"" . $line["weblink"] . "\">";
		print $line["title"] . "<br></a>" . " (" . $line["year"] . ") ";
		print $line["journal"] . " <b>" . $line["volume"] . "</b>:" . $line["pages"] . "</a></td></tr>\n";
	}
	print "</table><p>";
}

echo "<a href=http://www.ultrascan.uthscsa.edu/search-refs.html>
Perform another search</a><p>";
echo "<a href=http://www.ultrascan.uthscsa.edu/>
Return to the UltraScan Home Page</a>";
?>
