--
-- us3_hardware_data.sql
--
-- Some hard-wired hardware data 
-- 

-- 
-- Rotor information ported from Beckman website, US2 data rotor.dat
--

DELETE FROM abstractRotor;
DELETE FROM abstractCenterpiece;
DELETE FROM rotor;
DELETE FROM rotorCalibration;
DELETE FROM lab;

INSERT INTO abstractRotor SET
  abstractRotorID   = 1,
  abstractRotorGUID = 'a06a2795-99bd-67b4-ad90-401eb5f560fd',
  name              = 'AN50',
  materialName      = 'Titanium',
  numHoles          = 8,
  maxRPM            = 50000,
  magnetOffset      = 0.0,
  cellCenter        = 6.5,        -- cm
  manufacturer      = 'Beckman';

INSERT INTO abstractRotor SET
  abstractRotorID   = 2,
  abstractRotorGUID = 'fda22413-8d1a-4ef4-a9fc-620d0c00d434',
  name              = 'AN60',
  materialName      = 'Titanium',
  numHoles          = 4,
  maxRPM            = 60000,
  magnetOffset      = 0.0,
  cellCenter        = 6.5,        -- cm
  manufacturer      = 'Beckman';

INSERT INTO abstractRotor SET
  abstractRotorID   = 3,
  abstractRotorGUID = '4f9326de-0f40-5474-f957-1eaea5b1478c',
  name              = 'Simulation',
  materialName      = 'simulation',
  numHoles          = 8,
  maxRPM            = 60000,
  magnetOffset      = 0.0,
  cellCenter        = 6.5,        -- cm
  manufacturer      = 'Simulation';

INSERT INTO abstractRotor SET
  abstractRotorID   = 4,
  abstractRotorGUID = '68ea4167-f76a-1214-154b-54876bbed32b',
  name              = 'CFA',
  materialName      = 'CarbonFiber',
  numHoles          = 8,
  maxRPM            = 60000,
  magnetOffset      = 0.0,
  cellCenter        = 6.5,        -- cm
  manufacturer      = 'SpinAnalytical';

-- either angle or width will be null, depending on shape
INSERT INTO abstractCenterpiece SET
  abstractCenterpieceID = 1,
  loadMethod        = 'top',
  abstractCenterpieceGUID = '08cbd35f-151c-4814-8457-6ef8dd523a23',
  name              = 'Simulation 1-channel standard',
  materialName      = 'Simulated',
  channels          = 1,          -- x2 for actual number
  bottom            = '7.2',
  shape             = 'standard',
  maxRPM            = 60000,
  pathLength        = 1.2,
  angle             = 2.5,
  width             = NULL,
  canHoldSample     = 1,
  materialRefURI    = '',
  centerpieceRefURI = '',
  dataUpdated       = NOW();

-- either angle or width will be null, depending on shape
INSERT INTO abstractCenterpiece SET
  abstractCenterpieceID = 2,
  loadMethod        = 'top',
  abstractCenterpieceGUID = '340540ba-a225-4214-9136-2b4003af772e',
  name              = 'Epon 2-channel standard',
  materialName      = 'Epon',
  channels          = 1,          -- x2 for actual number
  bottom            = '7.2',
  shape             = 'standard',
  maxRPM            = 42000,
  pathLength        = 1.2,
  angle             = 2.5,
  width             = NULL,
  canHoldSample     = 1,
  materialRefURI    = '',
  centerpieceRefURI = '',
  dataUpdated       = NOW();

-- either angle or width will be null, depending on shape
INSERT INTO abstractCenterpiece SET
  abstractCenterpieceID = 3,
  loadMethod        = 'top',
  abstractCenterpieceGUID = 'e1dfbc5b-ceeb-4275-a280-0e3a5ab0c490',
  name              = 'Aluminum 2-channel standard',
  materialName      = 'Aluminum',
  channels          = 1,          -- x2 for actual number
  bottom            = '7.15',
  shape             = 'standard',
  maxRPM            = 60000,
  pathLength        = 1.2,
  angle             = 2.5,
  width             = NULL,
  canHoldSample     = 1,
  materialRefURI    = '',
  centerpieceRefURI = '',
  dataUpdated       = NOW();

