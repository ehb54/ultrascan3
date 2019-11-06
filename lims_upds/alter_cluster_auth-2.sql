--
-- Alter/Update clusterAuthorizations to reflect current clusters
--

ALTER TABLE people
  CHANGE ClusterAuthorizations clusterAuthorizations varchar(255) NOT NULL;

ALTER TABLE people
  ALTER COLUMN clusterAuthorizations
  SET DEFAULT 'stampede:lonestar5:comet:gordon:alamo';

UPDATE people
  SET clusterAuthorizations='stampede:lonestar5:comet:gordon:alamo'
  WHERE userlevel<4;

UPDATE people
  SET clusterAuthorizations='stampede:lonestar5:comet:gordon:alamo:alamo-local'
  WHERE userlevel>3;

