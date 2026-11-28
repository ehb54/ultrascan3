CREATE USER 'fixture'@'%' IDENTIFIED BY '';
GRANT ALL ON profile1329.* TO 'fixture'@'%';
-- Load only into a disposable database dedicated to #1329 fixtures.
CREATE TABLE fixture_profile (id INT PRIMARY KEY, xml LONGTEXT);
DELIMITER //
CREATE PROCEDURE validate_user(g TEXT,e TEXT,p TEXT)
BEGIN SELECT 0; SELECT 42,'fixture@example.invalid'; END//
CREATE PROCEDURE get_aprofile_info(g TEXT,p TEXT,a TEXT)
BEGIN SELECT 0; SELECT 29,'Stored profile',xml,'unused','{"fixture":"mask"}','{"fixture":"plots"}' FROM fixture_profile WHERE id=1; END//
CREATE PROCEDURE get_report_info_by_id(g TEXT,p TEXT,i TEXT)
BEGIN SELECT 0; SELECT '1A:UV/vis.:Sample',1.25,0.004,15000,9000,IF(i=17,280,MOD(i,1000)),12,15,
'{"tot_conc":"0","rmsd_lim":"1","intensity":"0","exp_dur":"1","integration":"0","plots":"1"}'; END//
CREATE PROCEDURE get_report_items_ids_by_report_id(g TEXT,p TEXT,i TEXT)
BEGIN IF i>1000 THEN SELECT 4; ELSE SELECT 0; SELECT 41; END IF; END//
CREATE PROCEDURE get_report_item_info_by_id(g TEXT,p TEXT,i TEXT)
BEGIN SELECT 0; SELECT 's','2DSA-MC',2.5,4.5,0.8,5,80,1,0; END//
DELIMITER ;
