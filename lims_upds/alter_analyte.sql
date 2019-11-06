--
-- Alter/Update analyte and buffer tables for gradientForming and manual
--

ALTER TABLE analyte
  ADD COLUMN gradientForming tinyint(1) NOT NULL DEFAULT '0'
  AFTER molecularWeight;

ALTER TABLE analyte
  CHANGE doubleStranded  doubleStranded tinyint(1) NOT NULL default '0';

ALTER TABLE analyte
  CHANGE complement      complement     tinyint(1) NOT NULL default '0';

ALTER TABLE analyte
  CHANGE _3prime         _3prime        tinyint(1) NOT NULL default '0';

ALTER TABLE analyte
  CHANGE _5prime         _5prime        tinyint(1) NOT NULL default '0';

ALTER TABLE buffer
  ADD COLUMN manual tinyint(1) NOT NULL DEFAULT '0'
  AFTER density;

ALTER TABLE bufferComponent
  ADD COLUMN c_range text
  AFTER density;

ALTER TABLE bufferComponent
  ADD COLUMN gradientForming tinyint(1) NOT NULL DEFAULT '0'
  AFTER c_range;

