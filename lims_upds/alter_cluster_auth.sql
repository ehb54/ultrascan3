--
-- Alter/Update clusterAuthorizations to reflect current clusters
--

-- ALTER TABLE people
-- CHANGE ClusterAuthorizations clusterAuthorizations varchar(255) NOT NULL;

ALTER TABLE people
  ALTER COLUMN clusterAuthorizations
  SET DEFAULT 'lonestar5:stampede2:comet:jetstream';

UPDATE people
  SET clusterAuthorizations='lonestar5:stampede2:comet:jetstream'
  WHERE userlevel<4;

UPDATE people
  SET clusterAuthorizations='lonestar5:stampede2:comet:jetstream:jetstream-local'
  WHERE userlevel>3;

