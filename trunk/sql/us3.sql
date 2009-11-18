SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL';

CREATE SCHEMA IF NOT EXISTS us3 DEFAULT CHARACTER SET ascii COLLATE ascii_bin ;
USE us3;

-- -----------------------------------------------------
-- Table us3.people
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.people (
  personID INT NOT NULL AUTO_INCREMENT ,
  fname VARCHAR(30) NULL ,
  lname VARCHAR(30) NULL ,
  address VARCHAR(255) NULL ,
  city VARCHAR(30) NULL ,
  state CHAR(2) NULL ,
  zip VARCHAR(10) NULL ,
  phone VARCHAR(24) NULL ,
  email VARCHAR(63) NOT NULL ,
  organization VARCHAR(45) NULL ,
  username VARCHAR(80) NOT NULL ,
  password VARCHAR(80) NULL ,
  activated BOOLEAN NOT NULL DEFAULT false ,
  signup TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ,
  lastLogin DATETIME NULL ,
  clusterAuthorizations varchar(255) NOT NULL default 'bcf:alamo:laredo:lonestar:bigred:steele:queenbee',
  userlevel TINYINT NOT NULL DEFAULT 0 ,
  PRIMARY KEY (personID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.project
-- -----------------------------------------------------
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
CREATE  TABLE IF NOT EXISTS us3.lab (
  labID INT NOT NULL AUTO_INCREMENT ,
  name TEXT NULL ,
  building TEXT NULL ,
  room TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (labID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.instrument
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.instrument (
  instrumentID INT NOT NULL AUTO_INCREMENT ,
  name TEXT NULL ,
  serialNumber TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (instrumentID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.abstractRotor
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.abstractRotor (
  abstractRotorID INT NOT NULL AUTO_INCREMENT ,
  name TEXT NULL ,
  materialName TEXT NULL ,
  numHoles INT NULL ,
  maxRPM INT NULL ,
  manufacturer TEXT NULL ,
  materialRefURI TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (abstractRotorID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.rotor
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.rotor (
  rotorID INT NOT NULL AUTO_INCREMENT ,
  abstractRotorID INT NULL ,
  name TEXT NULL ,
  serialNumber TEXT NULL ,
  stretchFunction TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (rotorID) ,
  INDEX ndx_rotor_abstractRotorID (abstractRotorID ASC) ,
  CONSTRAINT rotor_abstractRotorID
    FOREIGN KEY (abstractRotorID )
    REFERENCES us3.abstractRotor (abstractRotorID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.experiment
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.experiment (
  experimentID INT NOT NULL AUTO_INCREMENT ,
  projectID INT NULL ,
  labID INT NULL ,
  instrumentID INT NULL ,
  operatorID INT NULL ,
  rotorID INT NULL ,
  type ENUM('velocity', 'equilibrium', 'other') NULL ,
  date DATE NOT NULL ,
  runTemp FLOAT NULL ,
  label VARCHAR(80) NULL ,
  comment TEXT NULL ,
  PRIMARY KEY (experimentID) ,
  INDEX ndx_experiment_projectID (projectID ASC) ,
  INDEX ndx_experiment_labID (labID ASC) ,
  INDEX ndx_experiment_instrumentID (instrumentID ASC) ,
  INDEX ndx_experiment_operatorID (operatorID ASC) ,
  INDEX ndx_experiment_rotorID (rotorID ASC) ,
  CONSTRAINT experiment_projectID
    FOREIGN KEY (projectID )
    REFERENCES us3.project (projectID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT experiment_labID
    FOREIGN KEY (labID )
    REFERENCES us3.lab (labID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT experiment_instrumentID
    FOREIGN KEY (instrumentID )
    REFERENCES us3.instrument (instrumentID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT experiment_operatorID
    FOREIGN KEY (operatorID )
    REFERENCES us3.people (personID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT experiment_rotorID
    FOREIGN KEY (rotorID )
    REFERENCES us3.rotor (rotorID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.solution
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.solution (
  solutionID INT NOT NULL AUTO_INCREMENT ,
  description VARCHAR(80) NOT NULL ,
  storageTemp TINYINT NULL ,
  notes TEXT NULL ,
  PRIMARY KEY (solutionID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.image
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.image (
  imageID INT NOT NULL AUTO_INCREMENT ,
  description VARCHAR(80) NOT NULL DEFAULT 'No description was entered for this image' ,
  gelPicture LONGBLOB NOT NULL ,
  date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ,
  PRIMARY KEY (imageID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.channelType
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.channelType (
  channelTypeID INT NOT NULL AUTO_INCREMENT ,
  description TEXT NULL ,
  PRIMARY KEY (channelTypeID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.channelShape
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.channelShape (
  channelShapeID INT NOT NULL AUTO_INCREMENT ,
  description TEXT NULL ,
  PRIMARY KEY (channelShapeID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.loadMethod
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.loadMethod (
  loadMethodID INT NOT NULL AUTO_INCREMENT ,
  name TEXT NULL ,
  PRIMARY KEY (loadMethodID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.abstractCenterpiece
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.abstractCenterpiece (
  abstractCenterpieceID INT NOT NULL AUTO_INCREMENT ,
  loadMethodID INT NULL ,
  name TEXT NULL ,
  materialName TEXT NULL ,
  maxRPM INT NULL ,
  MmThick FLOAT NULL ,
  canHoldSample INT NULL ,
  materialRefURI TEXT NULL ,
  centerpieceRefURI TEXT NULL ,
  dataUpdated TIMESTAMP NULL ,
  PRIMARY KEY (abstractCenterpieceID) ,
  INDEX ndx_abstractCenterpiece_loadMethodID (loadMethodID ASC) ,
  CONSTRAINT abstractCenterpiece_loadMethodID
    FOREIGN KEY (loadMethodID )
    REFERENCES us3.loadMethod (loadMethodID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.abstractChannel
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.abstractChannel (
  AbstractChannel_ID INT NOT NULL AUTO_INCREMENT ,
  ChannelTypeID INT NULL ,
  ChannelShapeID INT NULL ,
  AbstractCenterpieceID INT NULL ,
  PRIMARY KEY (AbstractChannel_ID) ,
  INDEX ndx_abstractChannel_ChannelTypeID (ChannelTypeID ASC) ,
  INDEX ndx_abstractChannel_ChannelShapeID (ChannelShapeID ASC) ,
  INDEX ndx_abstractChannel_AbstractCenterpieceID (AbstractChannel_ID ASC) ,
  CONSTRAINT abstractChannel_ChannelTypeID
    FOREIGN KEY (ChannelTypeID )
    REFERENCES us3.channelType (channelTypeID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT abstractChannel_ChannelShapeID
    FOREIGN KEY (ChannelShapeID )
    REFERENCES us3.channelShape (channelShapeID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT abstractChannel_AbstractCenterpieceID
    FOREIGN KEY (AbstractCenterpieceID )
    REFERENCES us3.abstractCenterpiece (abstractCenterpieceID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.channel
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.channel (
  channel_ID INT NOT NULL AUTO_INCREMENT ,
  solutionID INT NULL ,
  abstractChannel_ID INT NULL ,
  cell_ID INT NULL ,
  comments TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (channel_ID) ,
  INDEX ndx_channel_solutionID (solutionID ASC) ,
  INDEX ndx_channel_abstractChannel_ID (abstractChannel_ID ASC) ,
  CONSTRAINT channel_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES us3.solution (solutionID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT channel_abstractChannel_ID
    FOREIGN KEY (abstractChannel_ID )
    REFERENCES us3.abstractChannel (AbstractChannel_ID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.rawData
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.rawData (
  rawDataID INT NOT NULL AUTO_INCREMENT ,
  label VARCHAR(80) NOT NULL ,
  data LONGBLOB NOT NULL ,
  comment TEXT NULL ,
  experimentID INT NOT NULL ,
  channel_ID INT NOT NULL ,
  PRIMARY KEY (rawDataID) ,
  INDEX ndx_rawData_experimentID (experimentID ASC) ,
  INDEX ndx_rawData_channelID (channel_ID ASC) ,
  CONSTRAINT rawData_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES us3.experiment (experimentID )
    ON UPDATE CASCADE,
  CONSTRAINT rawData_channelID
    FOREIGN KEY (channel_ID )
    REFERENCES us3.channel (channel_ID )
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.editedData
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.editedData (
  editedDataID INT NOT NULL AUTO_INCREMENT ,
  rawDataID INT NOT NULL ,
  label VARCHAR(80) NOT NULL ,
  data LONGBLOB NOT NULL ,
  comment TEXT NULL ,
  PRIMARY KEY (editedDataID) ,
  INDEX ndx_editedData_rawDataID (rawDataID ASC) ,
  CONSTRAINT editedData_rawDataID
    FOREIGN KEY (rawDataID )
    REFERENCES us3.rawData (rawDataID )
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.results
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.results (
  resultsID INT NOT NULL AUTO_INCREMENT ,
  editedDataID INT NOT NULL ,
  label VARCHAR(80) NOT NULL ,
  type ENUM('velocity', 'equilibrium', 'equilproject', 'montecarlo', 'other') NOT NULL ,
  reportData LONGBLOB NULL ,
  resultData LONGBLOB NULL ,
  PRIMARY KEY (resultsID) ,
  INDEX ndx_results_editDataID (editedDataID ASC) ,
  CONSTRAINT results_editDataID
    FOREIGN KEY (editedDataID )
    REFERENCES us3.editedData (editedDataID )
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.projectPerson
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.projectPerson (
  projectID INT NOT NULL ,
  personID INT NOT NULL ,
  INDEX ndx_projectPerson_projectID (projectID ASC, personID ASC) ,
  INDEX ndx_projectPerson_personID (personID ASC) ,
  CONSTRAINT projectPerson_projectID
    FOREIGN KEY (projectID )
    REFERENCES us3.project (projectID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT projectPerson_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.experimentPerson
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.experimentPerson (
  experimentID INT NOT NULL ,
  personID INT NOT NULL ,
  INDEX ndx_experimentPerson_experimentID (experimentID ASC) ,
  INDEX ndx_experimentPerson_personID (personID ASC) ,
  CONSTRAINT experimentPerson_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES us3.experiment (experimentID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT experimentPerson_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.experimentSolutionChannel
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.experimentSolutionChannel (
  experimentID INT NOT NULL ,
  solutionID INT NOT NULL ,
  channel_ID INT NOT NULL ,
  INDEX ndx_experimentSolutionChannel_experimentID (experimentID ASC) ,
  INDEX ndx_experimentSolutionChannel_solutionID (solutionID ASC) ,
  INDEX ndx_experimentSolutionChannel_channel_ID (channel_ID ASC) ,
  CONSTRAINT experimentSolutionChannel_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES us3.experiment (experimentID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT experimentSolutionChannel_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES us3.solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT experimentSolutionChannel_channel_ID
    FOREIGN KEY (channel_ID )
    REFERENCES us3.channel (channel_ID )
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.imagePerson
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.imagePerson (
  imageID INT NOT NULL ,
  personID INT NOT NULL ,
  INDEX ndx_imagePerson_imageID (imageID ASC) ,
  INDEX ndx_imagePerson_personID (personID ASC) ,
  CONSTRAINT imagePerson_imageID
    FOREIGN KEY (imageID )
    REFERENCES us3.image (imageID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT imagePerson_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.analyte
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.analyte (
  analyteID INT NOT NULL AUTO_INCREMENT ,
  type ENUM( 'DNA', 'Peptide', 'Other' ) NULL ,
  sequence TEXT NULL ,
  vbar FLOAT NULL ,
  descrption TEXT NULL ,
  spectrum TEXT NULL ,
  molecularWeight FLOAT NULL ,
  PRIMARY KEY (analyteID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.bufferComponent
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.bufferComponent (
  bufferComponentID INT NOT NULL AUTO_INCREMENT ,
  description TEXT NULL ,
  viscosity TEXT NULL ,
  density TEXT NULL ,
  PRIMARY KEY (bufferComponentID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.refraction
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.refraction (
  refractionID INT NOT NULL AUTO_INCREMENT ,
  solutionID INT NULL ,
  lambda FLOAT NULL ,
  refractiveIndex FLOAT NULL ,
  PRIMARY KEY (refractionID) ,
  INDEX ndx_refraction_solutionID (solutionID ASC) ,
  CONSTRAINT refraction_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES us3.solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
COMMENT = 'Either bufferComponentID or analytID will be null';


-- -----------------------------------------------------
-- Table us3.buffer
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.buffer (
  bufferID INT NOT NULL AUTO_INCREMENT ,
  description TEXT NULL ,
  spectrum TEXT NULL ,
  pH FLOAT NULL ,
  viscosity FLOAT NULL ,
  density FLOAT NULL ,
  PRIMARY KEY (bufferID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.extinction
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.extinction (
  extinctionID INT NOT NULL AUTO_INCREMENT ,
  analyteID INT NULL ,
  bufferID INT NULL ,
  lambda FLOAT NULL ,
  molarExtinctionCoef FLOAT NULL ,
  PRIMARY KEY (extinctionID) ,
  INDEX ndx_extinction_analyteID (analyteID ASC) ,
  INDEX ndx_extinction_bufferID (bufferID ASC) ,
  CONSTRAINT analyteID
    FOREIGN KEY (analyteID )
    REFERENCES us3.analyte (analyteID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT bufferID
    FOREIGN KEY (bufferID )
    REFERENCES us3.buffer (bufferID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB
COMMENT = 'Either bufferID or analyteID will be null';


-- -----------------------------------------------------
-- Table us3.solutionAnalyte
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.solutionAnalyte (
  solutionID INT NOT NULL ,
  analyteID INT NOT NULL ,
  amount FLOAT NOT NULL ,
  PRIMARY KEY (solutionID) ,
  INDEX ndx_solutionAnalyte_solutionID (solutionID ASC) ,
  INDEX ndx_solutionAnalyte_analyteID (analyteID ASC) ,
  CONSTRAINT solutionAnalyte_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES us3.solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT solutionAnalyte_analyteID
    FOREIGN KEY (analyteID )
    REFERENCES us3.analyte (analyteID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.solutionBuffer
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.solutionBuffer (
  solutionID INT NOT NULL ,
  bufferID INT NOT NULL ,
  PRIMARY KEY (solutionID) ,
  INDEX ndx_solutionBuffer_solutionID (solutionID ASC) ,
  INDEX ndx_solutionBuffer_bufferID (bufferID ASC) ,
  CONSTRAINT solutionBuffer_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES us3.solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT solutionBuffer_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES us3.buffer (bufferID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.bufferLink
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.bufferLink (
  bufferID INT NOT NULL ,
  bufferComponentID INT NOT NULL ,
  concentration FLOAT NULL ,
  PRIMARY KEY (bufferID) ,
  INDEX ndx_bufferLink_bufferID (bufferID ASC) ,
  INDEX ndx_bufferLink_bufferComponentID (bufferComponentID ASC) ,
  CONSTRAINT bufferLink_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES us3.buffer (bufferID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT bufferLink_bufferComponentID
    FOREIGN KEY (bufferComponentID )
    REFERENCES us3.bufferComponent (bufferComponentID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.imageAnalyte
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.imageAnalyte (
  imageID INT NOT NULL ,
  analyteID INT NOT NULL ,
  PRIMARY KEY (imageID) ,
  INDEX ndx_imageAnalyte_imageID (imageID ASC) ,
  INDEX ndx_imageAnalyte_analyteID (analyteID ASC) ,
  CONSTRAINT imageAnalyte_imageID
    FOREIGN KEY (imageID )
    REFERENCES us3.image (imageID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT imageAnalyte_analyteID
    FOREIGN KEY (analyteID )
    REFERENCES us3.analyte (analyteID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.imageSolution
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.imageSolution (
  imageID INT NOT NULL ,
  solutionID INT NOT NULL ,
  PRIMARY KEY (imageID) ,
  INDEX ndx_imageSolution_imageID (imageID ASC) ,
  INDEX ndx_imageSolution_solutionID (solutionID ASC) ,
  CONSTRAINT imageSolution_imageID
    FOREIGN KEY (imageID )
    REFERENCES us3.image (imageID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT imageSolution_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES us3.solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.solutionPerson
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.solutionPerson (
  solutionID INT NOT NULL ,
  personID INT NOT NULL ,
  PRIMARY KEY (solutionID) ,
  INDEX ndx_solutionPerson_solutionID (solutionID ASC) ,
  INDEX ndx_solutionPerson_personID (personID ASC) ,
  CONSTRAINT solutionPerson_solutionID
    FOREIGN KEY (solutionID )
    REFERENCES us3.solution (solutionID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT solutionPerson_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.bufferPerson
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.bufferPerson (
  bufferID INT NOT NULL ,
  personID INT NOT NULL ,
  PRIMARY KEY (bufferID) ,
  INDEX ndx_bufferPerson_bufferID (bufferID ASC) ,
  INDEX ndx_bufferPerson_personID (personID ASC) ,
  CONSTRAINT bufferPerson_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES us3.buffer (bufferID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT bufferPerson_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.analytePerson
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.analytePerson (
  analyteID INT NOT NULL ,
  personID INT NOT NULL ,
  PRIMARY KEY (analyteID) ,
  INDEX ndx_analytePerson_analyteID (analyteID ASC) ,
  INDEX ndx_analytePerson_personID (personID ASC) ,
  CONSTRAINT analytePerson_analyteID
    FOREIGN KEY (analyteID )
    REFERENCES us3.analyte (analyteID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT analytePerson_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.bufferComponentPerson
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.bufferComponentPerson (
  bufferComponentID INT NOT NULL ,
  personID INT NOT NULL ,
  PRIMARY KEY (bufferComponentID) ,
  INDEX ndx_bufferComponentPerson_bufferComponentID (bufferComponentID ASC) ,
  INDEX ndx_bufferComponentPerson_personID (personID ASC) ,
  CONSTRAINT bufferComponentPerson_bufferComponentID
    FOREIGN KEY (bufferComponentID )
    REFERENCES us3.bufferComponent (bufferComponentID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT bufferComponentPerson_personID
    FOREIGN KEY (personID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.windowType
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.windowType (
  windowTypeID INT NOT NULL AUTO_INCREMENT ,
  name TEXT NULL ,
  PRIMARY KEY (windowTypeID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.cell
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.cell (
  cell_ID INT NOT NULL AUTO_INCREMENT ,
  windowTypeID INT NULL ,
  abstractCenterpieceID INT NULL ,
  experimentID INT NULL ,
  name TEXT NULL ,
  holeNumber INT NULL ,
  centerpieceSerialNumber TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (cell_ID) ,
  INDEX ndx_cell_windowTypeID (windowTypeID ASC) ,
  INDEX ndx_cell_abstractCenterpieceID (abstractCenterpieceID ASC) ,
  INDEX ndx_cell_experimentID (experimentID ASC) ,
  CONSTRAINT cell_windowTypeID
    FOREIGN KEY (windowTypeID )
    REFERENCES us3.windowType (windowTypeID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT cell_abstractCenterpieceID
    FOREIGN KEY (abstractCenterpieceID )
    REFERENCES us3.abstractCenterpiece (abstractCenterpieceID )
    ON DELETE SET NULL
    ON UPDATE CASCADE,
  CONSTRAINT cell_experimentID
    FOREIGN KEY (experimentID )
    REFERENCES us3.experiment (experimentID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.hoursInUse
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.hoursInUse (
  hoursInUseID INT NOT NULL AUTO_INCREMENT ,
  rotorID INT NULL ,
  dateUpdated TIMESTAMP NULL ,
  hoursInUse FLOAT NULL ,
  PRIMARY KEY (hoursInUseID) ,
  INDEX ndx_hoursInUse_rotorID (rotorID ASC) ,
  CONSTRAINT hoursInUse_rotorID
    FOREIGN KEY (rotorID )
    REFERENCES us3.rotor (rotorID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.opticalSystemSetting
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.opticalSystemSetting (
  opticalSystemSettingID INT NOT NULL AUTO_INCREMENT ,
  rawDataID INT NULL ,
  name TEXT NULL ,
  value FLOAT NULL ,
  dateUpdated TIMESTAMP NULL ,
  secondsDuration INT NULL ,
  hwType ENUM( 'fluor', 'wlAbsorb', 'interfere', 'radialAbs', 'MWL', 'other') NULL ,
  hwIndex INT NULL ,
  PRIMARY KEY (opticalSystemSettingID) ,
  INDEX ndx_opticalSystemSetting_rawDataID (rawDataID ASC) ,
  CONSTRAINT opticalSystemSetting_rawDataID
    FOREIGN KEY (rawDataID )
    REFERENCES us3.rawData (rawDataID )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.avivFluorescence
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.avivFluorescence (
  avivFluorescenceID INT NOT NULL AUTO_INCREMENT ,
  topRadius FLOAT NULL ,
  bottomRadius FLOAT NULL ,
  mmStepSize FLOAT NULL ,
  replicates INT NULL ,
  nmExcitation FLOAT NULL ,
  nmEmission FLOAT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (avivFluorescenceID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.multiWavelengthSystem
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.multiWavelengthSystem (
  multiWavelengthSystemID INT NOT NULL AUTO_INCREMENT ,
  startWavelength FLOAT NULL ,
  endWavelength FLOAT NULL ,
  nmStepsize FLOAT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (multiWavelengthSystemID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.beckmanWavelengthAbsorbance
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.beckmanWavelengthAbsorbance (
  beckmanWavelengthAbsorbanceID INT NOT NULL AUTO_INCREMENT ,
  radialPosition FLOAT NULL ,
  startWavelength FLOAT NULL ,
  endWavelength FLOAT NULL ,
  nmStepsize FLOAT NULL ,
  replicates INT NULL ,
  isIntensity BOOLEAN NULL ,
  secondsBetween INT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (beckmanWavelengthAbsorbanceID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.beckmanInterference
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.beckmanInterference (
  beckmanInterferenceID INT NOT NULL AUTO_INCREMENT ,
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
  PRIMARY KEY (beckmanInterferenceID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.beckmanRadialAbsorbance
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.beckmanRadialAbsorbance (
  beckmanRadialAbsorbanceID INT NOT NULL AUTO_INCREMENT ,
  topRadius FLOAT NULL ,
  bottomRadius FLOAT NULL ,
  mmStepSize FLOAT NULL ,
  replicates INT NULL ,
  isContinuousMode BOOLEAN NULL ,
  isIntensity BOOLEAN NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (beckmanRadialAbsorbanceID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.otherOpticalSystem
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.otherOpticalSystem (
  otherOpticalSystemID INT NOT NULL AUTO_INCREMENT ,
  name TEXT NULL ,
  dateUpdated TIMESTAMP NULL ,
  PRIMARY KEY (otherOpticalSystemID) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table us3.permits
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS us3.permits (
  investigatorID INT NOT NULL ,
  collaboratorID INT NOT NULL ,
  PRIMARY KEY (investigatorID) ,
  INDEX ndx_permits_investigatorID (investigatorID ASC) ,
  INDEX ndx_permits_collaboratorID (collaboratorID ASC) ,
  CONSTRAINT permits_investigatorID
    FOREIGN KEY (investigatorID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT permits_collaboratorID
    FOREIGN KEY (collaboratorID )
    REFERENCES us3.people (personID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;



SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
