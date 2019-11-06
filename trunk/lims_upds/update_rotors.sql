--
-- Script to re-establish abstract rotor ID's for default rotors in rotor table
--

UPDATE rotor
SET abstractRotorID = 1 
WHERE rotorID = 1;

UPDATE rotor
SET abstractRotorID = 2 
WHERE rotorID = 2;

UPDATE rotor
SET abstractRotorID = 3 
WHERE rotorID = 3;

