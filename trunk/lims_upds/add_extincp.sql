-- -----------------------------------------------------
-- Table extinctionProfile
-- -----------------------------------------------------
DROP TABLE IF EXISTS extinctionProfile;

CREATE TABLE extinctionProfile (
  profileID int(11) NOT NULL AUTO_INCREMENT ,
  componentID int(11) NOT NULL ,
  componentType  enum( 'Buffer', 'Analyte', 'Sample' ) NOT NULL,
  valueType  enum( 'absorbance', 'molarExtinction', 'massExtinction' ) NOT NULL,
  xml longtext DEFAULT NULL ,
  PRIMARY KEY (profileID) ,
  INDEX ndx_component_ID (componentID ASC) )
ENGINE = InnoDB;

