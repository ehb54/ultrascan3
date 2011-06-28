--
-- create_us2_grants.sql
--
-- This script will create all the grants on the new ultrascan3 implementation
-- of lims2 databases
--

--
-- Script created as follows:
-- mysql> use lims2_config;
-- mysql> select CONCAT( 'GRANT SELECT, INSERT, UPDATE, DELETE ON ', 
-- database_name, 
-- '.* TO ', 
-- dbusername, 
-- '@\'%\' IDENTIFIED BY \'', 
-- dbpasswd, 
-- '\';' ) as query 
-- from config;
--

GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_cauma.* TO cauma@'%' 
  IDENTIFIED BY 'Aik4keu8';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_nwu.* TO nwu@'%' 
  IDENTIFIED BY 'ki5wi2Fo';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_duesseldorf.* TO duesseldorf@'%' 
  IDENTIFIED BY 'dae3aePe';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_rice.* TO rice@'%' 
  IDENTIFIED BY 'que4baiW';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_marshall.* TO marshall@'%' 
  IDENTIFIED BY 'nakaiV2d';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_osaka.* TO osaka@'%' 
  IDENTIFIED BY 'aw0yeiJu';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_siuc.* TO siuc@'%' 
  IDENTIFIED BY 'Ohz1Cuti';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_umc.* TO umc@'%' 
  IDENTIFIED BY 'Ohy8Eixe';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_tamu_hsc.* TO tamu_hsc@'%' 
  IDENTIFIED BY 'Hiexu1Be';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_mpig.* TO mpig@'%' 
  IDENTIFIED BY 'boRe9bai';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_epfl.* TO epfl@'%' 
  IDENTIFIED BY 'ook8UuZ9';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_fmp.* TO fmp@'%' 
  IDENTIFIED BY 'Deizeel2';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_utoronto.* TO utoronto@'%' 
  IDENTIFIED BY 'Angoojo2';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_utrecht.* TO utrecht@'%' 
  IDENTIFIED BY 'aamiSh6a';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_fsu.* TO fsu@'%' 
  IDENTIFIED BY 'gahp3Lee';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_unimelb.* TO unimelb@'%' 
  IDENTIFIED BY 'Ood5Mahk';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_umassmed.* TO umassmed@'%' 
  IDENTIFIED BY 'beeTeih0';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_charlesu.* TO charlesu@'%' 
  IDENTIFIED BY 'Aephei0y';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_uindiana.* TO uindiana@'%' 
  IDENTIFIED BY 'oeba6Ahu';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_tum.* TO tum@'%' 
  IDENTIFIED BY 'Zoon8toh';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_colostate.* TO colostate@'%' 
  IDENTIFIED BY 'si1Oosh0';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_mcgill.* TO mcgill@'%' 
  IDENTIFIED BY 'kai2doZ0';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_burnet.* TO burnet@'%' 
  IDENTIFIED BY 'Gee5quij';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_umich.* TO umich@'%' 
  IDENTIFIED BY 'Eboo3ith';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_ncmh.* TO ncmh@'%' 
  IDENTIFIED BY 'eeH5eovu';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_igbmc.* TO igbmc@'%' 
  IDENTIFIED BY 'hihool8Y';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_sinica.* TO sinica@'%' 
  IDENTIFIED BY 'fei9ke9A';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_bakh.* TO bakh@'%' 
  IDENTIFIED BY 'uuLah0ae';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_um.* TO um@'%' 
  IDENTIFIED BY 'iugh2Zee';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_mdacc.* TO mdacc@'%' 
  IDENTIFIED BY 'Eiz7eiHi';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_uic.* TO uic@'%' 
  IDENTIFIED BY 'ri4Ohgh4';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_suny.* TO suny@'%' 
  IDENTIFIED BY 'Om1EeGoh';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_sbu.* TO sbu@'%' 
  IDENTIFIED BY 'aeghaeG3';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_nhlbi.* TO nhlbi@'%' 
  IDENTIFIED BY 'thei2Luv';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_uwashington.* TO uwashington@'%' 
  IDENTIFIED BY 'UShohr8L';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_uwklevit.* TO uwklevit@'%' 
  IDENTIFIED BY 'Bae6aeng';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_uwcatalano.* TO uwcatalano@'%' 
  IDENTIFIED BY 'Kah1Lahf';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_uky.* TO uky@'%' 
  IDENTIFIED BY 'ieth5eeZ';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_uglasgow.* TO uglasgow@'%' 
  IDENTIFIED BY 'JiT6Yaig';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_ibs.* TO ibs@'%' 
  IDENTIFIED BY 'na5Foegh';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_mit.* TO mit@'%' 
  IDENTIFIED BY 'AegaeSh4';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_gilead.* TO gilead@'%' 
  IDENTIFIED BY 'choh2ieL';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_anglia.* TO anglia@'%' 
  IDENTIFIED BY 'Ushahwi5';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_regensburg.* TO regensburg@'%' 
  IDENTIFIED BY 'va6IeKae';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_lund.* TO lund@'%' 
  IDENTIFIED BY 'Le0pie4i';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_baxter_at.* TO baxter_at@'%' 
  IDENTIFIED BY 'Doe5Quai';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_nrel.* TO nrel@'%' 
  IDENTIFIED BY 'eiyai4Ae';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_sinai.* TO sinai@'%' 
  IDENTIFIED BY 'Ieghoh5z';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_bristol.* TO bristol@'%' 
  IDENTIFIED BY 'Aevah4Ph';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_osu.* TO osu@'%' 
  IDENTIFIED BY 'Vai3thee';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_shriners.* TO shriners@'%' 
  IDENTIFIED BY 'Reedeex1';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_uvic.* TO uvic@'%' 
  IDENTIFIED BY 'Oequie8i';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_mainz.* TO mainz@'%' 
  IDENTIFIED BY 'ia7Ieyoh';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_pasteur.* TO pasteur@'%' 
  IDENTIFIED BY 'seeWiew6';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_embl.* TO embl@'%' 
  IDENTIFIED BY 'Koot8vei';
GRANT SELECT, INSERT, UPDATE, DELETE ON uslims2_utaustin.* TO utaustin@'%' 
  IDENTIFIED BY 'Ees2cipe';
