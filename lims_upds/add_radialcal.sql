-- -----------------------------------------------------
-- Table radialCalibration
-- -----------------------------------------------------
DROP TABLE IF EXISTS radialCalibration ;

CREATE  TABLE IF NOT EXISTS radialCalibration (
  radialCalID int(11) NOT NULL AUTO_INCREMENT ,
  radialCalGUID char(36) NOT NULL UNIQUE ,
  speed int(11) NOT NULL DEFAULT 0 ,
  rotorCalID int(11) NOT NULL DEFAULT 0 ,
  dateUpdated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ,
  PRIMARY KEY (radialCalID) )
ENGINE = InnoDB;


