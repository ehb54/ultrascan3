<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
  <title>UltraScan Software Download Page</title>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <link rel='stylesheet' type='text/css' href='mainstyle1.css' media='screen'/>
</head>
<body>

<h1><span class='ultrascan'>UltraScan</span> Software Download</h1>

<h2 class='important'>PLEASE NOTE:</h2>

<p class='important'>When selecting a license key please specify version. Thank
you for using UltraScan!</p>

<hr/>


<p>In order to use <span class='ultrascan'>UltraScan</span> a license key is
required which can be obtained through online registration only.  Please take a
moment and register your <span class='ultrascan'>UltraScan</span> copy.  A license key will be
e-mailed to you upon registration. A valid e-mail address is required. All
licenses are made available for free and are applicable to academic, non-profit
and for-profit commercial use. A free license key can be obtained from <a
href='register.html'>this page</a>.</p>

<?php
function package( $url )
{
   return "<a href='software/$url'>$url</a>";
}

$linuxVersion   = "9.9-973";
$opteronVersion = "9.9-973";
$msversion      = "9.9-973";
$macVersion     = "9.9-973";
$isoVersion     = "9.9-857";

$amd     = package( "ultrascan-$opteronVersion-qt-3.3.6-opteron-linux.tar.gz" );
$win32   = package( "ultrascan-$msversion-mswin-setup.exe" );
$win64   = $win32;
$linux   = package( "ultrascan-$linuxVersion-qt-3.3.8-ix86-linux.tar.gz" );
$iso     = package( "ultrascan-$isoVersion-image.iso" );
$mac104  = package( "ultrascan-$macVersion-MacOSX-10.4-X11.dmg.zip" );
$mac105  = package( "ultrascan-$macVersion-MacOSX-10.5-X11.dmg.zip" );
$slack   = '<a href="http://ultrascan.uthscsa.edu/software/ultrascan">UltraScan Repository</a>';

echo <<<HTML

<p>The current version of <span class='ultrascan'>UltraScan</span> is $linuxVersion.</p>

<table>
<tr><th>Architecture</th>                <th>Operating System</th>             <th>Link</th>  </tr>
<tr><th rowspan='2'>AMD/Intel 64-bit</th><td>Linux (64-bit)</td>               <td>$amd</td>  </tr>
<tr>                                     <td>Windows (Vista 64-bit)</td>       <td>$win64</td></tr>
<tr><th rowspan='4'>AMD/Intel 32-bit</th><td>Linux (32 bit)</td>                        <td>$linux</td></tr>
<tr>                                     <td>Windows (NT, 2000, XP, Vista)</td><td>$win32</td></tr>
<tr>                                     <td>Linux ISO Image</td>              <td>$iso</td>  </tr>
<tr>                                     <td>Slackware Packages</td>           <td>$slack</td></tr>
<tr><th rowspan='2'>Macintosh G3/G4, <br/>
                    PPC and Intel</th>   <td>Macintosh OS-X 10.4 (Apple/X11 required)</td>
                                                                               <td>$mac104</td></tr>
<tr><td>Macintosh OS-X 10.5 (Apple/X11 required)</td>                          <td>$mac105</td></tr>
</table>

HTML;
?>
<h4>Prerequsite Software</h4>

<p>Linux requires Qt-3.3.x to be installed.  Qt is bundled with the 
Windows and OS-X versions of <span class='ultrascan'>UltraScan</span>.</p>

<h4>Other Platforms</h4>

<p>Other Architecture and Operating Systems may be available for older versions 
of <span class='ultrascan'>UltraScan</span> upon request.  Contact
<a href='mailto:demeler@biochem.uthscsa.edu'>Borries Demeler</a> for more information.</p>

<ul>
  <li>AMD/Intel
      <ul>
        <li>NetBSD</li>
        <li>FreeBSD</li>
      </ul>
    </li>
  <li>Sun Sparc
      <ul>
      <li>Solaris</li>
      </ul>
  </li>
  <li>Silicon Graphics
    <ul>
      <li>Irix</li>
    </ul>
  </li>
</ul>

<h4>Installation Help</h4>

      <p>Detailed instructions for the UltraScan installation 
	for various platforms can be found at:
      <a href="http://www.ultrascan.uthscsa.edu/install.html">
      http://www.ultrascan.uthscsa.edu/install.html</a></p>

  </li>
</ul>

<h4>Source Code</h4>

<p>Source code is available through the 
<a href='http://subversion.tigris.org/'>subversion</a> source code control system.  
To check out a current copy of the code, use the following command:</p>

<div class='command'>svn co svn://bcf.uthscsa.edu/ultrascan/trunk ultrascan</div>

<p>The current source can also be viewed at the <span class='ultrascan'>UltraScan</span>
<a href='http://wiki.bcf.uthscsa.edu/ultrascan/browser/trunk'>wiki</a>.</p>


</body>
</html>