-- either angle or width will be null, depending on shape
INSERT INTO abstractCenterpiece SET
  abstractCenterpieceID = 4,
  loadMethod        = 'top',
  abstractCenterpieceGUID = '25d18488-1afa-4b9b-8a6d-b59802f0a74a',
  name              = 'Epon 6-channel rectangular',
  materialName      = 'Epon',
  channels          = 3,          -- x2 for actual number
  bottom            = '6.111:6.613:7.104',
  shape             = 'rectangular',
  maxRPM            = 48000,
  pathLength        = 0.3,
  angle             = NULL,
  width             = 0.323,
  canHoldSample     = 1,
  materialRefURI    = '',
  centerpieceRefURI = '',
  dataUpdated       = NOW();

-- either angle or width will be null, depending on shape
INSERT INTO abstractCenterpiece SET
  abstractCenterpieceID = 5,
  loadMethod        = 'top',
  abstractCenterpieceGUID = 'd8eec2e8-cc11-46fd-b96e-8fe82059317d',
  name              = 'Titanium 2-channel standard',
  materialName      = 'Titanium',
  channels          = 1,          -- x2 for actual number
  bottom            = '7.15',
  shape             = 'standard',
  maxRPM            = 60000,
  pathLength        = 1.2,
  angle             = 2.5,
  width             = NULL,
  canHoldSample     = 1,
  materialRefURI    = '',
  centerpieceRefURI = '',
  dataUpdated       = NOW();

-- either angle or width will be null, depending on shape
INSERT INTO abstractCenterpiece SET
  abstractCenterpieceID = 6,
  loadMethod        = 'top',
  abstractCenterpieceGUID = '70641c51-ac63-4862-baf4-1f418489ed06',
  name              = 'Titanium 6-channel rectangular',
  materialName      = 'Titanium',
  channels          = 3,          -- x2 for actual number
  bottom            = '6.170:6.690:7.210', -- change to 6.111:6.613:7.104 ?
  shape             = 'rectangular',
  maxRPM            = 60000,
  pathLength        = 1.2,
  angle             = NULL,
  width             = 0.320,
  canHoldSample     = 1,
  materialRefURI    = '',
  centerpieceRefURI = '',
  dataUpdated       = NOW();

-- either angle or width will be null, depending on shape
INSERT INTO abstractCenterpiece SET
  abstractCenterpieceID = 7,
  loadMethod        = 'top',
  abstractCenterpieceGUID = '4a6410c5-099f-44d8-baa2-0e2d4c099ced',
  name              = 'Epon 2-channel band forming',
  materialName      = 'Epon',
  channels          = 1,          -- x2 for actual number
  bottom            = '7.2',
  shape             = 'band forming',
  maxRPM            = 42000,
  pathLength        = 1.2,
  angle             = 2.5,
  width             = NULL,
  canHoldSample     = 1,
  materialRefURI    = '',
  centerpieceRefURI = '',
  dataUpdated       = NOW();

-- either angle or width will be null, depending on shape
INSERT INTO abstractCenterpiece SET
  abstractCenterpieceID = 8,
  loadMethod        = 'top',
  abstractCenterpieceGUID = '30a5f2f8-b31a-42b8-a8a8-df077bf1b432',
  name              = 'Epon-Helmut 2-channel band forming',
  materialName      = 'Epon-Helmut',
  channels          = 1,          -- x2 for actual number
  bottom            = '7.2',
  shape             = 'band forming',
  maxRPM            = 60000,
  pathLength        = 1.2,
  angle             = 2.5,
  width             = NULL,
  canHoldSample     = 1,
  materialRefURI    = '',
  centerpieceRefURI = '',
  dataUpdated       = NOW();

-- either angle or width will be null, depending on shape
INSERT INTO abstractCenterpiece SET
  abstractCenterpieceID = 9,
  loadMethod        = 'top',
  abstractCenterpieceGUID = '2e4414c9-babf-4505-8556-4bd492b9fb27',
  name              = 'SVEL60 2-channel standard',
  materialName      = 'SVEL60',
  channels          = 1,          -- x2 for actual number
  bottom            = '7.2',
  shape             = 'standard',
  maxRPM            = 60000,
  pathLength        = 1.2,
  angle             = 2.0,
  width             = NULL,
  canHoldSample     = 1,
  materialRefURI    = '',
  centerpieceRefURI = '',
  dataUpdated       = NOW();

