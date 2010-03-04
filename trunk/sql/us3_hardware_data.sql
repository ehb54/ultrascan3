--
-- us3_hardware_data.sql
--
-- Some hard-wired hardware data 
-- 

-- 
-- Rotor information ported from Beckman website, US2 data rotor.dat
--

DELETE FROM abstractRotor;

INSERT INTO abstractRotor SET
  abstractRotorID   = 1,
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

INSERT INTO abstractRotor SET
  abstractRotorID   = 2,
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

INSERT INTO abstractRotor SET
  abstractRotorID   = 3,
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

