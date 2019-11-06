--
-- Add the data description field to the reports. 
--

ALTER TABLE reportTriple
  ADD COLUMN dataDescription VARCHAR(255) NOT NULL DEFAULT ''
  AFTER triple;
