<?php 
print "<html><title>UltraScan Download Form</title>\n<body bgcolor=white>";

print "<p><table border=0 cellpadding=0>
		<tr>
			<td><b>Hardware:</b></td>
			<td>&nbsp;&nbsp;&nbsp;" . $_POST['Hardware'] .  "</td>
		</tr>
		<tr>
			<td><b>Operating System:</b></td>
			<td>&nbsp;&nbsp;&nbsp;" . $_POST['OS'] . "</td>
   	</tr>
		<tr>
			<td><b>UltraScan Version:</b></td>
			<td>&nbsp;&nbsp;&nbsp;" . $_POST['US_VERSION'] .  "</td>
		</tr>
		<tr>
			<td><b>QT Version:</b></td>
			<td>&nbsp;&nbsp;&nbsp;" . $_POST['QT_VERSION'] .  "</td>
		</tr></table><p>";

	if ($OS == "win32")
	{
		$FILE = $_POST['US_VERSION'] . "-qt-" . $QT_VERSION . "-" . $OS . ".exe";
	}
	else
	{
		$FILE = $_POST['US_VERSION'] . "-qt-" .
  	  		     $_POST['QT_VERSION'] . "-" . $Hardware . "-" . $OS . ".tar.gz";
	}
	$DOWNLOAD = "http://www.ultrascan.uthscsa.edu/software/ultrascan-" . $FILE;
	print "<a href=" . $DOWNLOAD . ">ultrascan-" . $FILE . "</a>";
	if ($OS == "win32")
	{
		print "<p><a href=ftp://ftp.biochem.uthscsa.edu/pub/windows/C++_Compilers/Borland_C++_5.5/cc3250mt.dll>cc3250mt.dll</a> (needed on some Installations - copy into C:\\Program Files\\UltraScan\\bin\\cc3250mt.dll)"; 
		print "<p><a href=software/install_win32.html>Installation Instructions</a> 
<font color=red>(Contains important note for NON-US Windows Installations!)</font>";
	}
	else
	{
		print "<p><a href=software/qt-" . $QT_VERSION . "-" . $Hardware . "-" 
		. $OS . ".tar.gz>qt-" . $QT_VERSION . "-" . $Hardware . "-". $OS 
		. ".tar.gz</a> (QT version for this distribution if not preinstalled 
		on host computer)";
		print "<p><a href=software/install_tar_gz.html>Installation Instructions</a>";
	}
	print "<p>";

print "<hr>
<p>
<b>PLEASE NOTE:</b>
<p align=justify>
<font color=red>
In order to use <b><i>UltraScan</b></i> a license key is required 
which can be obtained through online registration only.
Please take a moment and register your <b><i>UltraScan</i></b> copy at 
<a href=http://www.ultrascan.uthscsa.edu/register.html>http://www.ultrascan.uthscsa.edu/register.html</a>. 
A license key will be e-mailed to you within 1 business day.
</font>

<p>
<a href=register.html>License Application</a>";
print "<hr>\n
<a href=download.html>Return to download page</a>
</body>\n</html>";
?>
