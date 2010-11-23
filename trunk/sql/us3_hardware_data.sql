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
  abstractRotorGUID = '',
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
  abstractCenterpieceGUID = NULL,
  name              = 'Simulation 1-channel standard',
  materialName      = 'Simulation',
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
  abstractCenterpieceGUID = NULL,
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
  abstractCenterpieceGUID = NULL,
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
  abstractCenterpieceGUID = NULL,
  name              = 'Epon 6-channel rectangular',
  materialName      = 'Epon',
  channels          = 3,          -- x2 for actual number
  bottom            = '6.111:6.613:7.104',
  shape             = 'rectangular',
  maxRPM            = 48000,
  pathLength        = 1.2,
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
  abstractCenterpieceGUID = NULL,
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
  abstractCenterpieceGUID = NULL,
  name              = 'Titanium 6-channel rectangular',
  materialName      = 'Titanium',
  channels          = 3,          -- x2 for actual number
  bottom            = '6.170:6.690:7.210',
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
  abstractCenterpieceGUID = NULL,
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

