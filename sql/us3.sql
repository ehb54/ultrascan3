SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS;
SET @OLD_SQL_MODE=@@SQL_MODE;

SET UNIQUE_CHECKS = 0;
SET FOREIGN_KEY_CHECKS=0;
SET SQL_MODE='TRADITIONAL';

CREATE SCHEMA IF NOT EXISTS us3 DEFAULT CHARACTER SET ascii COLLATE ascii_bin ;
USE us3;

-- -----------------------------------------------------
-- Table us3.people
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.people ;

CREATE  TABLE IF NOT EXISTS us3.people (
  personID INT NOT NULL AUTO_INCREMENT ,
  GUID CHAR(36) NOT NULL UNIQUE,
  fname VARCHAR(30) NULL ,
  lname VARCHAR(30) NULL ,
  address VARCHAR(255) NULL ,
  city VARCHAR(30) NULL ,
  state CHAR(2) NULL ,
  zip VARCHAR(10) NULL ,
  phone VARCHAR(24) NULL ,
  email VARCHAR(63) NOT NULL UNIQUE ,
  organization VARCHAR(45) NULL ,
  username VARCHAR(80) NULL,
  password VARCHAR(80) NOT NULL ,
  activated BOOLEAN NOT NULL DEFAULT false ,
  signup TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ,
  lastLogin DATETIME NULL ,
  clusterAuthorizations VARCHAR(255) NOT NULL default 'bcf:alamo:laredo:lonestar:bigred:steele:queenbee',
  userlevel TINYINT NOT NULL DEFAULT 0 ,
  PRIMARY KEY (personID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.project
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.project ;

CREATE  TABLE IF NOT EXISTS us3.project (
  projectID INT NOT NULL AUTO_INCREMENT ,
  goals TEXT NULL ,
  molecules TEXT NULL ,
  purity VARCHAR(10) NULL ,
  expense TEXT NULL ,
  bufferComponents TEXT NULL ,
  saltInformation TEXT NULL ,
  AUC_questions TEXT NULL ,
  notes TEXT NULL ,
  description TEXT NULL ,
  status ENUM('submitted', 'designed', 'scheduled','uploaded', 'anlyzed', 'invoiced', 'paid', 'other') NOT NULL ,
  PRIMARY KEY (projectID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.instrument
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.instrument ;

CREATE  TABLE IF NOT EXISTS us3.instrument (
  instrumentID INT NOT NULL AUTO_INCREMENT ,
  name TEXT NULL ,
  serialNumber TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (instrumentID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.lab
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.lab ;

CREATE  TABLE IF NOT EXISTS us3.lab (
  labID INT NOT NULL AUTO_INCREMENT ,
  GUID CHAR(36) NULL ,
  name TEXT NULL ,
  building TEXT NULL ,
  room TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (labID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.abstractRotor
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.abstractRotor ;

CREATE  TABLE IF NOT EXISTS us3.abstractRotor (
  abstractRotorID INT NOT NULL AUTO_INCREMENT ,
  GUID CHAR(36) NULL ,
  name enum( 'AN50', 'AN60', 'CFA' ) NULL ,
  materialName enum( 'Titanium', 'Carbon fiber' ) NULL ,
  numHoles INT NULL ,
  maxRPM INT NULL ,
  magnetOffset FLOAT NULL ,
  cellCenter FLOAT NULL ,
  manufacturer enum( 'Beckman' ) NULL ,
  materialRefURI TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (abstractRotorID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.rotor
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.rotor ;

CREATE  TABLE IF NOT EXISTS us3.rotor (
  rotorID INT NOT NULL AUTO_INCREMENT ,
  abstractRotorID INT NULL ,
  GUID CHAR(36) NULL ,
  name TEXT NULL ,
  serialNumber TEXT NULL ,
  stretchFunction TEXT NULL ,
  omega2_t FLOAT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (rotorID) ,
  INDEX abstractRotorID (abstractRotorID ASC) ,
  CONSTRAINT abstractRotorID
    FOREIGN KEY (abstractRotorID )
    REFERENCES us3.abstractRotor (abstractRotorID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.experiment
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.experiment ;

CREATE  TABLE IF NOT EXISTS us3.experiment (
  experimentID INT NOT NULL AUTO_INCREMENT ,
  projectID INT NULL ,
  labID INT NULL ,
  instrumentID INT NOT NULL ,
  operatorID INT NULL ,
  rotorID INT NULL ,
  GUID CHAR(36) NULL ,
  type ENUM('velocity', 'equilibrium', 'other') NULL ,
  dateBegin DATE NOT NULL ,
  runTemp FLOAT NULL ,
  label VARCHAR(80) NULL ,
  comment TEXT NULL ,
  centrifugeProtocol TEXT NULL ,
  dateUpdated DATE NULL ,
  PRIMARY KEY (experimentID) ,
  INDEX projectID (projectID ASC) ,
  INDEX operatorID (operatorID ASC) ,
  INDEX instrumentID (instrumentID ASC) ,
  INDEX labID (labID ASC) ,
  INDEX rotorID (rotorID ASC) ,
  CONSTRAINT projectID
    FOREIGN KEY (projectID )
    REFERENCES us3.project (projectID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT operatorID
    FOREIGN KEY (operatorID )
    REFERENCES us3.people (personID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT instrumentID
    FOREIGN KEY (instrumentID )
    REFERENCES us3.instrument (instrumentID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT labID
    FOREIGN KEY (labID )
    REFERENCES us3.lab (labID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT rotorID
    FOREIGN KEY (rotorID )
    REFERENCES us3.rotor (rotorID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.abstractCenterpiece
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.abstractCenterpiece ;

CREATE  TABLE IF NOT EXISTS us3.abstractCenterpiece (
  abstractCenterpieceID INT NOT NULL AUTO_INCREMENT ,
  loadMethod enum('top', 'fill-hole') NULL ,
  GUID CHAR(36) NULL ,
  name TEXT NULL ,
  materialName TEXT NULL ,
  maxRPM INT NULL ,
  pathLength FLOAT NULL ,
  canHoldSample INT NULL ,
  materialRefURI TEXT NULL ,
  centerpieceRefURI TEXT NULL ,
  dataUpdated TIMESTAMP NULL ,
  PRIMARY KEY (abstractCenterpieceID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.abstractChannel
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.abstractChannel ;

CREATE  TABLE IF NOT EXISTS us3.abstractChannel (
  abstractChannel_ID INT NOT NULL AUTO_INCREMENT ,
  channelType enum('reference') NULL ,
  channelShape enum( 'sector', 'square' ) NULL ,
  GUID CHAR(36) NULL ,
  name VARCHAR(100) NULL ,
  number INT NULL ,
  radialBegin FLOAT NULL ,
  radialEnd FLOAT NULL ,
  degreesWide FLOAT NULL ,
  degreesOffset FLOAT NULL ,
  radialBandTop FLOAT NULL ,
  radialBandBottom FLOAT NULL ,
  radialMeniscusPos FLOAT NULL ,
  dateUpdated DATE NULL ,
  PRIMARY KEY (abstractChannel_ID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.channel
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.channel ;

CREATE  TABLE IF NOT EXISTS us3.channel (
  channel_ID INT NOT NULL AUTO_INCREMENT ,
  abstractChannel_ID INT NULL DEFAULT NULL ,
  GUID CHAR(36) NULL ,
  comments TEXT NULL DEFAULT NULL ,
  dateUpdated TIMESTAMP NULL DEFAULT NULL ,
  PRIMARY KEY (channel_ID) ,
  INDEX abstractChannel_ID (abstractChannel_ID ASC) ,
  CONSTRAINT abstractChannel_ID
    FOREIGN KEY (abstractChannel_ID )
    REFERENCES us3.abstractChannel (abstractChannel_ID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.cell
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.cell ;

CREATE  TABLE IF NOT EXISTS us3.cell (
  cell_ID INT NOT NULL AUTO_INCREMENT ,
  cellType enum( 'counterbalance', 'sample', 'reference' ) NULL ,
  windowType enum( 'Quartz', 'Saffire' ) NULL ,
  abstractCenterpieceID INT NULL ,
  experimentID INT NULL ,
  GUID CHAR(36) NULL ,
  name TEXT NULL ,
  holeNumber INT NULL ,
  housing TEXT NULL ,
  centerpieceSerialNumber TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (cell_ID) ,
  INDEX abstractCenterpieceID (abstractCenterpieceID ASC) ,
  INDEX c_experimentID (experimentID ASC) ,
  CONSTRAINT c_abstractCenterpieceID
    FOREIGN KEY (abstractCenterpieceID )
    REFERENCES us3.abstractCenterpiece (abstractCenterpieceID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT c_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES us3.experiment (experimentID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.rawData
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.rawData ;

CREATE  TABLE IF NOT EXISTS us3.rawData (
  rawDataID INT NOT NULL AUTO_INCREMENT ,
  label VARCHAR(80) NOT NULL ,
  data LONGBLOB NOT NULL ,
  comment TEXT NULL ,
  experimentID INT NOT NULL ,
  channel_ID INT NOT NULL ,
  PRIMARY KEY (rawDataID) ,
  INDEX experimentID (experimentID ASC) ,
  INDEX channelID (channel_ID ASC) ,
  CONSTRAINT experimentID
    FOREIGN KEY (experimentID )
    REFERENCES us3.experiment (experimentID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT rd_channelID
    FOREIGN KEY (channel_ID )
    REFERENCES us3.channel (channel_ID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.editedData
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.editedData ;

CREATE  TABLE IF NOT EXISTS us3.editedData (
  editedDataID INT NOT NULL AUTO_INCREMENT ,
  rawDataID INT NOT NULL ,
  label VARCHAR(80) NOT NULL ,
  data LONGBLOB NOT NULL ,
  comment TEXT NULL ,
  PRIMARY KEY (editedDataID) ,
  INDEX rawDataID (rawDataID ASC) ,
  CONSTRAINT rawDataID
    FOREIGN KEY (rawDataID )
    REFERENCES us3.rawData (rawDataID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.results
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.results ;

CREATE  TABLE IF NOT EXISTS us3.results (
  resultsID INT NOT NULL AUTO_INCREMENT ,
  editedDataID INT NOT NULL ,
  label VARCHAR(80) NOT NULL ,
  type ENUM('velocity', 'equilibrium', 'equilproject', 'montecarlo', 'other') NOT NULL ,
  reportData LONGBLOB NULL ,
  resultData LONGBLOB NULL ,
  PRIMARY KEY (resultsID) ,
  INDEX editDataID (editedDataID ASC) ,
  CONSTRAINT editDataID
    FOREIGN KEY (editedDataID )
    REFERENCES us3.editedData (editedDataID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.projectPerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.projectPerson ;

CREATE  TABLE IF NOT EXISTS us3.projectPerson (
  projectID INT NOT NULL ,
  personID INT NOT NULL ,
  INDEX projectID (projectID ASC) ,
  INDEX personID (personID ASC) ,
  INDEX pp_projectID (projectID ASC) ,
  CONSTRAINT personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT pp_projectID
    FOREIGN KEY (projectID )
    REFERENCES us3.project (projectID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.experimentPerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.experimentPerson ;

CREATE  TABLE IF NOT EXISTS us3.experimentPerson (
  experimentID INT NOT NULL ,
  personID INT NOT NULL ,
  INDEX ep_experimentID (experimentID ASC) ,
  INDEX ep_personID (personID ASC) ,
  CONSTRAINT ep_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES us3.experiment (experimentID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT ep_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.buffer
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.buffer ;

CREATE  TABLE IF NOT EXISTS us3.buffer (
  bufferID INT NOT NULL AUTO_INCREMENT ,
  GUID CHAR(36) NULL ,
  description TEXT NULL DEFAULT NULL ,
  spectrum TEXT NULL DEFAULT NULL ,
  pH FLOAT NULL DEFAULT NULL ,
  viscosity FLOAT NULL DEFAULT NULL ,
  density FLOAT NULL DEFAULT NULL ,
  PRIMARY KEY (bufferID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.bufferComponent
-- Handle bufferComponentID in code, so they can be
--  the same on multiple databases
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.bufferComponent ;

CREATE  TABLE IF NOT EXISTS us3.bufferComponent (
  bufferComponentID INT NOT NULL UNIQUE ,
  units VARCHAR(8) NOT NULL DEFAULT 'mM', 
  description TEXT NULL DEFAULT NULL ,
  viscosity TEXT NULL DEFAULT NULL ,
  density TEXT NULL DEFAULT NULL ,
  PRIMARY KEY (bufferComponentID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.bufferLink
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.bufferLink ;

CREATE  TABLE IF NOT EXISTS us3.bufferLink (
  bufferID INT NOT NULL ,
  bufferComponentID INT NOT NULL ,
  concentration FLOAT NULL ,
  INDEX bl_bufferID (bufferID ASC) ,
  INDEX bl_bufferComponentID (bufferComponentID ASC) ,
  CONSTRAINT bl_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES us3.buffer (bufferID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT bl_bufferComponentID
    FOREIGN KEY (bufferComponentID )
    REFERENCES us3.bufferComponent (bufferComponentID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.bufferPerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.bufferPerson ;

CREATE  TABLE IF NOT EXISTS us3.bufferPerson (
  bufferID INT NOT NULL ,
  personID INT NOT NULL ,
  private TINYINT NOT NULL DEFAULT 0,
  PRIMARY KEY (bufferID) ,
  INDEX bp_personID (personID ASC) ,
  INDEX bp_bufferID (bufferID ASC) ,
  CONSTRAINT bp_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT bp_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES us3.buffer (bufferID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.bufferRefraction
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.bufferRefraction ;

CREATE  TABLE IF NOT EXISTS us3.bufferRefraction (
  refractionID INT NOT NULL AUTO_INCREMENT ,
  bufferID INT NULL ,
  lambda FLOAT NULL ,
  refractiveIndex FLOAT NULL ,
  PRIMARY KEY (refractionID) ,
  INDEX br_bufferID (bufferID ASC) ,
  CONSTRAINT br_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES us3.buffer (bufferID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;

-- -----------------------------------------------------
-- Table us3.bufferExtinction
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.bufferExtinction ;

CREATE  TABLE IF NOT EXISTS us3.bufferExtinction (
  extinctionID INT NOT NULL AUTO_INCREMENT ,
  bufferID INT NULL ,
  lambda FLOAT NULL ,
  molarExtinctionCoef FLOAT NULL ,
  PRIMARY KEY (extinctionID) ,
  INDEX be_bufferID (bufferID ASC) ,
  CONSTRAINT be_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES us3.buffer (bufferID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.analyte
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.analyte ;

CREATE  TABLE IF NOT EXISTS us3.analyte (
  analyteID INT NOT NULL AUTO_INCREMENT ,
  GUID CHAR(36) NULL ,
  type ENUM('DNA', 'Peptide', 'Other') NULL DEFAULT NULL ,
  sequence TEXT NULL DEFAULT NULL ,
  vbar FLOAT NULL DEFAULT NULL ,
  description TEXT NULL DEFAULT NULL ,
  spectrum TEXT NULL DEFAULT NULL ,
  molecularWeight FLOAT NULL DEFAULT NULL ,
  PRIMARY KEY (analyteID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.analytePerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.analytePerson ;

CREATE  TABLE IF NOT EXISTS us3.analytePerson (
  analyteID INT NOT NULL ,
  personID INT NOT NULL ,
  PRIMARY KEY (analyteID) ,
  INDEX ap_personID (personID ASC) ,
  INDEX ap_analyteID (analyteID ASC) ,
  CONSTRAINT ap_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT ap_analyteID
    FOREIGN KEY (analyteID )
    REFERENCES us3.analyte (analyteID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.analyteRefraction
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.analyteRefraction ;

CREATE  TABLE IF NOT EXISTS us3.analyteRefraction (
  refractionID INT NOT NULL AUTO_INCREMENT ,
  analyteID INT NULL ,
  lambda FLOAT NULL ,
  refractiveIndex FLOAT NULL ,
  PRIMARY KEY (refractionID) ,
  INDEX ar_analyteID (analyteID ASC) ,
  CONSTRAINT ar_analyteID
    FOREIGN KEY (analyteID )
    REFERENCES us3.analyte (analyteID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;

-- -----------------------------------------------------
-- Table us3.analyteExtinction
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.analyteExtinction ;

CREATE  TABLE IF NOT EXISTS us3.analyteExtinction (
  extinctionID INT NOT NULL AUTO_INCREMENT ,
  analyteID INT NULL ,
  lambda FLOAT NULL ,
  molarExtinctionCoef FLOAT NULL ,
  PRIMARY KEY (extinctionID) ,
  INDEX ae_analyteID (analyteID ASC) ,
  CONSTRAINT ae_analyteID
    FOREIGN KEY (analyteID )
    REFERENCES us3.analyte (analyteID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.solution
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.solution ;

CREATE  TABLE IF NOT EXISTS us3.solution (
  solutionID INT NOT NULL AUTO_INCREMENT ,
  GUID CHAR(36) NULL ,
  description VARCHAR(80) NOT NULL ,
  storageTemp TINYINT NULL DEFAULT NULL ,
  notes TEXT NULL DEFAULT NULL ,
  PRIMARY KEY (solutionID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.solutionBuffer
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.solutionBuffer ;

CREATE  TABLE IF NOT EXISTS us3.solutionBuffer (
  solutionID INT NOT NULL ,
  bufferID INT NOT NULL ,
  PRIMARY KEY (solutionID) ,
  INDEX sb_solutionID (solutionID ASC) ,
  INDEX sb_bufferID (bufferID ASC) ,
  CONSTRAINT sb_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES us3.solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT sb_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES us3.buffer (bufferID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.solutionAnalyte
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.solutionAnalyte ;

CREATE  TABLE IF NOT EXISTS us3.solutionAnalyte (
  solutionID INT NOT NULL ,
  analyteID INT NOT NULL ,
  amount FLOAT NOT NULL ,
  PRIMARY KEY (solutionID) ,
  INDEX sa_solutionID (solutionID ASC) ,
  INDEX sa_analyteID (analyteID ASC) ,
  CONSTRAINT sa_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES us3.solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT sa_analyteID
    FOREIGN KEY (analyteID )
    REFERENCES us3.analyte (analyteID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.solutionPerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.solutionPerson ;

CREATE  TABLE IF NOT EXISTS us3.solutionPerson (
  solutionID INT NOT NULL ,
  personID INT NOT NULL ,
  PRIMARY KEY (solutionID) ,
  INDEX sp_personID (personID ASC) ,
  INDEX sp_solutionID (solutionID ASC) ,
  CONSTRAINT sp_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT sp_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES us3.solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.experimentSolutionChannel
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.experimentSolutionChannel ;

CREATE  TABLE IF NOT EXISTS us3.experimentSolutionChannel (
  experimentID INT NOT NULL ,
  solutionID INT NOT NULL ,
  channel_ID INT NOT NULL ,
  INDEX experimentID (experimentID ASC) ,
  INDEX channelID (channel_ID ASC) ,
  INDEX solutionID (solutionID ASC) ,
  CONSTRAINT esc_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES us3.experiment (experimentID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT esc_channelID
    FOREIGN KEY (channel_ID )
    REFERENCES us3.channel (channel_ID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT esc_solutionID
    FOREIGN KEY ( solutionID )
    REFERENCES us3.solution (solutionID)
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.image
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.image ;

CREATE  TABLE IF NOT EXISTS us3.image (
  imageID INT NOT NULL AUTO_INCREMENT ,
  GUID CHAR(36) NULL ,
  description VARCHAR(80) NOT NULL DEFAULT 'No description was entered for this image' ,
  gelPicture LONGBLOB NOT NULL ,
  date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ,
  PRIMARY KEY (imageID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.imagePerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.imagePerson ;

CREATE  TABLE IF NOT EXISTS us3.imagePerson (
  imageID INT NOT NULL ,
  personID INT NOT NULL ,
  INDEX ip_personID (personID ASC) ,
  INDEX imageID (imageID ASC) ,
  CONSTRAINT ip_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT imageID
    FOREIGN KEY (imageID )
    REFERENCES us3.image (imageID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.imageAnalyte
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.imageAnalyte ;

CREATE  TABLE IF NOT EXISTS us3.imageAnalyte (
  imageID INT NOT NULL ,
  analyteID INT NOT NULL ,
  PRIMARY KEY (imageID) ,
  INDEX ia_imageID (imageID ASC) ,
  INDEX ia_analyteID (analyteID ASC) ,
  CONSTRAINT ia_imageID
    FOREIGN KEY (imageID )
    REFERENCES us3.image (imageID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT ia_analyteID
    FOREIGN KEY (analyteID )
    REFERENCES us3.analyte (analyteID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.imageSolution
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.imageSolution ;

CREATE  TABLE IF NOT EXISTS us3.imageSolution (
  imageID INT NOT NULL ,
  solutionID INT NOT NULL ,
  PRIMARY KEY (imageID) ,
  INDEX is_imageID (imageID ASC) ,
  INDEX is_solutionID (solutionID ASC) ,
  CONSTRAINT is_imageID
    FOREIGN KEY (imageID )
    REFERENCES us3.image (imageID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT is_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES us3.solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.avivFluorescence
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.avivFluorescence ;

CREATE  TABLE IF NOT EXISTS us3.avivFluorescence (
  avivFluorescenceID INT NOT NULL AUTO_INCREMENT ,
  opticalSystemSettingID INT NULL ,
  topRadius FLOAT NULL ,
  bottomRadius FLOAT NULL ,
  mmStepSize FLOAT NULL ,
  replicates INT NULL ,
  nmExcitation FLOAT NULL ,
  nmEmission FLOAT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (avivFluorescenceID) ,
  INDEX opticalSystemSettingID (opticalSystemSettingID ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.beckmanRadialAbsorbance
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.beckmanRadialAbsorbance ;

CREATE  TABLE IF NOT EXISTS us3.beckmanRadialAbsorbance (
  beckmanRadialAbsorbanceID INT NOT NULL AUTO_INCREMENT ,
  opticalSystemSettingID INT NULL ,
  topRadius FLOAT NULL ,
  bottomRadius FLOAT NULL ,
  mmStepSize FLOAT NULL ,
  replicates INT NULL ,
  isContinuousMode BOOLEAN NULL ,
  isIntensity BOOLEAN NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (beckmanRadialAbsorbanceID) ,
  INDEX opticalSystemSettingID (opticalSystemSettingID ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.beckmanWavelengthAbsorbance
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.beckmanWavelengthAbsorbance ;

CREATE  TABLE IF NOT EXISTS us3.beckmanWavelengthAbsorbance (
  beckmanWavelengthAbsorbanceID INT NOT NULL AUTO_INCREMENT ,
  opticalSystemSettingID INT NULL ,
  radialPosition FLOAT NULL ,
  startWavelength FLOAT NULL ,
  endWavelength FLOAT NULL ,
  nmStepsize FLOAT NULL ,
  replicates INT NULL ,
  isIntensity BOOLEAN NULL ,
  secondsBetween INT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (beckmanWavelengthAbsorbanceID) ,
  INDEX opticalSystemSettingID (opticalSystemSettingID ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.beckmanInterference
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.beckmanInterference ;

CREATE  TABLE IF NOT EXISTS us3.beckmanInterference (
  beckmanInterferenceID INT NOT NULL AUTO_INCREMENT ,
  opticalSystemSettingID INT NULL ,
  topRadious FLOAT NULL ,
  bottomRadius FLOAT NULL ,
  pixelsPerFringe INT NULL ,
  numberOfFringes INT NULL ,
  radialIncrement FLOAT NULL ,
  cameraGain FLOAT NULL ,
  cameraOffset FLOAT NULL ,
  cameraGamma FLOAT NULL ,
  barAngleDegrees FLOAT NULL ,
  startingRow INT NULL ,
  endingRow INT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (beckmanInterferenceID) ,
  INDEX opticalSystemSettingID (opticalSystemSettingID ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.multiWavelengthSystem
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.multiWavelengthSystem ;

CREATE  TABLE IF NOT EXISTS us3.multiWavelengthSystem (
  multiWavelengthSystemID INT NOT NULL AUTO_INCREMENT ,
  opticalSystemSettingID INT NULL ,
  startWavelength FLOAT NULL ,
  endWavelength FLOAT NULL ,
  nmStepsize FLOAT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (multiWavelengthSystemID) ,
  INDEX opticalSystemSettingID (opticalSystemSettingID ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.otherOpticalSystem
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.otherOpticalSystem ;

CREATE  TABLE IF NOT EXISTS us3.otherOpticalSystem (
  otherOpticalSystemID INT NOT NULL AUTO_INCREMENT ,
  opticalSystemSettingID INT NULL ,
  name TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (otherOpticalSystemID) ,
  INDEX opticalSystemSettingID (opticalSystemSettingID ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.opticalSystemSetting
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.opticalSystemSetting ;

CREATE  TABLE IF NOT EXISTS us3.opticalSystemSetting (
  opticalSystemSettingID INT NOT NULL AUTO_INCREMENT ,
  rawDataID INT NULL ,
  GUID CHAR(36) NULL ,
  name TEXT NULL ,
  value FLOAT NULL ,
  dateUpdated TIMESTAMP NULL ,
  secondsDuration INT NULL ,
  hwType ENUM( 'fluor', 'wlAbsorb', 'interfere', 'radialAbs', 'MWL', 'other') NULL ,
  hwIndex INT NULL ,
  channelID INT NULL ,
  PRIMARY KEY (opticalSystemSettingID) ,
  INDEX oss_rawDataID (rawDataID ASC) ,
  INDEX oss_channelID (channelID ASC) ,
  INDEX oss_fluorescenceOpSysID (opticalSystemSettingID ASC) ,
  CONSTRAINT oss_rawDataID
    FOREIGN KEY (rawDataID )
    REFERENCES us3.rawData (rawDataID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT oss_channelID
    FOREIGN KEY (channelID )
    REFERENCES us3.channel (channel_ID )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT oss_fluorescenceOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES us3.avivFluorescence (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT oss_radialOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES us3.beckmanRadialAbsorbance (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT oss_wavelengthOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES us3.beckmanWavelengthAbsorbance (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT oss_interferenceOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES us3.beckmanInterference (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT oss_MWLOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES us3.multiWavelengthSystem (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT oss_otherOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES us3.otherOpticalSystem (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.permits
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.permits ;

CREATE  TABLE IF NOT EXISTS us3.permits (
  investigatorID INT NOT NULL ,
  collaboratorID INT NOT NULL ,
  instrumentID INT NOT NULL ,
  PRIMARY KEY (investigatorID) ,
  INDEX p_investigatorID (investigatorID ASC) ,
  INDEX p_collaboratorID (collaboratorID ASC) ,
  INDEX p_instrumentID (instrumentID ASC) ,
  CONSTRAINT p_investigatorID
    FOREIGN KEY (investigatorID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT p_collaboratorID
    FOREIGN KEY (collaboratorID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT p_instrumentID
    FOREIGN KEY (instrumentID )
    REFERENCES us3.instrument (instrumentID )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;



SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
