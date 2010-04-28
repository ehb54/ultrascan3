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
-- Table us3.lab
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.lab ;

CREATE  TABLE IF NOT EXISTS us3.lab (
  labID INT NOT NULL AUTO_INCREMENT ,
  GUID CHAR(36) NOT NULL UNIQUE,
  name TEXT NULL ,
  building TEXT NULL ,
  room TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (labID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.instrument
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.instrument ;

CREATE  TABLE IF NOT EXISTS us3.instrument (
  instrumentID INT NOT NULL AUTO_INCREMENT ,
  labID INT NOT NULL ,
  name TEXT NULL ,
  serialNumber TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (instrumentID) ,
  INDEX ndx_instrument_labID (labID ASC) ,
  CONSTRAINT fk_instrument_labID
    FOREIGN KEY ( labID )
    REFERENCES us3.lab ( labID )
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.abstractRotor
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.abstractRotor ;

CREATE  TABLE IF NOT EXISTS us3.abstractRotor (
  abstractRotorID INT NOT NULL UNIQUE ,
  GUID CHAR(36) NULL ,
  name enum( 'Simulation', 'AN50', 'AN60', 'CFA' ) NULL ,
  materialName enum( 'titanium', 'carbon' ) NULL ,
  numHoles INT NULL ,
  maxRPM INT NULL ,
  magnetOffset FLOAT NULL ,
  cellCenter FLOAT NULL ,
  defaultStretch TEXT NULL ,
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
  labID INT NOT NULL ,
  GUID CHAR(36) NOT NULL UNIQUE,
  name TEXT NULL ,
  serialNumber TEXT NULL ,
  stretchFunction TEXT NULL ,
  omega2_t FLOAT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (rotorID) ,
  INDEX ndx_rotor_abstractRotorID (abstractRotorID ASC) ,
  INDEX ndx_rotor_labID (labID ASC) ,
  CONSTRAINT fk_rotor_abstractRotorID
    FOREIGN KEY (abstractRotorID )
    REFERENCES us3.abstractRotor (abstractRotorID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT fk_rotor_labID
    FOREIGN KEY (labID )
    REFERENCES us3.lab (labID )
    ON DELETE CASCADE 
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.experiment
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.experiment ;

CREATE  TABLE IF NOT EXISTS us3.experiment (
  experimentID INT NOT NULL AUTO_INCREMENT ,
  projectID INT NULL ,
  runID VARCHAR(80) NOT NULL ,
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
  INDEX ndx_experiment_projectID (projectID ASC) ,
  INDEX ndx_experiment_operatorID (operatorID ASC) ,
  INDEX ndx_experiment_instrumentID (instrumentID ASC) ,
  INDEX ndx_experiment_labID (labID ASC) ,
  INDEX ndx_experiment_rotorID (rotorID ASC) ,
  CONSTRAINT fk_experiment_projectID
    FOREIGN KEY (projectID )
    REFERENCES us3.project (projectID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT fk_experiment_operatorID
    FOREIGN KEY (operatorID )
    REFERENCES us3.people (personID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT fk_experiment_instrumentID
    FOREIGN KEY (instrumentID )
    REFERENCES us3.instrument (instrumentID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT fk_experiment_labID
    FOREIGN KEY (labID )
    REFERENCES us3.lab (labID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT fk_experiment_rotorID
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
  abstractCenterpieceID INT NOT NULL UNIQUE ,
  loadMethod enum('top', 'fill') NULL ,
  GUID CHAR(36) NULL ,
  name TEXT NULL ,
  materialName TEXT NULL ,
  channels INT NOT NULL ,
  bottom VARCHAR(20) NOT NULL ,
  shape enum( 'standard', 'rectangular', 'circular', 'synthetic', 'band forming' )
              NOT NULL DEFAULT 'standard',
  maxRPM INT NULL ,
  pathLength FLOAT NULL ,
  angle FLOAT NULL ,
  width FLOAT NULL ,
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
  abstractChannelID INT NOT NULL AUTO_INCREMENT ,
  channelType enum('reference','sample') NULL ,
  channelShape enum( 'sector', 'rectangular' ) NULL ,
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
  PRIMARY KEY (abstractChannelID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.channel
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.channel ;

CREATE  TABLE IF NOT EXISTS us3.channel (
  channelID INT NOT NULL AUTO_INCREMENT ,
  abstractChannelID INT NULL ,
  GUID CHAR(36) NULL ,
  comments TEXT NULL DEFAULT NULL ,
  dateUpdated TIMESTAMP NULL DEFAULT NULL ,
  PRIMARY KEY (channelID) ,
  INDEX ndx_channel_abstractChannelID (abstractChannelID ASC) ,
  CONSTRAINT fk_channel_abstractChannelID
    FOREIGN KEY (abstractChannelID )
    REFERENCES us3.abstractChannel (abstractChannelID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.cell
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.cell ;

CREATE  TABLE IF NOT EXISTS us3.cell (
  cellID INT NOT NULL AUTO_INCREMENT ,
  cellType enum( 'counterbalance', 'sample', 'reference' ) NULL ,
  windowType enum( 'quartz', 'saffire' ) NULL ,
  abstractCenterpieceID INT NULL ,
  experimentID INT NULL ,
  GUID CHAR(36) NULL ,
  name TEXT NULL ,
  holeNumber INT NULL ,
  housing TEXT NULL ,
  centerpieceSerialNumber TEXT NULL ,
  numChannels INT ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (cellID) ,
  INDEX ndx_cell_abstractCenterpieceID (abstractCenterpieceID ASC) ,
  INDEX ndx_cell_experimentID (experimentID ASC) ,
  CONSTRAINT fk_cell_abstractCenterpieceID
    FOREIGN KEY (abstractCenterpieceID )
    REFERENCES us3.abstractCenterpiece (abstractCenterpieceID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT fk_cell_experimentID
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
  channelID INT NOT NULL ,
  PRIMARY KEY (rawDataID) ,
  INDEX ndx_rawData_experimentID (experimentID ASC) ,
  INDEX ndx_rawData_channelID (channelID ASC) ,
  CONSTRAINT fk_rawData_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES us3.experiment (experimentID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT fk_rawData_channelID
    FOREIGN KEY (channelID )
    REFERENCES us3.channel (channelID )
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
  INDEX ndx_editedData_rawDataID (rawDataID ASC) ,
  CONSTRAINT fk_editedData_rawDataID
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
  INDEX ndx_results_editDataID (editedDataID ASC) ,
  CONSTRAINT fk_results_editDataID
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
  INDEX ndx_projectPerson_personID  (personID ASC) ,
  INDEX ndx_projectPerson_projectID (projectID ASC) ,
  CONSTRAINT fk_projectPerson_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_projectPerson_projectID
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
  INDEX ndx_experimentPerson_experimentID (experimentID ASC) ,
  INDEX ndx_experimentPerson_personID (personID ASC) ,
  CONSTRAINT fk_experimentPerson_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES us3.experiment (experimentID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_experimentPerson_personID
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
  GUID CHAR(36) NOT NULL UNIQUE ,
  description TEXT NULL DEFAULT NULL ,
  compressibility FLOAT NULL DEFAULT NULL ,
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
  INDEX ndx_bufferLink_bufferID (bufferID ASC) ,
  INDEX ndx_bufferLink_bufferComponentID (bufferComponentID ASC) ,
  CONSTRAINT fk_bufferLink_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES us3.buffer (bufferID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_bufferLink_bufferComponentID
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
  INDEX ndx_bufferPerson_personID (personID ASC) ,
  INDEX ndx_bufferPerson_bufferID (bufferID ASC) ,
  CONSTRAINT fk_bufferPerson_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_bufferPerson_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES us3.buffer (bufferID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.analyte
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.analyte ;

CREATE  TABLE IF NOT EXISTS us3.analyte (
  analyteID INT NOT NULL AUTO_INCREMENT ,
  GUID CHAR(36) NOT NULL UNIQUE ,
  type ENUM('DNA', 'RNA', 'Protein', 'Other') NULL DEFAULT 'Other' ,
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
  INDEX ndx_analytePerson_personID  (personID ASC) ,
  INDEX ndx_analytePerson_analyteID (analyteID ASC) ,
  CONSTRAINT fk_analytePerson_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_analytePerson_analyteID
    FOREIGN KEY (analyteID )
    REFERENCES us3.analyte (analyteID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.spectrum
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.spectrum;

CREATE TABLE IF NOT EXISTS us3.spectrum (
  spectrumID INT NOT NULL AUTO_INCREMENT ,
  componentID INT NOT NULL ,
  componentType  enum( 'Buffer', 'Analyte' ) NOT NULL,
  opticsType enum( 'Extinction', 'Refraction', 'Fluorescence' ) NOT NULL,
  lambda FLOAT NOT NULL ,
  molarCoefficient FLOAT NOT NULL ,
  PRIMARY KEY (spectrumID) ,
  INDEX ndx_component_ID (componentID ASC) )
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
  INDEX ndx_solutionBuffer_solutionID (solutionID ASC) ,
  INDEX ndx_solutionBuffer_bufferID   (bufferID ASC) ,
  CONSTRAINT fk_solutionBuffer_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES us3.solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_solutionBuffer_bufferID
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
  INDEX ndx_solutionAnalyte_solutionID (solutionID ASC) ,
  INDEX ndx_solutionAnalyte_analyteID (analyteID ASC) ,
  CONSTRAINT fk_solutionAnalyte_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES us3.solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_solutionAnalyte_analyteID
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
  INDEX ndx_solutionPerson_personID (personID ASC) ,
  INDEX ndx_solutionPerson_solutionID (solutionID ASC) ,
  CONSTRAINT fk_solutionPerson_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_solutionPerson_solutionID
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
  channelID INT NOT NULL ,
  INDEX ndx_experimentSolutionChannel_experimentID (experimentID ASC) ,
  INDEX ndx_experimentSolutionChannel_channelID (channelID ASC) ,
  INDEX ndx_experimentSolutionChannel_solutionID (solutionID ASC) ,
  CONSTRAINT fk_experimentSolutionChannel_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES us3.experiment (experimentID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_experimentSolutionChannel_channelID
    FOREIGN KEY (channelID )
    REFERENCES us3.channel (channelID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT fk_experimentSolutionChannel_solutionID
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
  INDEX ndx_imagePerson_personID (personID ASC) ,
  INDEX ndx_imagePerson_imageID (imageID ASC) ,
  CONSTRAINT fk_imagePerson_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_imagePerson_imageID
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
  INDEX ndx_imageAnalyte_imageID   (imageID ASC) ,
  INDEX ndx_imageAnalyte_analyteID (analyteID ASC) ,
  CONSTRAINT fk_imageAnalyte_imageID
    FOREIGN KEY (imageID )
    REFERENCES us3.image (imageID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_imageAnalyte_analyteID
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
  INDEX ndx_imageSolution_imageID    (imageID ASC) ,
  INDEX ndx_imageSolution_solutionID (solutionID ASC) ,
  CONSTRAINT fk_imageSolution_imageID
    FOREIGN KEY (imageID )
    REFERENCES us3.image (imageID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_imageSolution_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES us3.solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.avivFluorescence
-- -----------------------------------------------------
DROP TABLE IF EXISTS us3.avivFluorescence ;

CREATE  TABLE IF NOT EXISTS us3.avivfluorescence (
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
  INDEX ndx_avivfluorescence_opticalSystemSettingID 
    (opticalSystemSettingID ASC) )
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
  INDEX ndx_beckmanRadialAbsorbance_opticalSystemSettingID 
    (opticalSystemSettingID ASC) )
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
  INDEX ndx_beckmanWavelengthAbsorbance_opticalSystemSettingID 
    (opticalSystemSettingID ASC) )
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
  INDEX ndx_beckmanInterference_opticalSystemSettingID 
    (opticalSystemSettingID ASC) )
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
  INDEX ndx_multiWavelengthSystem_opticalSystemSettingID 
    (opticalSystemSettingID ASC) )
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
  INDEX ndx_otherOpticalSystem_opticalSystemSettingID 
    (opticalSystemSettingID ASC) )
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
  INDEX ndx_opticalSystemSetting_rawDataID (rawDataID ASC) ,
  INDEX ndx_opticalSystemSetting_channelID (channelID ASC) ,
  INDEX ndx_opticalSystemSetting_fluorescenceOpSysID 
    (opticalSystemSettingID ASC) ,
  CONSTRAINT fk_opticalSystemSetting_rawDataID
    FOREIGN KEY (rawDataID )
    REFERENCES us3.rawData (rawDataID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT fk_opticalSystemSetting_channelID
    FOREIGN KEY (channelID )
    REFERENCES us3.channel (channelID )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT fk_opticalSystemSetting_fluorescenceOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES us3.avivFluorescence (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_opticalSystemSetting_radialOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES us3.beckmanRadialAbsorbance (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_opticalSystemSetting_wavelengthOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES us3.beckmanWavelengthAbsorbance (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_opticalSystemSetting_interferenceOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES us3.beckmanInterference (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_opticalSystemSetting_MWLOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES us3.multiWavelengthSystem (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_opticalSystemSetting_otherOpSysID
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
  permitID INT NOT NULL AUTO_INCREMENT ,
  personID INT NOT NULL ,
  collaboratorID INT default NULL ,
  instrumentID INT NOT NULL ,
  PRIMARY KEY (permitID) ,
  INDEX ndx_permits_personID (personID ASC) ,
  INDEX ndx_permits_collaboratorID (collaboratorID ASC) ,
  INDEX ndx_permits_instrumentID (instrumentID ASC) ,
  CONSTRAINT fk_permits_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_permits_collaboratorID
    FOREIGN KEY (collaboratorID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_permits_instrumentID
    FOREIGN KEY (instrumentID )
    REFERENCES us3.instrument (instrumentID )
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;

SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
