-- -----------------------------------------------------
-- Table protocol
-- -----------------------------------------------------
DROP TABLE IF EXISTS protocol ;

CREATE  TABLE IF NOT EXISTS protocol (
  protocolID int(11) NOT NULL AUTO_INCREMENT ,
  protocolGUID char(36) NOT NULL UNIQUE ,
  description varchar(160) NOT NULL UNIQUE ,
  xml longtext NOT NULL ,
  optimaHost varchar(24) NOT NULL ,
  dateUpdated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ,
  rotorID int(11) NULL ,
  speed1 int(11) NULL ,
  duration float NULL ,
  usedcells int(11) NULL ,
  estscans int(11) NULL ,
  solution1 varchar(80) NULL ,
  solution2 varchar(80) NULL ,
  wavelengths int(11) NULL ,
  PRIMARY KEY (protocolID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table prototcolPerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS protocolPerson ;

CREATE  TABLE IF NOT EXISTS protocolPerson (
  protocolID int(11) NOT NULL ,
  personID int(11) NOT NULL ,
  INDEX ndx_protocolPerson_personID  (personID ASC) ,
  INDEX ndx_protocolPerson_protocolID (protocolID ASC) ,
  CONSTRAINT fk_protocolPerson_personID
    FOREIGN KEY (personID )
    REFERENCES people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_protocolPerson_protocolID
    FOREIGN KEY (protocolID )
    REFERENCES protocol (protocolID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;

