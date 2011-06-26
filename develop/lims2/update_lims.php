<?php
include 'dbconfig.php';
   $link = mysql_connect($host, $username, $password) or die("Could not connect");
   mysql_select_db("lims2_config") or die("Could not select database");

	$query = "SELECT * FROM config";
	$result = mysql_query($query) or die("Query failed");
	$pw = array( "Ahhai5te", "Aik4keu8", "ki5wi2Fo", "dae3aePe",
	"que4baiW", "nakaiV2d", "aw0yeiJu", "Ohz1Cuti", "Ohy8Eixe",
	"Hiexu1Be", "boRe9bai", "ook8UuZ9", "Deizeel2", "Angoojo2",
	"aamiSh6a", "gahp3Lee", "Ood5Mahk", "beeTeih0", "Aephei0y",
	"oeba6Ahu", "Zoon8toh", "si1Oosh0", "kai2doZ0", "Gee5quij",
	"Eboo3ith", "eeH5eovu", "hihool8Y", "fei9ke9A", "uuLah0ae",
	"iugh2Zee", "Eiz7eiHi", "ri4Ohgh4", "Om1EeGoh", "aeghaeG3",
	"thei2Luv", "UShohr8L", "Bae6aeng", "Kah1Lahf", "ieth5eeZ",
	"JiT6Yaig", "na5Foegh", "AegaeSh4", "choh2ieL", "Ushahwi5",
	"va6IeKae", "Le0pie4i", "Doe5Quai", "eiyai4Ae", "Ieghoh5z",
	"Aevah4Ph", "Vai3thee", "Reedeex1", "Oequie8i", "ia7Ieyoh",
	"seeWiew6", "Koot8vei", "Ees2cipe", "goop8ohW", "ahNie0zo",
	"Voh5ohth", "Shai5idi", "iequao3A", "ahv5Saih", "aiChag7m",
	"rudaip9C", "kohGuo0s", "aul4Ighu", "iemuo7Ae", "eif2Eiyu",
	"Rieb3fae", "ua7ThoeN", "aiW6LieC", "Iheetae4", "ang0Oow6",
	"Uayai8oh", "Ahchiu8u", "Paefai7l", "PeiSo7ae", "lohy7Kei",
	"mahwo4Ai", "Iux7Uet8", "Mephoe8b", "Bai8ooYe", "ahR7aesa",
	"naeyei7O", "ievei5Jo", "Qui9uR4f", "kah4dieC", "us8thoCh",
	"thias9Ei", "Aw4eic1i", "faed3Tar", "ooNgia0O", "eeyei7Ma",
	"uiw2looX", "deete9Ae", "Aiphee0V", "phohRi6b", "cai3UQua",
	"eib3eeQu", "ahm5Xooh", "EeG1zohm", "yaexe6Ee", "shoh6Thi",
	"tu4xui2G", "Haer7oor", "aiCic9ki", "uthei7Sh", "Pae3shah",
	"keiG7mai", "Shi2cie6", "Iir6ahg3", "ewaeTh5P", "noshoh8E",
	"Eib2iiGi", "bai3eD6e", "aeNiep7I", "Iin5pee6", "soo8yaPu",
	"xoGhoo3o");
	$i = 0;
	while ($line = mysql_fetch_array($result, MYSQL_ASSOC))
	{
/*
		$i = $i + 1;
		$newuser=substr($line["database_name"], 8);
		$query2="UPDATE config SET dbusername='$newuser', dbpasswd='$pw[$i]' WHERE recordID='$line[recordID]' ";
		print "$query2 \n";
		$result2 = mysql_query($query2) or die('Invalid query: ' . mysql_error());	
*/

		print
		$line["recordID"] . " " . 
		$line["topImage"] . " " . 
		$line["topTitle"] . " " . 
		$line["facilityName"] . " " . 
		$line["facilityAdmin"] . " " . 
		$line["facilityAdminPhone"] . " " . 
		$line["facilityAdminEmail"] . " " . 
		$line["dbusername"] . " " . 
		$line["dbpasswd"] . " " . 
		$line["database_name"] . " " .  "\n";

	}
?>
