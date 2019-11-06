--
-- drop_spectrum_constraints.sql
--
-- erase the constraints that go with the spectrum table; one or the other 
--   will always fail
--

ALTER TABLE spectrum
  DROP FOREIGN KEY fk_spectrum_bufferID;

ALTER TABLE spectrum
  DROP FOREIGN KEY fk_spectrum_analyteID;
