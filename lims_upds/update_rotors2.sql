--
-- Script to re-establish abstract rotor ID's for default rotors in rotor table
--

SELECT COUNT(*)
FROM rotor
where abstractRotorID IS NULL;

