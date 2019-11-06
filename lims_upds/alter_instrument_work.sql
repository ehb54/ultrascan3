--
-- Alter instrument table by adding new column
--


--ALTER TABLE instrument         
--  DROP COLUMN chromaticAB;


--ALTER TABLE instrument         
--  ADD COLUMN chromaticAB  TEXT  NULL AFTER optimaDBpassw;

--ALTER TABLE instrument                                         
--  ADD COLUMN RadCalWvl  TEXT  NULL AFTER opsys3;       

ALTER TABLE instrument
  ADD COLUMN optimaHost         TEXT             NULL AFTER radialCalID,
  ADD COLUMN optimaPort         int(11)          NULL AFTER optimaHost,
  ADD COLUMN optimaDBname       TEXT             NULL AFTER optimaPort,	
  ADD COLUMN optimaDBusername   TEXT             NULL AFTER optimaDBname,	
  ADD COLUMN optimaDBpassw      BLOB             NULL AFTER optimaDBusername,
  ADD COLUMN chromaticAB        TEXT             NULL AFTER optimaDBpassw,		
  ADD COLUMN selected           BOOL             DEFAULT false AFTER chromaticAB,
  ADD COLUMN opsys1             ENUM('UV/visible', 'Rayleigh Interference', 'Fluorescense', '(not installed)') NOT NULL AFTER selected,
  ADD COLUMN opsys2             ENUM('UV/visible', 'Rayleigh Interference', 'Fluorescense', '(not installed)') NOT NULL AFTER opsys1,
  ADD COLUMN opsys3             ENUM('UV/visible', 'Rayleigh Interference', 'Fluorescense', '(not installed)') NOT NULL AFTER opsys2,
  ADD COLUMN RadCalWvl          int(11)          NULL AFTER opsys3;       
			

--ALTER TABLE instrument
--  DROP COLUMN optimaHost, 
--  DROP COLUMN optimaPort, 
--  DROP COLUMN optimaDBname,
--  DROP COLUMN optimaDBusername,
--  DROP COLUMN optimaDBpassw,
--  DROP COLUMN chromaticAB, 
--  DROP COLUMN selected,
--  DROP COLUMN opsys1,
--  DROP COLUMN opsys2,
--  DROP COLUMN opsys3,
--  DROP COLUMN RadCalWvl; 
