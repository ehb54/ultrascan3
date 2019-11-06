--
-- Alter analysis profile and related tables;
--

-- -----------------------------------------------------
-- Table protocol (add column)
-- -----------------------------------------------------
ALTER TABLE protocol
  ADD COLUMN aprofileGUID char(36) NULL
  AFTER wavelengths;

-- -----------------------------------------------------
-- -- Table experiment (replace column)
-- Table experiment (add column)
-- -----------------------------------------------------
-- ALTER TABLE experiment
--   DROP COLUMN centrifugeProtocol;

ALTER TABLE experiment
  ADD COLUMN protocolGUID char(36) NULL
  AFTER RIProfile;

-- -----------------------------------------------------
-- Table analysisprofile (new)
-- -----------------------------------------------------
DROP TABLE IF EXISTS analysisprofile ;

CREATE TABLE IF NOT EXISTS analysisprofile (
  aprofileID int(11) NOT NULL AUTO_INCREMENT ,
  aprofileGUID char(36) NOT NULL UNIQUE ,
  name TEXT NOT NULL ,
  xml LONGTEXT NOT NULL ,
  dateUpdated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ,
  PRIMARY KEY (aprofileID) )
ENGINE = InnoDB;

-- -----------------------------------------------------
-- Table protocolAprofile (new link table)
-- -----------------------------------------------------
DROP TABLE IF EXISTS protocolAprofile ;

CREATE  TABLE IF NOT EXISTS protocolAprofile (
  protocolID int(11) NOT NULL ,
  aprofileID int(11) NOT NULL ,
  INDEX ndx_protocolAprofile_aprofileID (aprofileID ASC) ,
  INDEX ndx_protocolAprofile_protocolID (protocolID ASC) ,
  CONSTRAINT fk_protocolAprofile_aprofileID
    FOREIGN KEY (aprofileID )
    REFERENCES analysisprofile (aprofileID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_protocolAprofile_protocolID
    FOREIGN KEY (protocolID )
    REFERENCES protocol (protocolID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;

-- -----------------------------------------------------
-- Table experimentProtocol (new link table)
-- -----------------------------------------------------
DROP TABLE IF EXISTS experimentProtocol ;

CREATE  TABLE IF NOT EXISTS experimentProtocol (
  experimentID int(11) NOT NULL ,
  protocolID int(11) NOT NULL ,
  INDEX ndx_experimentProtocol_protocolID (protocolID ASC) ,
  INDEX ndx_experimentProtocol_experimentID (experimentID ASC) ,
  CONSTRAINT fk_experimentProtocol_protocolID
    FOREIGN KEY (protocolID )
    REFERENCES protocol (protocolID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_experimentProtocol_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES experiment (experimentID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;

