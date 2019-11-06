-- -----------------------------------------------------
-- Table autoflow
-- -----------------------------------------------------
DROP TABLE IF EXISTS autoflow;

CREATE  TABLE IF NOT EXISTS autoflow (
  ID int(11) NOT NULL AUTO_INCREMENT ,
  protName varchar(80) NULL,
  cellChNum int(10) NULL,
  tripleNum int(10) NULL,
  duration int(10)  NULL,
  runName varchar(300) NULL,
  expID  int(10) NULL,
  runID  int(10) NULL,
  status enum('LIVE_UPDATE','EDITING','ANALYSIS','REPORT') NOT NULL,
  dataPath varchar(300) NULL,
  optimaName varchar(300) NULL,
  runStarted TIMESTAMP NULL,
  invID  INT NULL,
  corrRadii enum('YES', 'NO') NOT NULL,
  created TIMESTAMP NULL,
  PRIMARY KEY (ID) )
ENGINE = InnoDB;


