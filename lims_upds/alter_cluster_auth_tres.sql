--
-- Alter/Update clusterAuthorizations to reflect current clusters
--

-- ALTER TABLE people
-- CHANGE ClusterAuthorizations clusterAuthorizations varchar(255) NOT NULL;

ALTER TABLE people
  ALTER COLUMN clusterAuthorizations
  SET DEFAULT 'trestles:stampede:lonestar:comet:gordon:alamo';

UPDATE people
  SET clusterAuthorizations='trestles:stampede:lonestar:comet:gordon:alamo'
  WHERE userlevel<4;

UPDATE people
  SET clusterAuthorizations='trestles:stampede:lonestar:comet:gordon:alamo:juropa:jacinto:alamo-local:jacinto-local'
  WHERE userlevel>3;

