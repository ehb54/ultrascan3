SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS;
SET @OLD_SQL_MODE=@@SQL_MODE;

SET UNIQUE_CHECKS = 0;
SET FOREIGN_KEY_CHECKS=0;
SET SQL_MODE='STRICT_TRANS_TABLES,STRICT_ALL_TABLES,NO_ZERO_IN_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER';

-- Uncomment the following lines to create and use us3 schema
-- CREATE SCHEMA IF NOT EXISTS us3 DEFAULT CHARACTER SET ascii COLLATE ascii_bin ;
-- USE us3;

-- -----------------------------------------------------
-- Table people
-- -----------------------------------------------------
DROP TABLE IF EXISTS people ;

CREATE  TABLE IF NOT EXISTS people (
  personID int(11) NOT NULL AUTO_INCREMENT ,
  personGUID CHAR(36) NOT NULL UNIQUE,
  fname VARCHAR(30) NULL ,
  lname VARCHAR(30) NULL ,
  address VARCHAR(255) NULL ,
  city VARCHAR(30) NULL ,
  state VARCHAR(20) NULL ,
  zip VARCHAR(10) NULL ,
  country VARCHAR(64) NULL ,
  phone VARCHAR(24) NULL ,
  email VARCHAR(63) NOT NULL UNIQUE ,
  organization VARCHAR(45) NULL ,
  username VARCHAR(80) NULL,
  password VARCHAR(80) NOT NULL ,
  activated TINYINT(1) NOT NULL DEFAULT false ,
  signup TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ,
  lastLogin DATETIME NULL ,
  clusterAuthorizations VARCHAR(255) NOT NULL default 'lonestar5:stampede2:comet:jetstream',
  userlevel TINYINT NOT NULL DEFAULT 0 ,
  advancelevel TINYINT NOT NULL DEFAULT 0 ,
  PRIMARY KEY (personID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table project
-- -----------------------------------------------------
DROP TABLE IF EXISTS project ;

CREATE  TABLE IF NOT EXISTS project (
  projectID int(11) NOT NULL AUTO_INCREMENT ,
  projectGUID CHAR(36) NOT NULL UNIQUE,
  goals TEXT NULL ,
  molecules TEXT NULL ,
  purity VARCHAR(10) NULL ,
  expense TEXT NULL ,
  bufferComponents TEXT NULL ,
  saltInformation TEXT NULL ,
  AUC_questions TEXT NULL ,
  expDesign TEXT NULL ,
  notes TEXT NULL ,
  description TEXT NULL ,
  status ENUM('submitted', 'designed', 'scheduled','uploaded', 'anlyzed', 'invoiced', 'paid', 'other') NOT NULL ,
  lastUpdated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ,
  PRIMARY KEY (projectID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table lab
-- -----------------------------------------------------
DROP TABLE IF EXISTS lab ;

CREATE  TABLE IF NOT EXISTS lab (
  labID int(11) NOT NULL AUTO_INCREMENT ,
  labGUID CHAR(36) NOT NULL UNIQUE,
  name TEXT NULL ,
  building TEXT NULL ,
  room TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (labID) )
ENGINE = InnoDB;


-----------------------------------------------------
-- Table autoflow--
-----------------------------------------------------

DROP TABLE IF EXISTS autoflow;

CREATE  TABLE IF NOT EXISTS autoflow (
  ID int(11) NOT NULL AUTO_INCREMENT ,
  protName varchar(80) NULL,
  cellChNum varchar(80) NULL,
  tripleNum varchar(80) NULL,
  duration int(10)  NULL,
  runName varchar(300) NULL,
  expID  int(10) NULL,
  runID  int(10) NULL,
  status enum('LIVE_UPDATE','EDITING','EDIT_DATA','ANALYSIS','REPORT') NOT NULL,
  dataPath varchar(300) NULL,
  optimaName varchar(300) NULL,
  runStarted TIMESTAMP NULL,
  invID  INT NULL,
  created TIMESTAMP NULL,
  corrRadii enum('YES', 'NO') NOT NULL,
  expAborted enum('NO', 'YES') NOT NULL,
  label varchar(80) NULL,
  gmpRun enum ('NO', 'YES') NOT	NULL,
  filename varchar(80) NULL,
  aprofileGUID varchar(80) NULL,
  PRIMARY KEY (ID) )
ENGINE = InnoDB;



-- -----------------------------------------------------
-- Table instrument
-- -----------------------------------------------------
DROP TABLE IF EXISTS instrument ;

CREATE  TABLE IF NOT EXISTS instrument (
  instrumentID int(11) NOT NULL AUTO_INCREMENT ,
  labID int(11) NOT NULL ,
  name TEXT NULL ,
  serialNumber TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  radialCalID int(11) NOT NULL DEFAULT 0 ,
  optimaHost TEXT NULL ,
  optimaPort int(11) NULL ,
  optimaDBname TEXT NULL ,
  optimaDBusername TEXT NULL ,
  optimaDBpassw BLOB NULL ,
  chromaticAB TEXT NULL ,
  selected BOOL DEFAULT false ,
  opsys1 ENUM ('UV/visible', 'Rayleigh Interference', 'Fluorescense', '(not installed)') NOT NULL ,
  opsys2 ENUM ('UV/visible', 'Rayleigh Interference', 'Fluorescense', '(not installed)') NOT NULL ,
  opsys3 ENUM ('UV/visible', 'Rayleigh Interference', 'Fluorescense', '(not installed)') NOT NULL ,
  RadCalWvl int(11) NULL ,
  PRIMARY KEY (instrumentID) ,
  INDEX ndx_instrument_labID (labID ASC) ,
  CONSTRAINT fk_instrument_labID
    FOREIGN KEY ( labID )
    REFERENCES lab ( labID )
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table abstractRotor
-- -----------------------------------------------------
DROP TABLE IF EXISTS abstractRotor ;

CREATE  TABLE IF NOT EXISTS abstractRotor (
  abstractRotorID int(11) NOT NULL UNIQUE ,
  abstractRotorGUID CHAR(36) NULL ,
  name enum( 'Simulation', 'AN50', 'AN60', 'CFA' ) NULL ,
  materialName enum( 'Titanium', 'CarbonFiber', 'Simulation' ) NULL ,
  numHoles INT NULL ,
  maxRPM INT NULL ,
  magnetOffset FLOAT NULL ,
  cellCenter FLOAT DEFAULT 6.5 ,
  manufacturer enum( 'Beckman', 'SpinAnalytical', 'Simulation' ) NULL,
  PRIMARY KEY (abstractRotorID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table rotor
-- -----------------------------------------------------
DROP TABLE IF EXISTS rotor ;

CREATE  TABLE IF NOT EXISTS rotor (
  rotorID int(11) NOT NULL AUTO_INCREMENT ,
  abstractRotorID int(11) NULL ,
  labID int(11) NOT NULL ,
  rotorGUID CHAR(36) NOT NULL UNIQUE,
  name TEXT NULL ,
  serialNumber TEXT NULL ,
  PRIMARY KEY (rotorID) ,
  INDEX ndx_rotor_abstractRotorID (abstractRotorID ASC) ,
  INDEX ndx_rotor_labID (labID ASC) ,
  CONSTRAINT fk_rotor_abstractRotorID
    FOREIGN KEY (abstractRotorID )
    REFERENCES abstractRotor (abstractRotorID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT fk_rotor_labID
    FOREIGN KEY (labID )
    REFERENCES lab (labID )
    ON DELETE CASCADE 
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table rotorCalibration
-- -----------------------------------------------------
DROP TABLE IF EXISTS rotorCalibration ;

CREATE  TABLE IF NOT EXISTS rotorCalibration (
  rotorCalibrationID int(11) NOT NULL AUTO_INCREMENT ,
  rotorID int(11) NULL ,
  rotorCalibrationGUID CHAR(36) NOT NULL UNIQUE,
  label VARCHAR(80) NOT NULL DEFAULT '',
  report TEXT NULL ,
  coeff1 FLOAT default 0.0,
  coeff2 FLOAT default 0.0,
  omega2_t FLOAT NULL ,
  dateUpdated TIMESTAMP NULL ,
  calibrationExperimentID int(11) NULL ,
  PRIMARY KEY (rotorCalibrationID) ,
  INDEX ndx_rotorCalibration_rotorID (rotorID ASC) ,
  INDEX ndx_rotorCalibration_experimentID (calibrationExperimentID ASC) ,
  CONSTRAINT fk_rotorCalibration_rotorID
    FOREIGN KEY (rotorID )
    REFERENCES rotor (rotorID )
    ON DELETE SET NULL
    ON UPDATE CASCADE )
--  CONSTRAINT fk_rotorCalibration_experimentID
--    FOREIGN KEY (calibrationExperimentID )
--    REFERENCES experiment (experimentID ) )
--    ON DELETE SET NULL
--    ON UPDATE NO ACTION)
ENGINE = InnoDB;

-- -----------------------------------------------------
-- Table experiment
-- -----------------------------------------------------
DROP TABLE IF EXISTS experiment ;

CREATE  TABLE IF NOT EXISTS experiment (
  experimentID int(11) NOT NULL AUTO_INCREMENT ,
  projectID int(11) NULL ,
  runID VARCHAR(80) NOT NULL ,
  labID int(11) NULL ,
  instrumentID int(11) NOT NULL ,
  operatorID int(11) NULL ,
  rotorID int(11) NULL ,
  rotorCalibrationID int(11) NULL ,
  experimentGUID CHAR(36) NULL UNIQUE,
  type ENUM('velocity', 'equilibrium', 'diffusion', 'buoyancy', 'calibration', 'other') NULL 
    DEFAULT 'velocity',
  runType ENUM('RA', 'RI', 'IP', 'FI', 'WA', 'WI', 'RI+IP', 'RI+FI', 'IP+FI', 'RI+IP+FI') NULL
    DEFAULT NULL,
  dateBegin DATE NOT NULL ,
  runTemp FLOAT NULL ,
  label VARCHAR(80) NULL ,
  comment TEXT NULL ,
  RIProfile LONGTEXT NULL ,
  protocolGUID CHAR(36) NULL ,
  centrifugeProtocol TEXT NULL ,
  dateUpdated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ,
  PRIMARY KEY (experimentID) ,
  INDEX ndx_experiment_projectID (projectID ASC) ,
  INDEX ndx_experiment_operatorID (operatorID ASC) ,
  INDEX ndx_experiment_instrumentID (instrumentID ASC) ,
  INDEX ndx_experiment_labID (labID ASC) ,
  INDEX ndx_experiment_rotorID (rotorID ASC) ,
  INDEX ndx_experiment_calibrationID (rotorCalibrationID ASC) ,
  CONSTRAINT fk_experiment_projectID
    FOREIGN KEY (projectID )
    REFERENCES project (projectID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT fk_experiment_operatorID
    FOREIGN KEY (operatorID )
    REFERENCES people (personID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT fk_experiment_instrumentID
    FOREIGN KEY (instrumentID )
    REFERENCES instrument (instrumentID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT fk_experiment_labID
    FOREIGN KEY (labID )
    REFERENCES lab (labID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT fk_experiment_rotorID
    FOREIGN KEY (rotorID )
    REFERENCES rotor (rotorID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT fk_experiment_calibrationID
    FOREIGN KEY (rotorCalibrationID )
    REFERENCES rotorCalibration (rotorCalibrationID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table abstractCenterpiece
-- -----------------------------------------------------
DROP TABLE IF EXISTS abstractCenterpiece ;

CREATE  TABLE IF NOT EXISTS abstractCenterpiece (
  abstractCenterpieceID int(11) NOT NULL UNIQUE ,
  loadMethod enum('top', 'side') NULL ,
  abstractCenterpieceGUID CHAR(36) NULL ,
  name TEXT NULL ,
  materialName TEXT NULL ,
  channels INT NOT NULL ,
  bottom VARCHAR(20) NOT NULL ,
  shape enum( 'standard', 'rectangular', 'circular', 'synthetic', 
              'band forming', 'meniscus matching', 'sector' )
              NOT NULL DEFAULT 'standard',
  maxRPM INT NULL ,
  pathLength FLOAT NULL ,
  angle FLOAT NULL ,
  width FLOAT NULL ,
  canHoldSample INT NULL ,
  materialRefURI TEXT NULL ,
  centerpieceRefURI TEXT NULL ,
  dataUpdated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ,
  PRIMARY KEY (abstractCenterpieceID) )
ENGINE = InnoDB;


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


-- -----------------------------------------------------
-- Table cell
-- -----------------------------------------------------
DROP TABLE IF EXISTS cell ;

CREATE  TABLE IF NOT EXISTS cell (
  cellID int(11) NOT NULL AUTO_INCREMENT ,
  cellType enum( 'counterbalance', 'sample', 'reference' ) NULL ,
  windowType enum( 'quartz', 'saffire' ) NULL ,
  abstractCenterpieceID int(11) NULL ,
  experimentID int(11) NULL ,
  cellGUID CHAR(36) NULL ,
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
    REFERENCES abstractCenterpiece (abstractCenterpieceID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT fk_cell_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES experiment (experimentID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table abstractChannel
-- -----------------------------------------------------
DROP TABLE IF EXISTS abstractChannel ;

CREATE  TABLE IF NOT EXISTS abstractChannel (
  abstractChannelID int(11) NOT NULL AUTO_INCREMENT ,
  channelType enum('reference','sample') NULL ,
  channelShape enum( 'sector', 'rectangular' ) NULL ,
  abstractChannelGUID CHAR(36) NULL ,
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
-- Table channel
-- -----------------------------------------------------
DROP TABLE IF EXISTS channel ;

CREATE  TABLE IF NOT EXISTS channel (
  channelID int(11) NOT NULL AUTO_INCREMENT ,
  abstractChannelID int(11) NULL ,
  channelGUID CHAR(36) NULL ,
  comments TEXT NULL DEFAULT NULL ,
  dateUpdated TIMESTAMP NULL DEFAULT NULL ,
  PRIMARY KEY (channelID) ,
  INDEX ndx_channel_abstractChannelID (abstractChannelID ASC) )
--  CONSTRAINT fk_channel_abstractChannelID
--    FOREIGN KEY (abstractChannelID )
--    REFERENCES abstractChannel (abstractChannelID )
--    ON DELETE SET NULL
--    ON UPDATE NULL)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table solution
-- -----------------------------------------------------
DROP TABLE IF EXISTS solution ;

CREATE  TABLE IF NOT EXISTS solution (
  solutionID int(11) NOT NULL AUTO_INCREMENT ,
  solutionGUID CHAR(36) NULL ,
  description VARCHAR(80) NOT NULL ,
  commonVbar20 DOUBLE DEFAULT 0.0,
  storageTemp FLOAT DEFAULT NULL ,
  notes TEXT NULL DEFAULT NULL ,
  PRIMARY KEY (solutionID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table solutionBuffer
-- -----------------------------------------------------
DROP TABLE IF EXISTS solutionBuffer ;

CREATE  TABLE IF NOT EXISTS solutionBuffer (
  solutionID int(11) NOT NULL ,
  bufferID int(11) NOT NULL ,
  PRIMARY KEY (solutionID) ,
  INDEX ndx_solutionBuffer_solutionID (solutionID ASC) ,
  INDEX ndx_solutionBuffer_bufferID   (bufferID ASC) ,
  CONSTRAINT fk_solutionBuffer_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_solutionBuffer_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES buffer (bufferID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table solutionAnalyte
-- -----------------------------------------------------
DROP TABLE IF EXISTS solutionAnalyte ;

CREATE  TABLE IF NOT EXISTS solutionAnalyte (
  solutionID int(11) NOT NULL ,
  analyteID int(11) NOT NULL ,
  amount FLOAT NOT NULL ,
  INDEX ndx_solutionAnalyte_solutionID (solutionID ASC) ,
  INDEX ndx_solutionAnalyte_analyteID (analyteID ASC) ,
  CONSTRAINT fk_solutionAnalyte_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_solutionAnalyte_analyteID
    FOREIGN KEY (analyteID )
    REFERENCES analyte (analyteID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table solutionPerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS solutionPerson ;

CREATE  TABLE IF NOT EXISTS solutionPerson (
  solutionID int(11) NOT NULL ,
  personID int(11) NOT NULL ,
  PRIMARY KEY (solutionID) ,
  INDEX ndx_solutionPerson_personID (personID ASC) ,
  INDEX ndx_solutionPerson_solutionID (solutionID ASC) ,
  CONSTRAINT fk_solutionPerson_personID
    FOREIGN KEY (personID )
    REFERENCES people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_solutionPerson_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table experimentSolutionChannel
-- -----------------------------------------------------
DROP TABLE IF EXISTS experimentSolutionChannel ;

CREATE  TABLE IF NOT EXISTS experimentSolutionChannel (
  experimentID int(11) NOT NULL ,
  solutionID int(11) NOT NULL ,
  channelID int(11) NOT NULL ,
  INDEX ndx_experimentSolutionChannel_experimentID (experimentID ASC) ,
  INDEX ndx_experimentSolutionChannel_channelID (channelID ASC) ,
  INDEX ndx_experimentSolutionChannel_solutionID (solutionID ASC) ,
  CONSTRAINT fk_experimentSolutionChannel_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES experiment (experimentID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_experimentSolutionChannel_channelID
    FOREIGN KEY (channelID )
    REFERENCES channel (channelID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT fk_experimentSolutionChannel_solutionID
    FOREIGN KEY ( solutionID )
    REFERENCES solution (solutionID)
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table rawData
-- The data field is not expected to be NULL, but loading
--  this table is a 2-step process
-- -----------------------------------------------------
DROP TABLE IF EXISTS rawData ;

CREATE  TABLE IF NOT EXISTS rawData (
  rawDataID int(11) NOT NULL AUTO_INCREMENT ,
  rawDataGUID CHAR(36) NOT NULL UNIQUE ,
  label VARCHAR(80) NOT NULL default '',
  filename VARCHAR(255) NOT NULL DEFAULT '',
  data LONGBLOB NULL ,
  comment TEXT NULL ,
  experimentID int(11) NOT NULL ,
  solutionID int(11) NOT NULL ,
  channelID int(11) NOT NULL ,
  lastUpdated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ,
  PRIMARY KEY (rawDataID) ,
  INDEX ndx_rawData_experimentID (experimentID ASC) ,
  INDEX ndx_rawData_channelID (channelID ASC) ,
  CONSTRAINT fk_rawData_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES experiment (experimentID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE,
  CONSTRAINT fk_rawData_channelID
    FOREIGN KEY (channelID )
    REFERENCES channel (channelID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table editedData
--
-- Notes:
--  rawDataID is set to allow NULL to accommodate
--   certain special records
-- -----------------------------------------------------
DROP TABLE IF EXISTS editedData ;

CREATE  TABLE IF NOT EXISTS editedData (
  editedDataID int(11) NOT NULL AUTO_INCREMENT ,
  rawDataID int(11) NULL DEFAULT NULL ,
  editGUID CHAR(36) NOT NULL UNIQUE ,
  label VARCHAR(80) NOT NULL DEFAULT '' ,
  data LONGBLOB NULL ,
  filename VARCHAR(255) NOT NULL DEFAULT '' ,
  comment TEXT NULL ,
  lastUpdated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ,
  PRIMARY KEY (editedDataID) ,
  INDEX ndx_editedData_rawDataID (rawDataID ASC) ,
  CONSTRAINT fk_editedData_rawDataID
    FOREIGN KEY (rawDataID )
    REFERENCES rawData (rawDataID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Special records for table editedData
-- -----------------------------------------------------
INSERT INTO editedData SET
  editedDataID  = 1 ,
  rawDataID     = NULL ,
  editGUID      = '00000000-0000-0000-0000-000000000000',
  label         = 'UNASSIGNED' ,
  data          = '' ,
  comment       = 'Link to this record when an editedData record is unassigned' ;

-- -----------------------------------------------------
-- Table projectPerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS projectPerson ;

CREATE  TABLE IF NOT EXISTS projectPerson (
  projectID int(11) NOT NULL ,
  personID int(11) NOT NULL ,
  INDEX ndx_projectPerson_personID  (personID ASC) ,
  INDEX ndx_projectPerson_projectID (projectID ASC) ,
  CONSTRAINT fk_projectPerson_personID
    FOREIGN KEY (personID )
    REFERENCES people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_projectPerson_projectID
    FOREIGN KEY (projectID )
    REFERENCES project (projectID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table experimentPerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS experimentPerson ;

CREATE  TABLE IF NOT EXISTS experimentPerson (
  experimentID int(11) NOT NULL ,
  personID int(11) NOT NULL ,
  INDEX ndx_experimentPerson_experimentID (experimentID ASC) ,
  INDEX ndx_experimentPerson_personID (personID ASC) ,
  CONSTRAINT fk_experimentPerson_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES experiment (experimentID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_experimentPerson_personID
    FOREIGN KEY (personID )
    REFERENCES people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table buffer
-- -----------------------------------------------------
DROP TABLE IF EXISTS buffer ;

CREATE  TABLE IF NOT EXISTS buffer (
  bufferID int(11) NOT NULL AUTO_INCREMENT ,
  bufferGUID CHAR(36) NOT NULL UNIQUE ,
  description TEXT NULL DEFAULT NULL ,
  compressibility FLOAT NULL DEFAULT NULL ,
  pH FLOAT NULL DEFAULT NULL ,
  viscosity FLOAT NULL DEFAULT NULL ,
  density FLOAT NULL DEFAULT NULL ,
  manual TINYINT(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (bufferID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table bufferComponent
-- Handle bufferComponentID in code, so they can be
--  the same on multiple databases
-- -----------------------------------------------------
DROP TABLE IF EXISTS bufferComponent ;

CREATE  TABLE IF NOT EXISTS bufferComponent (
  bufferComponentID int(11) NOT NULL UNIQUE ,
  units VARCHAR(16) NOT NULL DEFAULT 'mM', 
  description TEXT NULL DEFAULT NULL ,
  viscosity TEXT NULL DEFAULT NULL ,
  density TEXT NULL DEFAULT NULL ,
  c_range TEXT NULL DEFAULT NULL ,
  gradientForming TINYINT(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (bufferComponentID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table bufferLink
-- -----------------------------------------------------
DROP TABLE IF EXISTS bufferLink ;

CREATE  TABLE IF NOT EXISTS bufferLink (
  bufferID int(11) NOT NULL ,
  bufferComponentID int(11) NOT NULL ,
  concentration FLOAT NULL ,
  INDEX ndx_bufferLink_bufferID (bufferID ASC) ,
  INDEX ndx_bufferLink_bufferComponentID (bufferComponentID ASC) ,
  CONSTRAINT fk_bufferLink_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES buffer (bufferID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_bufferLink_bufferComponentID
    FOREIGN KEY (bufferComponentID )
    REFERENCES bufferComponent (bufferComponentID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table bufferPerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS bufferPerson ;

CREATE  TABLE IF NOT EXISTS bufferPerson (
  bufferID int(11) NOT NULL ,
  personID int(11) NOT NULL ,
  private TINYINT NOT NULL DEFAULT 1,
  PRIMARY KEY (bufferID) ,
  INDEX ndx_bufferPerson_personID (personID ASC) ,
  INDEX ndx_bufferPerson_bufferID (bufferID ASC) ,
  CONSTRAINT fk_bufferPerson_personID
    FOREIGN KEY (personID )
    REFERENCES people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_bufferPerson_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES buffer (bufferID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table analyte
-- -----------------------------------------------------
DROP TABLE IF EXISTS analyte ;

CREATE  TABLE IF NOT EXISTS analyte (
  analyteID int(11) NOT NULL AUTO_INCREMENT ,
  analyteGUID CHAR(36) NOT NULL UNIQUE ,
  type ENUM('DNA', 'RNA', 'Protein', 'Other') NULL DEFAULT 'Other' ,
  sequence TEXT NULL DEFAULT NULL ,
  vbar FLOAT NULL DEFAULT NULL ,
  description TEXT NULL DEFAULT NULL ,
  spectrum TEXT NULL DEFAULT NULL ,
  molecularWeight FLOAT NULL DEFAULT NULL ,
  gradientForming TINYINT(1) NULL DEFAULT 0,
  doubleStranded TINYINT(1) NULL DEFAULT 0,
  complement TINYINT(1) NULL DEFAULT 0,
  _3prime TINYINT(1) NULL DEFAULT 0,
  _5prime TINYINT(1) NULL DEFAULT 0,
  sodium DOUBLE NULL DEFAULT 0.0,
  potassium DOUBLE NULL DEFAULT 0.0,
  lithium DOUBLE NULL DEFAULT 0.0,
  magnesium DOUBLE NULL DEFAULT 0.0,
  calcium DOUBLE NULL DEFAULT 0.0,
  PRIMARY KEY (analyteID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table analytePerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS analytePerson ;

CREATE  TABLE IF NOT EXISTS analytePerson (
  analyteID int(11) NOT NULL ,
  personID int(11) NOT NULL ,
  PRIMARY KEY (analyteID) ,
  INDEX ndx_analytePerson_personID  (personID ASC) ,
  INDEX ndx_analytePerson_analyteID (analyteID ASC) ,
  CONSTRAINT fk_analytePerson_personID
    FOREIGN KEY (personID )
    REFERENCES people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_analytePerson_analyteID
    FOREIGN KEY (analyteID )
    REFERENCES analyte (analyteID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;

-- -----------------------------------------------------
-- Table spectrum
-- -----------------------------------------------------
DROP TABLE IF EXISTS spectrum;

CREATE TABLE IF NOT EXISTS spectrum (
  spectrumID int(11) NOT NULL AUTO_INCREMENT ,
  componentID int(11) NOT NULL ,
  componentType  enum( 'Buffer', 'Analyte' ) NOT NULL,
  opticsType enum( 'Extinction', 'Refraction', 'Fluorescence' ) NOT NULL,
  lambda FLOAT NOT NULL ,
  molarCoefficient FLOAT NOT NULL ,
  PRIMARY KEY (spectrumID) ,
  INDEX ndx_component_ID (componentID ASC) )
ENGINE = InnoDB;

-- -----------------------------------------------------
-- Table model
-- -----------------------------------------------------
DROP TABLE IF EXISTS model ;

CREATE  TABLE IF NOT EXISTS model (
  modelID int(11) NOT NULL AUTO_INCREMENT ,
  editedDataID int(11) NOT NULL DEFAULT 1,
  modelGUID CHAR(36) NOT NULL UNIQUE ,
  meniscus double NOT NULL default '0',
  MCIteration int(11) NOT NULL DEFAULT 1,
  variance double NOT NULL default 0,
  description VARCHAR(160) NULL DEFAULT NULL, -- includes 80 for runID and 80 for other
  xml LONGTEXT NULL DEFAULT NULL ,
  globalType ENUM( 'NORMAL', 'MENISCUS', 'GLOBAL', 'SUPERGLOBAL' ) DEFAULT 'NORMAL',
  lastUpdated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ,
  PRIMARY KEY (modelID) ,
  INDEX ndx_model_editedDataID (editedDataID ASC) ,
  CONSTRAINT fk_model_editDataID
    FOREIGN KEY (editedDataID )
    REFERENCES editedData (editedDataID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;

-- -----------------------------------------------------
-- Table modelPerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS modelPerson ;

CREATE  TABLE IF NOT EXISTS modelPerson (
  modelID int(11) NOT NULL ,
  personID int(11) NOT NULL ,
  PRIMARY KEY (modelID) ,
  INDEX ndx_modelPerson_personID  (personID ASC) ,
  INDEX ndx_modelPerson_modelID (modelID ASC) ,
  CONSTRAINT fk_modelPerson_personID
    FOREIGN KEY (personID )
    REFERENCES people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_modelPerson_modelID
    FOREIGN KEY (modelID )
    REFERENCES model (modelID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table noise
-- -----------------------------------------------------
DROP TABLE IF EXISTS noise ;

CREATE TABLE IF NOT EXISTS noise (
  noiseID INT(11) NOT NULL AUTO_INCREMENT ,
  noiseGUID CHAR(36) NOT NULL UNIQUE ,  -- needed?
  editedDataID int(11) NOT NULL ,
  modelID int(11) NOT NULL ,
  modelGUID CHAR(36) NULL ,
  noiseType enum('ri_noise', 'ti_noise') default 'ti_noise',
  description VARCHAR(160) NULL DEFAULT NULL, -- includes 80 for runID and 80 for other
  xml LONGTEXT ,
  timeEntered TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ,
  PRIMARY KEY ( noiseID ) ,
  INDEX ndx_noise_editedDataID (editedDataID ASC) ,
  INDEX ndx_noise_modelID (modelID ASC) ,
  CONSTRAINT fk_noise_editDataID
    FOREIGN KEY (editedDataID )
    REFERENCES editedData (editedDataID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table pcsa_modelrecs
-- -----------------------------------------------------
DROP TABLE IF EXISTS pcsa_modelrecs ;

CREATE  TABLE IF NOT EXISTS pcsa_modelrecs (
  mrecsID int(11) NOT NULL AUTO_INCREMENT ,
  editedDataID int(11) NOT NULL DEFAULT 1 ,
  modelID int(11) NOT NULL DEFAULT 0 ,
  mrecsGUID CHAR(36) NOT NULL UNIQUE ,
  description VARCHAR(160) NULL DEFAULT NULL, -- includes 80 for runID and 80 for other
  xml LONGTEXT NULL DEFAULT NULL ,
  lastUpdated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ,
  PRIMARY KEY ( mrecsID ) ,
  INDEX ndx_mrecs_editedDataID (editedDataID ASC) ,
  CONSTRAINT fk_mrecs_editDataID
    FOREIGN KEY ( editedDataID )
    REFERENCES editedData ( editedDataID )
    ON DELETE NO ACTION
    ON UPDATE CASCADE )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table structure for table HPCAnalysisRequest
-- -----------------------------------------------------
DROP TABLE IF EXISTS HPCAnalysisRequest;

CREATE TABLE IF NOT EXISTS HPCAnalysisRequest (
  HPCAnalysisRequestID int(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisRequestGUID CHAR(36) NOT NULL,
  investigatorGUID CHAR(36) NOT NULL,     -- maps to person.personGUID
  submitterGUID CHAR(36) NOT NULL,        -- maps to person.personGUID
  email VARCHAR(128) DEFAULT NULL,        -- could have multiple comma-separated
  experimentID int(11) NULL,
  requestXMLFile longtext NULL ,
  editXMLFilename varchar(255) NOT NULL default '',
  submitTime datetime NOT NULL default '0000-00-00 00:00:00',
  clusterName varchar(80) default NULL,
  method enum('2DSA','2DSA_CG','2DSA_MW','GA','GA_MW','GA_SC','DMGA','PCSA') NOT NULL default '2DSA',
  analType text DEFAULT NULL ,
  PRIMARY KEY (HPCAnalysisRequestID) )
ENGINE=InnoDB;

-- -----------------------------------------------------
-- Table structure for table HPCDataset
-- -----------------------------------------------------
DROP TABLE IF EXISTS HPCDataset;

CREATE TABLE IF NOT EXISTS HPCDataset (
  HPCDatasetID int(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisRequestID int(11) NOT NULL,
  editedDataID INT(11) DEFAULT NULL,
  simpoints int(11) default NULL,
  band_volume double default NULL,
  radial_grid tinyint(4) default NULL,
  time_grid tinyint(4) default NULL,
  rotor_stretch VARCHAR(80) NULL, 
  INDEX ndx_HPCDataset_HPCAnalysisRequestID (HPCAnalysisRequestID ASC),
  INDEX ndx_HPCDataset_editedDataID (editedDataID ASC),
  PRIMARY KEY (HPCDatasetID),
  CONSTRAINT fk_HPCDataset_HPCAnalysisRequestID
    FOREIGN KEY (HPCAnalysisRequestID)
    REFERENCES HPCAnalysisRequest (HPCAnalysisRequestID )
    ON DELETE CASCADE
    ON UPDATE NO ACTION ,
  CONSTRAINT fk_HPCDataset_editedDataID
    FOREIGN KEY (editedDataID)
    REFERENCES editedData ( editedDataID )
    ON DELETE SET NULL
    ON UPDATE NO ACTION )
ENGINE=InnoDB;

-- -----------------------------------------------------
-- Table structure for table HPCRequestData
-- -----------------------------------------------------
DROP TABLE IF EXISTS HPCRequestData;

CREATE TABLE IF NOT EXISTS HPCRequestData (
  HPCRequestDataID int(11) NOT NULL AUTO_INCREMENT,
  HPCDatasetID int(11) NOT NULL,
  noiseID int(11) DEFAULT NULL,
  INDEX ndx_HPCRequestData_HPCDatasetID (HPCDatasetID ASC) ,
  INDEX ndx_HPCRequestData_noiseID ( noiseID ASC ),
  PRIMARY KEY (HPCRequestDataID),
  CONSTRAINT fk_HPCRequestData_HPCDatasetID
    FOREIGN KEY (HPCDatasetID)
    REFERENCES HPCDataset (HPCDatasetID )
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
   CONSTRAINT fk_HPCRequestData_noiseID
     FOREIGN KEY (noiseID)
     REFERENCES noise (noiseID)
     ON DELETE SET NULL
     ON UPDATE NO ACTION)
ENGINE=InnoDB;
  

-- -----------------------------------------------------
-- Table structure for table HPCAnalysisResult
-- -----------------------------------------------------
DROP TABLE IF EXISTS HPCAnalysisResult;

CREATE TABLE IF NOT EXISTS HPCAnalysisResult (
  HPCAnalysisResultID INT(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisRequestID int(11) NOT NULL,
  startTime datetime NOT NULL default '0000-00-00 00:00:00',
  endTime datetime default NULL,          -- needed?
  queueStatus enum( 'queued','failed','running','aborted','completed') default 'queued',
  lastMessage text default NULL,          -- from nnls
  updateTime timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  gfacID TEXT DEFAULT NULL,
  jobfile TEXT,
  wallTime int(11) NOT NULL default '0',
  CPUTime double NOT NULL default '0',
  CPUCount int(11) default '0',
  mgroupcount int(11) NOT NULL DEFAULT 1,
  max_rss int(11) default '0',            -- from nnls finished message
  calculatedData TEXT,                          -- an xml file
  stderr LONGTEXT DEFAULT NULL,
  stdout LONGTEXT DEFAULT NULL,
  PRIMARY KEY (HPCAnalysisResultID),
  INDEX ndx_HPCAnalysisResult_HPCAnalysisRequestID (HPCAnalysisRequestID ASC),
  CONSTRAINT fk_HPCAnalysisResult_HPCAnalysisRequestID
    FOREIGN KEY (HPCAnalysisRequestID)
    REFERENCES HPCAnalysisRequest (HPCAnalysisRequestID)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;
  
  
-- -----------------------------------------------------
-- Table structure for table HPCAnalysisResultData
-- -----------------------------------------------------
DROP TABLE IF EXISTS HPCAnalysisResultData;

CREATE TABLE IF NOT EXISTS HPCAnalysisResultData (
  HPCAnalysisResultDataID INT(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisResultID INT(11) NOT NULL,
  HPCAnalysisResultType enum( 'model', 'noise', 'job_stats', 'mrecs', 'unknown' ) NOT NULL default 'unknown',
  resultID INT(11) NOT NULL,          -- could be a modelID,noiseID,mrecsID
  PRIMARY KEY (HPCAnalysisResultDataID),
  INDEX ndx_HPCAnalysisResultData_HPCAnalysisResultID (HPCAnalysisResultID ASC),
  CONSTRAINT fk_HPCAnalysisResultData_HPCAnalysisResultID
    FOREIGN KEY (HPCAnalysisResultID)
    REFERENCES HPCAnalysisResult (HPCAnalysisResultID)
    ON DELETE CASCADE
    ON UPDATE NO ACTION )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table structure for table 2DSA_Settings
-- -----------------------------------------------------
DROP TABLE IF EXISTS 2DSA_Settings;

CREATE TABLE IF NOT EXISTS 2DSA_Settings (
  2DSA_SettingsID int(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisRequestID int(11) NOT NULL,
  s_min double NOT NULL default '1',
  s_max double NOT NULL default '10',
  s_resolution double NOT NULL default '10',
  ff0_min double NOT NULL default '1',
  ff0_max double NOT NULL default '4',
  ff0_resolution double NOT NULL default '10',
  uniform_grid int(11) NOT NULL default '6',
  mc_iterations int(11) NOT NULL default 1,
  tinoise_option tinyint(1) NOT NULL default '0',
  regularization int(11) NOT NULL default '0',
  meniscus_range double NOT NULL default '0.01',
  meniscus_points double NOT NULL default '3',
  max_iterations int(11) NOT NULL default 1,
  rinoise_option tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (2DSA_SettingsID),
  INDEX ndx_2DSA_Settings_HPCAnalysisRequestID (HPCAnalysisRequestID ASC),
  CONSTRAINT fk_2DSA_Settings_HPCAnalysisRequestID
    FOREIGN KEY (HPCAnalysisRequestID)
    REFERENCES HPCAnalysisRequest (HPCAnalysisRequestID)
    ON DELETE CASCADE
    ON UPDATE NO ACTION )
ENGINE=InnoDB;


-- -----------------------------------------------------
-- Table structure for table 2DSA_CG_Settings
-- -----------------------------------------------------
DROP TABLE IF EXISTS 2DSA_CG_Settings;

CREATE TABLE IF NOT EXISTS 2DSA_CG_Settings (
  2DSA_CG_SettingsID int(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisRequestID int(11) NOT NULL,
  CG_modelID int(11) NOT NULL,    -- maps to model with 'CustomGrid...' description
  uniform_grid int(11) NOT NULL default '6',
  mc_iterations int(11) NOT NULL default 1,
  tinoise_option tinyint(1) NOT NULL default '0',
  regularization int(11) NOT NULL default '0',
  meniscus_range double NOT NULL default '0.01',
  meniscus_points double NOT NULL default '3',
  max_iterations int(11) NOT NULL default 1,
  rinoise_option tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (2DSA_CG_SettingsID),
  INDEX ndx_2DSA_CG_Settings_HPCAnalysisRequestID (HPCAnalysisRequestID ASC),
  CONSTRAINT fk_2DSA_CG_Settings_HPCAnalysisRequestID
    FOREIGN KEY (HPCAnalysisRequestID)
    REFERENCES HPCAnalysisRequest (HPCAnalysisRequestID)
    ON DELETE CASCADE
    ON UPDATE NO ACTION )
ENGINE=InnoDB;


-- -----------------------------------------------------
-- Table structure for table 2DSA_MW_Settings
-- -----------------------------------------------------
DROP TABLE IF EXISTS 2DSA_MW_Settings;

CREATE TABLE IF NOT EXISTS 2DSA_MW_Settings (
  2DSA_MW_SettingsID int(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisRequestID int(11) NOT NULL,
  mw_min double NOT NULL default '100',
  mw_max double NOT NULL default '1000',
  grid_resolution int(11) NOT NULL default '10',
  oligomer int(11) NOT NULL default '4',
  ff0_min double NOT NULL default '1',
  ff0_max double NOT NULL default '4',
  ff0_resolution int(11) NOT NULL default '10',
  uniform_grid int(11) NOT NULL default '6',
  montecarlo_value int(11) NOT NULL default '0',
  tinoise_option tinyint(1) NOT NULL default '0',
  regularization int(11) NOT NULL default '0',
  meniscus_value double NOT NULL default '0.01',
  meniscus_points int(11) NOT NULL default '3',
  iterations_value int(11) NOT NULL default '3',
  rinoise_option tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (2DSA_MW_SettingsID),
  INDEX ndx_2DSA_MW_Settings_HPCAnalysisRequestID (HPCAnalysisRequestID ASC),
  CONSTRAINT fk_2DSA_MW_Settings_HPCAnalysisRequestID
    FOREIGN KEY (HPCAnalysisRequestID)
    REFERENCES HPCAnalysisRequest (HPCAnalysisRequestID)
    ON DELETE CASCADE
    ON UPDATE NO ACTION )
ENGINE=InnoDB;


-- -----------------------------------------------------
-- Table structure for table GA_Settings
-- -----------------------------------------------------
DROP TABLE IF EXISTS GA_Settings;

CREATE TABLE IF NOT EXISTS GA_Settings (
  GA_SettingsID int(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisRequestID int(11) NOT NULL,
  montecarlo_value int(11) NOT NULL default '0',
  demes_value int(11) NOT NULL default '31',
  genes_value int(11) NOT NULL default '100',
  generations_value int(11) NOT NULL default '100',
  crossover_value int(11) NOT NULL default '50',
  mutation_value int(11) NOT NULL default '50',
  plague_value int(11) NOT NULL default '4',
  elitism_value int(11) NOT NULL default '2',
  migration_value int(11) NOT NULL default '3',
  regularization_value double NOT NULL default '5',
  seed_value int(11) NOT NULL default '0',
  conc_threshold FLOAT NOT NULL DEFAULT 1E-6,
  s_grid INT(11) NOT NULL DEFAULT 100,
  k_grid INT(11) NOT NULL DEFAULT 100,
  mutate_sigma FLOAT NOT NULL DEFAULT 2,
  mutate_s INT(11) NOT NULL DEFAULT 20,
  mutate_k INT(11) NOT NULL DEFAULT 20,
  mutate_sk INT(11) NOT NULL DEFAULT 20,
  PRIMARY KEY  (GA_SettingsID),
  INDEX ndx_GA_Settings_HPCAnalysisRequestID (HPCAnalysisRequestID ASC),
  CONSTRAINT fk_GA_Settings_HPCAnalysisRequestID
    FOREIGN KEY (HPCAnalysisRequestID)
    REFERENCES HPCAnalysisRequest (HPCAnalysisRequestID)
    ON DELETE CASCADE
    ON UPDATE NO ACTION )
ENGINE=InnoDB;


-- -----------------------------------------------------
-- Table structure for table GA_MW_Settings
-- -----------------------------------------------------
DROP TABLE IF EXISTS GA_MW_Settings;

CREATE TABLE IF NOT EXISTS GA_MW_Settings (
  GA_MW_SettingsID int(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisRequestID int(11) NOT NULL,
  mw_min double NOT NULL default '100',
  mw_max double NOT NULL default '1000',
  oligomer int(11) NOT NULL default '4',
  ff0_min double NOT NULL default '1',
  ff0_max double NOT NULL default '4',
  montecarlo_value int(11) NOT NULL default '0',
  tinoise_option tinyint(1) NOT NULL default '0',
  demes_value int(11) NOT NULL default '31',
  genes_value int(11) NOT NULL default '100',
  generations_value int(11) NOT NULL default '100',
  crossover_value int(11) NOT NULL default '50',
  mutation_value int(11) NOT NULL default '50',
  plague_value int(11) NOT NULL default '4',
  elitism_value int(11) NOT NULL default '2',
  migration_value int(11) NOT NULL default '3',
  regularization_value double NOT NULL default '5',
  seed_value int(11) NOT NULL default '0',
  rinoise_option tinyint(1) NOT NULL default '0',
  meniscus_value double NOT NULL default '0',
  PRIMARY KEY  (GA_MW_SettingsID),
  INDEX ndx_GA_MW_Settings_HPCAnalysisRequestID (HPCAnalysisRequestID ASC),
  CONSTRAINT fk_GA_MW_Settings_HPCAnalysisRequestID
    FOREIGN KEY (HPCAnalysisRequestID)
    REFERENCES HPCAnalysisRequest (HPCAnalysisRequestID)
    ON DELETE CASCADE
    ON UPDATE NO ACTION )
ENGINE=InnoDB;


-- -----------------------------------------------------
-- Table structure for table GA_SC_Settings
-- -----------------------------------------------------
DROP TABLE IF EXISTS GA_SC_Settings;

CREATE TABLE IF NOT EXISTS GA_SC_Settings (
  GA_SC_SettingsID int(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisRequestID int(11) NOT NULL,
  montecarlo_value int(11) NOT NULL default '0',
  demes_value int(11) NOT NULL default '31',
  genes_value int(11) NOT NULL default '100',
  generations_value int(11) NOT NULL default '100',
  crossover_value int(11) NOT NULL default '50',
  mutation_value int(11) NOT NULL default '50',
  plague_value int(11) NOT NULL default '4',
  elitism_value int(11) NOT NULL default '2',
  migration_value int(11) NOT NULL default '3',
  regularization_value double NOT NULL default '5',
  seed_value int(11) NOT NULL default '0',
  tinoise_option tinyint(1) NOT NULL default '0',
  rinoise_option tinyint(1) NOT NULL default '0',
  meniscus_option tinyint(1) NOT NULL default '0',
  meniscus_value double NOT NULL default '0',
  constraint_data mediumtext,
  PRIMARY KEY  (GA_SC_SettingsID),
  INDEX ndx_GA_SC_Settings_HPCAnalysisRequestID (HPCAnalysisRequestID ASC),
  CONSTRAINT fk_GA_SC_Settings_HPCAnalysisRequestID
    FOREIGN KEY (HPCAnalysisRequestID)
    REFERENCES HPCAnalysisRequest (HPCAnalysisRequestID)
    ON DELETE CASCADE
    ON UPDATE NO ACTION )
ENGINE=InnoDB;


-- -----------------------------------------------------
-- Table structure for table DMGA_Settings
-- -----------------------------------------------------
DROP TABLE IF EXISTS DMGA_Settings;

CREATE TABLE IF NOT EXISTS DMGA_Settings (
  DMGA_SettingsID int(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisRequestID int(11) NOT NULL,
  DC_modelID int(11) NOT NULL,    -- maps to model with 'DMGA_Constr..' description
  mc_iterations int(11) NOT NULL default '0',
  demes int(11) NOT NULL default '31',
  population int(11) NOT NULL default '100',
  generations int(11) NOT NULL default '100',
  mutation int(11) NOT NULL default '50',
  crossover int(11) NOT NULL default '50',
  plague int(11) NOT NULL default '4',
  elitism int(11) NOT NULL default '2',
  migration int(11) NOT NULL default '3',
  p_grid int(11) NOT NULL default '500',
  seed int(11) NOT NULL default '0',
  PRIMARY KEY  (DMGA_SettingsID),
  INDEX ndx_DMGA_Settings_HPCAnalysisRequestID (HPCAnalysisRequestID ASC),
  CONSTRAINT fk_DMGA_Settings_HPCAnalysisRequestID
    FOREIGN KEY (HPCAnalysisRequestID)
    REFERENCES HPCAnalysisRequest (HPCAnalysisRequestID)
    ON DELETE CASCADE
    ON UPDATE NO ACTION )
ENGINE=InnoDB;


-- -----------------------------------------------------
-- Table structure for table PCSA_Settings
-- -----------------------------------------------------
DROP TABLE IF EXISTS PCSA_Settings;

CREATE TABLE IF NOT EXISTS PCSA_Settings (
  PCSA_SettingsID int(11) NOT NULL AUTO_INCREMENT,
  HPCAnalysisRequestID int(11) NOT NULL,
  curve_type VARCHAR(8) NOT NULL default 'SL',
  s_min double NOT NULL default '1',
  s_max double NOT NULL default '10',
  ff0_min double NOT NULL default '1',
  ff0_max double NOT NULL default '4',
  vars_count int(11) NOT NULL default 10,
  gfit_iterations int(11) NOT NULL default 3,
  curves_points int(11) NOT NULL default 200,
  thr_deltr_ratio double NOT NULL default 0.0001,
  tikreg_option tinyint(1) NOT NULL default 0,
  tikreg_alpha double NOT NULL default 0.0,
  mc_iterations int(11) NOT NULL default 1,
  tinoise_option tinyint(1) NOT NULL default '0',
  rinoise_option tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (PCSA_SettingsID),
  INDEX ndx_PCSA_Settings_HPCAnalysisRequestID (HPCAnalysisRequestID ASC),
  CONSTRAINT fk_PCSA_Settings_HPCAnalysisRequestID
    FOREIGN KEY (HPCAnalysisRequestID)
    REFERENCES HPCAnalysisRequest (HPCAnalysisRequestID)
    ON DELETE CASCADE
    ON UPDATE NO ACTION )
ENGINE=InnoDB;


-- -----------------------------------------------------
-- Table structure for table HPCSoluteData
-- -----------------------------------------------------
DROP TABLE IF EXISTS HPCSoluteData;

CREATE TABLE IF NOT EXISTS HPCSoluteData (
  HPCSoluteDataID int(11) NOT NULL AUTO_INCREMENT,
  GA_SettingsID int(11) NOT NULL,
  s_min double NOT NULL default '0',
  s_max double NOT NULL default '0',
  ff0_min double NOT NULL default '0',
  ff0_max double NOT NULL default '0',
  PRIMARY KEY  (HPCSoluteDataID),
  INDEX ndx_HPCSoluteData_GA_SettingsID (GA_SettingsID ASC),
  CONSTRAINT fk_HPCSoluteData_GA_SettingsID
    FOREIGN KEY (GA_SettingsID)
    REFERENCES GA_Settings (GA_SettingsID)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE=InnoDB;


-- -----------------------------------------------------
-- Table image
-- -----------------------------------------------------
DROP TABLE IF EXISTS image ;

CREATE  TABLE IF NOT EXISTS image (
  imageID int(11) NOT NULL AUTO_INCREMENT ,
  imageGUID CHAR(36) NULL ,
  description VARCHAR(80) NOT NULL DEFAULT 'No description was entered for this image' ,
  gelPicture LONGBLOB NOT NULL ,
  filename VARCHAR(255) NOT NULL DEFAULT '',
  date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ,
  PRIMARY KEY (imageID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table imagePerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS imagePerson ;

CREATE  TABLE IF NOT EXISTS imagePerson (
  imageID int(11) NOT NULL ,
  personID int(11) NOT NULL ,
  INDEX ndx_imagePerson_personID (personID ASC) ,
  INDEX ndx_imagePerson_imageID (imageID ASC) ,
  CONSTRAINT fk_imagePerson_personID
    FOREIGN KEY (personID )
    REFERENCES people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_imagePerson_imageID
    FOREIGN KEY (imageID )
    REFERENCES image (imageID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table imageBuffer
-- -----------------------------------------------------
DROP TABLE IF EXISTS imageBuffer ;

CREATE  TABLE IF NOT EXISTS imageBuffer (
  imageID int(11) NOT NULL ,
  bufferID int(11) NOT NULL ,
  PRIMARY KEY (imageID) ,
  INDEX ndx_imageBuffer_imageID   (imageID ASC) ,
  INDEX ndx_imageBuffer_bufferID (bufferID ASC) ,
  CONSTRAINT fk_imageBuffer_imageID
    FOREIGN KEY (imageID )
    REFERENCES image (imageID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_imageBuffer_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES buffer (bufferID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table imageAnalyte
-- -----------------------------------------------------
DROP TABLE IF EXISTS imageAnalyte ;

CREATE  TABLE IF NOT EXISTS imageAnalyte (
  imageID int(11) NOT NULL ,
  analyteID int(11) NOT NULL ,
  PRIMARY KEY (imageID) ,
  INDEX ndx_imageAnalyte_imageID   (imageID ASC) ,
  INDEX ndx_imageAnalyte_analyteID (analyteID ASC) ,
  CONSTRAINT fk_imageAnalyte_imageID
    FOREIGN KEY (imageID )
    REFERENCES image (imageID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_imageAnalyte_analyteID
    FOREIGN KEY (analyteID )
    REFERENCES analyte (analyteID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table imageSolution
-- -----------------------------------------------------
DROP TABLE IF EXISTS imageSolution ;

CREATE  TABLE IF NOT EXISTS imageSolution (
  imageID int(11) NOT NULL ,
  solutionID int(11) NOT NULL ,
  PRIMARY KEY (imageID) ,
  INDEX ndx_imageSolution_imageID    (imageID ASC) ,
  INDEX ndx_imageSolution_solutionID (solutionID ASC) ,
  CONSTRAINT fk_imageSolution_imageID
    FOREIGN KEY (imageID )
    REFERENCES image (imageID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_imageSolution_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table avivFluorescence
-- -----------------------------------------------------
DROP TABLE IF EXISTS avivFluorescence ;

CREATE  TABLE IF NOT EXISTS avivFluorescence (
  avivFluorescenceID int(11) NOT NULL AUTO_INCREMENT ,
  opticalSystemSettingID int(11) NULL ,
  topRadius FLOAT NULL ,
  bottomRadius FLOAT NULL ,
  mmStepSize FLOAT NULL ,
  replicates INT NULL ,
  nmExcitation FLOAT NULL ,
  nmEmission FLOAT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (avivFluorescenceID) ,
  INDEX ndx_avivFluorescence_opticalSystemSettingID 
    (opticalSystemSettingID ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table beckmanRadialAbsorbance
-- -----------------------------------------------------
DROP TABLE IF EXISTS beckmanRadialAbsorbance ;

CREATE  TABLE IF NOT EXISTS beckmanRadialAbsorbance (
  beckmanRadialAbsorbanceID int(11) NOT NULL AUTO_INCREMENT ,
  opticalSystemSettingID int(11) NULL ,
  topRadius FLOAT NULL ,
  bottomRadius FLOAT NULL ,
  mmStepSize FLOAT NULL ,
  replicates INT NULL ,
  isContinuousMode TINYINT(1) NULL ,
  isIntensity TINYINT(1) NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (beckmanRadialAbsorbanceID) ,
  INDEX ndx_beckmanRadialAbsorbance_opticalSystemSettingID 
    (opticalSystemSettingID ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table beckmanWavelengthAbsorbance
-- -----------------------------------------------------
DROP TABLE IF EXISTS beckmanWavelengthAbsorbance ;

CREATE  TABLE IF NOT EXISTS beckmanWavelengthAbsorbance (
  beckmanWavelengthAbsorbanceID int(11) NOT NULL AUTO_INCREMENT ,
  opticalSystemSettingID int(11) NULL ,
  radialPosition FLOAT NULL ,
  startWavelength FLOAT NULL ,
  endWavelength FLOAT NULL ,
  nmStepsize FLOAT NULL ,
  replicates INT NULL ,
  isIntensity TINYINT(1) NULL ,
  secondsBetween INT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (beckmanWavelengthAbsorbanceID) ,
  INDEX ndx_beckmanWavelengthAbsorbance_opticalSystemSettingID 
    (opticalSystemSettingID ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table beckmanInterference
-- -----------------------------------------------------
DROP TABLE IF EXISTS beckmanInterference ;

CREATE  TABLE IF NOT EXISTS beckmanInterference (
  beckmanInterferenceID int(11) NOT NULL AUTO_INCREMENT ,
  opticalSystemSettingID int(11) NULL ,
  topRadius FLOAT NULL ,
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
-- Table multiWavelengthSystem
-- -----------------------------------------------------
DROP TABLE IF EXISTS multiWavelengthSystem ;

CREATE  TABLE IF NOT EXISTS multiWavelengthSystem (
  multiWavelengthSystemID int(11) NOT NULL AUTO_INCREMENT ,
  opticalSystemSettingID int(11) NULL ,
  startWavelength FLOAT NULL ,
  endWavelength FLOAT NULL ,
  nmStepsize FLOAT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (multiWavelengthSystemID) ,
  INDEX ndx_multiWavelengthSystem_opticalSystemSettingID 
    (opticalSystemSettingID ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table otherOpticalSystem
-- -----------------------------------------------------
DROP TABLE IF EXISTS otherOpticalSystem ;

CREATE  TABLE IF NOT EXISTS otherOpticalSystem (
  otherOpticalSystemID int(11) NOT NULL AUTO_INCREMENT ,
  opticalSystemSettingID int(11) NULL ,
  name TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (otherOpticalSystemID) ,
  INDEX ndx_otherOpticalSystem_opticalSystemSettingID 
    (opticalSystemSettingID ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table opticalSystemSetting
-- -----------------------------------------------------
DROP TABLE IF EXISTS opticalSystemSetting ;

CREATE  TABLE IF NOT EXISTS opticalSystemSetting (
  opticalSystemSettingID int(11) NOT NULL AUTO_INCREMENT ,
  rawDataID int(11) NULL ,
  opticalSystemSettingGUID CHAR(36) NULL ,
  name TEXT NULL ,
  value FLOAT NULL ,
  dateUpdated TIMESTAMP NULL ,
  secondsDuration INT NULL ,
  hwType ENUM( 'fluor', 'wlAbsorb', 'interfere', 'radialAbs', 'MWL', 'other') NULL ,
  hwIndex INT NULL ,
  channelID int(11) NULL ,
  PRIMARY KEY (opticalSystemSettingID) ,
  INDEX ndx_opticalSystemSetting_rawDataID (rawDataID ASC) ,
  INDEX ndx_opticalSystemSetting_channelID (channelID ASC) ,
  INDEX ndx_opticalSystemSetting_fluorescenceOpSysID 
    (opticalSystemSettingID ASC) ,
  CONSTRAINT fk_opticalSystemSetting_rawDataID
    FOREIGN KEY (rawDataID )
    REFERENCES rawData (rawDataID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT fk_opticalSystemSetting_channelID
    FOREIGN KEY (channelID )
    REFERENCES channel (channelID )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT fk_opticalSystemSetting_fluorescenceOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES avivFluorescence (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_opticalSystemSetting_radialOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES beckmanRadialAbsorbance (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_opticalSystemSetting_wavelengthOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES beckmanWavelengthAbsorbance (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_opticalSystemSetting_interferenceOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES beckmanInterference (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_opticalSystemSetting_MWLOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES multiWavelengthSystem (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_opticalSystemSetting_otherOpSysID
    FOREIGN KEY (opticalSystemSettingID )
    REFERENCES otherOpticalSystem (opticalSystemSettingID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table permits
-- -----------------------------------------------------
DROP TABLE IF EXISTS permits ;

CREATE  TABLE IF NOT EXISTS permits (
  permitID int(11) NOT NULL AUTO_INCREMENT ,
  personID int(11) NOT NULL ,
  collaboratorID int(11) default NULL ,
  instrumentID int(11) default NULL ,
  PRIMARY KEY (permitID) ,
  INDEX ndx_permits_personID (personID ASC) ,
  INDEX ndx_permits_collaboratorID (collaboratorID ASC) ,
  INDEX ndx_permits_instrumentID (instrumentID ASC) ,
  CONSTRAINT fk_permits_personID
    FOREIGN KEY (personID )
    REFERENCES people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_permits_collaboratorID
    FOREIGN KEY (collaboratorID )
    REFERENCES people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table report
-- -----------------------------------------------------
DROP TABLE IF EXISTS report ;

CREATE  TABLE IF NOT EXISTS report (
  reportID int(11) NOT NULL AUTO_INCREMENT,
  reportGUID char(36) NOT NULL UNIQUE,
  experimentID int(11) NOT NULL,
  runID varchar(80) NOT NULL,
  title varchar(255) NOT NULL DEFAULT '',
  html longtext,
  PRIMARY KEY (reportID),
  INDEX ndx_report_experimentID (experimentID),
  CONSTRAINT fk_report_experimentID 
    FOREIGN KEY (experimentID) 
    REFERENCES experiment (experimentID) 
    ON DELETE CASCADE 
    ON UPDATE CASCADE
) ENGINE=InnoDB;


-- -----------------------------------------------------
-- Table reportPerson
-- -----------------------------------------------------
DROP TABLE IF EXISTS reportPerson ;

CREATE  TABLE IF NOT EXISTS reportPerson (
  reportID int(11) NOT NULL,
  personID int(11) NOT NULL,
  PRIMARY KEY (reportID),
  INDEX ndx_reportPerson_personID (personID),
  INDEX ndx_reportPerson_reportID (reportID),
  CONSTRAINT fk_reportPerson_personID 
    FOREIGN KEY (personID) 
    REFERENCES people (personID) 
    ON DELETE CASCADE 
    ON UPDATE CASCADE,
  CONSTRAINT fk_reportPerson_reportID 
    FOREIGN KEY (reportID) 
    REFERENCES report (reportID) 
    ON DELETE CASCADE 
    ON UPDATE CASCADE
) ENGINE=InnoDB;


-- -----------------------------------------------------
-- Table reportTriple
-- -----------------------------------------------------
DROP TABLE IF EXISTS reportTriple ;

CREATE  TABLE IF NOT EXISTS reportTriple (
  reportTripleID int(11) NOT NULL AUTO_INCREMENT,
  reportTripleGUID char(36) NOT NULL UNIQUE,
  reportID int(11) NOT NULL,
  resultID int(11) DEFAULT NULL,
  triple varchar(20) NOT NULL DEFAULT '',     -- cell/channel/wavelength format
  dataDescription varchar(255) NOT NULL DEFAULT '', -- from 1st line in data file
  PRIMARY KEY (reportTripleID),
  INDEX ndx_reportTriple_reportID (reportID),
  CONSTRAINT fk_reportTriple_reportID 
    FOREIGN KEY (reportID) 
    REFERENCES report (reportID) 
    ON DELETE CASCADE 
    ON UPDATE CASCADE
) ENGINE=InnoDB;


-- -----------------------------------------------------
-- Table documentLink
-- -----------------------------------------------------
DROP TABLE IF EXISTS documentLink ;

CREATE  TABLE IF NOT EXISTS documentLink (
  documentLinkID int(11) NOT NULL AUTO_INCREMENT,
  reportTripleID int(11) NOT NULL,
  reportDocumentID int(11) NOT NULL,
  PRIMARY KEY (documentLinkID),
  INDEX ndx_documentLink_reportTripleID (reportTripleID),
  INDEX ndx_documentLink_reportDocumentID (reportDocumentID),
  CONSTRAINT fk_documentLink_reportDocumentID 
    FOREIGN KEY (reportDocumentID) 
    REFERENCES reportDocument (reportDocumentID) 
    ON DELETE CASCADE 
    ON UPDATE CASCADE,
  CONSTRAINT fk_documentLink_reportTripleID 
    FOREIGN KEY (reportTripleID) 
    REFERENCES reportTriple (reportTripleID) 
    ON DELETE CASCADE 
    ON UPDATE CASCADE
) ENGINE=InnoDB;


-- -----------------------------------------------------
-- Table reportDocument
-- -----------------------------------------------------
DROP TABLE IF EXISTS reportDocument ;

CREATE  TABLE IF NOT EXISTS reportDocument (
  reportDocumentID int(11) NOT NULL AUTO_INCREMENT,
  reportDocumentGUID char(36) NOT NULL UNIQUE,
  editedDataID int(11) NOT NULL DEFAULT '1',
  label varchar(160) NOT NULL DEFAULT '',
  filename varchar(255) NOT NULL DEFAULT '',
  analysis varchar(20) DEFAULT '2DSA',
  subAnalysis varchar(20) DEFAULT 'report',
  documentType varchar(20) DEFAULT 'png',
  contents longblob,
  PRIMARY KEY (reportDocumentID),
  INDEX ndx_reportDocument_editedDataID (editedDataID),
  CONSTRAINT fk_reportDocument_editedDataID 
    FOREIGN KEY (editedDataID) 
    REFERENCES editedData (editedDataID) 
    ON UPDATE NO ACTION
) ENGINE=InnoDB;

-- -----------------------------------------------------
-- Table speedstep
-- -----------------------------------------------------
DROP TABLE IF EXISTS speedstep ;

CREATE  TABLE IF NOT EXISTS speedstep (
  speedstepID int(11) NOT NULL AUTO_INCREMENT ,
  experimentID int(11) NOT NULL ,
  scans int(11) NOT NULL ,
  durationhrs int(11) NOT NULL ,
  durationmins double NOT NULL ,
  delayhrs int(11) NOT NULL ,
  delaymins double NOT NULL ,
  rotorspeed int(11) NOT NULL ,
  acceleration int(11) NOT NULL ,
  accelerflag tinyint(1) DEFAULT 1 ,
  w2tfirst float NOT NULL ,
  w2tlast float NOT NULL ,
  timefirst int(11) NOT NULL ,
  timelast int(11) NOT NULL ,
  setspeed int(11) NOT NULL ,
  avgspeed float NOT NULL ,
  speedsdev float NOT NULL ,
  PRIMARY KEY (speedstepID) ,
  CONSTRAINT fk_speedstep_experimentID
    FOREIGN KEY (experimentID)
    REFERENCES experiment (experimentID)
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;

-- -----------------------------------------------------
-- Table timestate
-- -----------------------------------------------------
DROP TABLE IF EXISTS timestate ;

CREATE  TABLE IF NOT EXISTS timestate (
  timestateID int(11) NOT NULL AUTO_INCREMENT ,
  experimentID int(11) NOT NULL ,
  filename varchar(255) NOT NULL default '',
  definitions longtext,
  data longblob,
  lastUpdated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP ,
  PRIMARY KEY (timestateID) ,
  CONSTRAINT fk_timestate_experimentID
    FOREIGN KEY (experimentID)
    REFERENCES experiment (experimentID)
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


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
  aprofileGUID char(36) NULL ,
  PRIMARY KEY (protocolID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table protocolPerson
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


-- -----------------------------------------------------
-- Table analysisprofile
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
-- Table protocolAprofile
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
-- Table experimentProtocol
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


-- Load some non-changing hardware data
SOURCE us3_hardware_data.sql
SOURCE us3_buffer_components.sql

SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;