-- either angle or width will be null, depending on shape
INSERT INTO abstractCenterpiece SET
  abstractCenterpieceID = 10,
  loadMethod        = 'top',
  abstractCenterpieceGUID = 'd8e93427-eb88-42d1-b459-ab6787e571c6',
  name              = 'SVEL60 2-channel meniscus matching',
  materialName      = 'SVEL60',
  channels          = 1,          -- x2 for actual number
  bottom            = '7.2',
  shape             = 'meniscus matching',
  maxRPM            = 60000,
  pathLength        = 1.2,
  angle             = 2.0,
  width             = NULL,
  canHoldSample     = 1,
  materialRefURI    = '',
  centerpieceRefURI = '',
  dataUpdated       = NOW();

-- Default rotors

INSERT INTO rotor SET
  rotorID         = 1,
  abstractRotorID = 1,
  labID           = 1,
  rotorGUID       = '851389f4-2155-7da4-05ae-ce4e51683a3c',
  name            = 'Default 1001 (AN50)',
  serialNumber    = 'DEFAULT 1001';

INSERT INTO rotor SET
  rotorID         = 2,
  abstractRotorID = 2,
  labID           = 1,
  rotorGUID       = 'bb014301-6b8b-f0f4-81b5-8ae36dac9ed5',
  name            = 'Default 1002 (AN60)',
  serialNumber    = 'DEFAULT 1002';

INSERT INTO rotor SET
  rotorID         = 3,
  abstractRotorID = 3,
  labID           = 1,
  rotorGUID       = '32673123-095b-7b64-49dd-e58cb14d4fed',
  name            = 'Default 1003 (Simulation)',
  serialNumber    = 'DEFAULT 1003';

INSERT INTO rotor SET
  rotorID         = 4,
  abstractRotorID = 4,
  labID           = 1,
  rotorGUID       = '4791c966-2fcd-4a1b-b049-ceab5ad11d3e',
  name            = 'Default 1004 (CFA)',
  serialNumber    = 'DEFAULT 1004';

-- Default rotor configurations

INSERT INTO rotorCalibration SET
  rotorCalibrationID   = 1,
  rotorID              = 1,
  rotorCalibrationGUID = 'f28482f7-9b43-49a3-a8b8-528484e795c2',
  report               = 'This is a dummy calibration --- please replace.',
  coeff1               = 0.0,
  coeff2               = 0.0,
  omega2_t             = 0,
  dateUpdated          = NOW(),
  calibrationExperimentID = -1;

INSERT INTO rotorCalibration SET
  rotorCalibrationID   = 2,
  rotorID              = 2,
  rotorCalibrationGUID = 'ca6d718b-db7a-4744-894d-ad73f8ee3569',
  report               = 'This is a dummy calibration --- please replace.',
  coeff1               = 0.0,
  coeff2               = 0.0,
  omega2_t             = 0,
  dateUpdated          = NOW(),
  calibrationExperimentID = -1;

INSERT INTO rotorCalibration SET
  rotorCalibrationID   = 3,
  rotorID              = 3,
  rotorCalibrationGUID = 'ba39dd96-f188-cd24-31b1-40d63ad83fa0',
  report               = 'This is a dummy calibration --- please replace.',
  coeff1               = 0.0,
  coeff2               = 0.0,
  omega2_t             = 0,
  dateUpdated          = NOW(),
  calibrationExperimentID = -1;

INSERT INTO rotorCalibration SET
  rotorCalibrationID   = 4,
  rotorID              = 4,
  rotorCalibrationGUID = 'a167e7f5-e554-1334-c1a6-af53531492ea',
  report               = 'This is a dummy calibration --- please replace.',
  coeff1               = 0.0,
  coeff2               = 0.0,
  omega2_t             = 0,
  dateUpdated          = NOW(),
  calibrationExperimentID = -1;

INSERT INTO lab SET
  labID         = 1,
  labGUID       = 'c61cedd3-dd0f-3db4-1118-ee4db019bcfd',
  name          = 'UTHSCSA Lab #1',
  building      = 'HSC-MED',
  room          = '425D',
  dateUpdated   = NOW();

INSERT INTO lab SET
  labID         = 2,
  labGUID       = '7532024e-afb4-ee84-d160-3b9625a10b43',
  name          = 'UTHSCSA Lab #2',
  building      = 'HSC-MED',
  room          = '420D',
  dateUpdated   = NOW();

