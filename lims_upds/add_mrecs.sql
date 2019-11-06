-- -----------------------------------------------------
-- Table pcsa_modelrecs
-- -----------------------------------------------------
DROP TABLE IF EXISTS pcsa_modelrecs ;

CREATE  TABLE IF NOT EXISTS pcsa_modelrecs (
  mrecsID int(11) NOT NULL AUTO_INCREMENT ,
  editedDataID int(11) NOT NULL DEFAULT 1,
  modelID int(11) NOT NULL DEFAULT 0,
  mrecsGUID CHAR(36) NOT NULL UNIQUE ,
  description VARCHAR(160) NULL DEFAULT NULL, -- includes 80 for runID and 80 for other
  xml LONGTEXT NULL DEFAULT NULL ,
  lastUpdated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ,
  PRIMARY KEY (mrecsID) ,
  INDEX ndx_mrecs_editedDataID (editedDataID ASC) ,
  CONSTRAINT fk_mrecs_editDataID
    FOREIGN KEY (editedDataID )
    REFERENCES editedData (editedDataID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


