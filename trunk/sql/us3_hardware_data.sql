--
-- us3_hardware_data.sql
--
-- Some hard-wired hardware data 
-- 

-- 
-- Rotor information ported from Beckman website, US2 data rotor.dat
--

DELETE FROM abstractRotor;
DELETE FROM rotor;

INSERT INTO abstractRotor SET
  name              = 'Simulation',
  materialName      = 'titanium',
  numHoles          = 8,
  maxRPM            = 60000,
  magnetOffset      = 0.0,
  cellCenter        = 6.5,        -- cm
  defaultStretch    = '0 0 0 0 0',
  manufacturer      = 'None',
  materialRefURI    = '',
  dateUpdated       = NOW();

SET @simulation_rotor_ID = LAST_INSERT_ID();

INSERT INTO abstractRotor SET
  name              = 'AN50',
  materialName      = 'titanium',
  numHoles          = 8,
  maxRPM            = 50000,
  magnetOffset      = 0.0,
  cellCenter        = 6.5,        -- cm
  defaultStretch    = '7.754e-5 -1.546e-8 9.601e-12 -5.800e-17 6.948e-22',
  manufacturer      = 'Beckman',
  materialRefURI    = '',
  dateUpdated       = NOW();

SET @AN50_rotor_ID = LAST_INSERT_ID();
  
INSERT INTO abstractRotor SET
  name              = 'AN60',
  materialName      = 'titanium',
  numHoles          = 4,
  maxRPM            = 60000,
  magnetOffset      = 0.0,
  cellCenter        = 6.5,        -- cm
  defaultStretch    = '3.128e-5 -6.620e-9 7.264e-12 -6.152e-17 5.760e-22',
  manufacturer      = 'Beckman',
  materialRefURI    = '',
  dateUpdated       = NOW();

SET @AN60_rotor_ID = LAST_INSERT_ID();

INSERT INTO rotor SET
  abstractRotorID = @simulation_rotor_ID,
  name            = 'UTHSCSA Simulation',
  serialNumber    = 'UTHSCSA 1001',
  stretchFunction = '0 0 0 0 0',
  omega2_t        = 0.0,
  dateUpdated     = NOW();

INSERT INTO rotor SET
  abstractRotorID = @AN50_rotor_ID,
  name            = 'UTHSCSA AN50 #1',
  serialNumber    = 'UTHSCSA 1002',
  stretchFunction = '7.754e-5 -1.546e-8 9.601e-12 -5.800e-17 6.948e-22',
  omega2_t        = 0.0,
  dateUpdated     = NOW();

INSERT INTO rotor SET
  abstractRotorID = @AN60_rotor_ID,
  name            = 'UTHSCSA AN60 #1',
  serialNumber    = 'UTHSCSA 1003',
  stretchFunction = '3.128e-5 -6.620e-9 7.264e-12 -6.152e-17 5.760e-22',
  omega2_t        = 0.0,
  dateUpdated     = NOW();

