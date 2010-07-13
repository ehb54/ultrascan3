--
-- us3_expdata_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are procedures related to the experiment data
-- Run as us3admin
--

DELIMITER $$

-- Verifies that the user has permission to view or modify
--  the specified experiment
DROP FUNCTION IF EXISTS verify_editData_permission$$
CREATE FUNCTION verify_editData_permission( p_personGUID   CHAR(36),
                                            p_password     VARCHAR(80),
                                            p_editedDataID INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE count_experiments INT;
  DECLARE status            INT;
  DECLARE l_experimentID    INT;

  CALL config();
  SET status   = @ERROR;
  SET l_experimentID = 0;

  SELECT COUNT(*)
  INTO   count_experiments
  FROM   editedData ed, rawData rd
  WHERE  ed.rawDataID    = rd.rawDataID
  AND    ed.editedDataID = p_editedDataID;
 
  IF ( count_experiments = 1 ) THEN           -- this is what it should be
    SELECT rd.experimentID
    INTO   l_experimentID
    FROM   editedData ed, rawData rd
    WHERE  ed.rawDataID  = rd.rawDataID
    AND    ed.editedDataID = p_editedDataID;

    IF ( verify_experiment_permission( p_personGUID, p_password, l_experimentID ) = @OK ) THEN
      SET status = @OK;

    ELSE
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view or modify this experiment';

      SET status = @NOTPERMITTED;

    END IF;

  ELSE
    SET @US3_LAST_ERRNO = @NO_EXPERIMENT;
    SET @US3_LAST_ERROR = 'MySQL: the associated experiment does not exist';

    SET status = @NO_EXPERIMENT;

  END IF;

  RETURN( status );

END$$

-- A series of procedures dealing with rawData

-- Get a list of rawDataID's that correspond with this experiment
DROP PROCEDURE IF EXISTS get_rawDataIDs$$
CREATE PROCEDURE get_rawDataIDs ( p_personGUID   CHAR(36),
                                  p_password     VARCHAR(80),
                                  p_experimentID INT )
  READS SQL DATA

BEGIN
  DECLARE count_rawData INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_rawData
  FROM       rawData
  WHERE      experimentID = p_experimentID;

  IF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) THEN
    IF ( count_rawData = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   rawDataID
      FROM     rawData
      WHERE    experimentID = p_experimentID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- Returns the count of raw data entries associated with an
-- experiment; should be the same as the number of triples,
-- or c/c/w combinations
DROP FUNCTION IF EXISTS count_rawData$$
CREATE FUNCTION count_rawData( p_personGUID   CHAR(36),
                               p_password     VARCHAR(80),
                               p_experimentID INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_rawData INT;

  CALL config();
  SET count_rawData = 0;

  IF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) THEN
    -- This is either an admin, or a person inquiring about his own experiment
    SELECT COUNT(*)
    INTO   count_rawData
    FROM   rawData
    WHERE  experimentID = p_experimentID;

  END IF;

  RETURN( count_rawData );

END$$

-- INSERTs new rawData information about one c/c/w combination in an experiment
DROP PROCEDURE IF EXISTS new_rawData$$
CREATE PROCEDURE new_rawData ( p_personGUID   CHAR(36),
                               p_password     VARCHAR(80),
                               p_experimentID INT,
                               p_filename     VARCHAR(255),
                               p_label        VARCHAR(80),
                               p_comment      TEXT,
                               p_channelID    INT )
  MODIFIES SQL DATA

BEGIN

  DECLARE null_field    TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR 1048
    SET null_field = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  IF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) THEN
 
    -- This is either an admin, or a person inquiring about his own experiment
    INSERT INTO rawData SET
      experimentID  = p_experimentID,
      filename      = p_filename,
      label         = p_label,
      comment       = p_comment,
      channelID     = p_channelID;
   
    IF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: Attempt to insert NULL value in the rawData table";

    ELSE
      SET @LAST_INSERT_ID  = LAST_INSERT_ID();

    END IF;
   
   END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- SELECTs rawData information about one c/c/w combination in an experiment
DROP PROCEDURE IF EXISTS get_rawData$$
CREATE PROCEDURE get_rawData ( p_personGUID   CHAR(36),
                               p_password     VARCHAR(80),
                               p_rawDataID    INT )
  READS SQL DATA

BEGIN

  DECLARE l_experimentID  INT;
  DECLARE count_rawData   INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  -- Do a preliminary call to check permissions
  SELECT experimentID
  INTO   l_experimentID
  FROM   rawData
  WHERE  rawDataID = p_rawDataID
  LIMIT  1;                         -- there could be multiple

  SELECT     COUNT(*)
  INTO       count_rawData
  FROM       rawData
  WHERE      rawDataID = p_rawDataID;

  IF ( count_rawData = 0 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( verify_experiment_permission( p_personGUID, p_password, l_experimentID ) = @OK ) THEN
    -- This is either an admin, or a person inquiring about his own experiment
    SELECT @OK as status;

    SELECT  label, data, comment, experimentID, channelID
    FROM    rawData
    WHERE   rawDataID = p_rawDataID;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- DELETEs all rawData related to an experiment, plus information in related tables
-- Most likely used if you're updating an experiment 
DROP PROCEDURE IF EXISTS delete_rawData$$
CREATE PROCEDURE delete_rawData ( p_personGUID   CHAR(36),
                                  p_password     VARCHAR(80),
                                  p_experimentID INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) THEN

    -- Make sure records match if they have related tables or not
    DELETE      rawData, editedData, model, noise, modelPerson
    FROM        rawData
    LEFT JOIN   editedData  ON ( editedData.rawDataID = rawData.rawDataID )
    LEFT JOIN   model       ON ( model.editedDataID   = editedData.editedDataID )
    LEFT JOIN   noise       ON ( noise.modelID        = model.modelID )
    LEFT JOIN   modelPerson ON ( modelPerson.modelID  = model.modelID )
    WHERE       rawData.experimentID = p_experimentID;

--    DELETE FROM rawData
--    WHERE experimentID = p_experimentID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs a rawData record with the auc data
DROP PROCEDURE IF EXISTS upload_aucData$$
CREATE PROCEDURE upload_aucData ( p_personGUID   CHAR(36),
                                  p_password     VARCHAR(80),
                                  p_rawDataID    INT,
                                  p_data         LONGBLOB )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_experimentID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  -- Get information we need to verify ownership
  SELECT experimentID
  INTO   l_experimentID
  FROM   rawData
  WHERE  rawDataID = p_rawDataID;

  IF ( verify_experiment_permission( p_personGUID, p_password, l_experimentID ) = @OK ) THEN
 
    -- This is either an admin, or a person inquiring about his own experiment
    UPDATE rawData SET
      data           = p_data
    WHERE  rawDataID = p_rawDataID;

    SET @LAST_INSERT_ID  = LAST_INSERT_ID();

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- SELECTs a rawData record of auc data previously saved with upload_aucData
DROP PROCEDURE IF EXISTS download_aucData$$
CREATE PROCEDURE download_aucData ( p_personGUID   CHAR(36),
                                    p_password     VARCHAR(80),
                                    p_rawDataID    INT )
  READS SQL DATA

BEGIN
  DECLARE l_count_aucData INT;
  DECLARE l_experimentID  INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  -- Get information to verify that there are records
  SELECT COUNT(*)
  INTO   l_count_aucData
  FROM   rawData
  WHERE  rawDataID = p_rawDataID;

SET @DEBUG = CONCAT('Raw data ID = ', p_rawDataID,
                    'Count = ', l_count_aucData );
  -- Get information we need to verify ownership
  SELECT experimentID
  INTO   l_experimentID
  FROM   rawData
  WHERE  rawDataID = p_rawDataID;

  IF ( l_count_aucData != 1 ) THEN
    -- Probably no rows
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows exist with that ID (or too many rows)';

    SELECT @NOROWS AS status;
    
  ELSEIF ( verify_experiment_permission( p_personGUID, p_password, l_experimentID ) != @OK ) THEN
 
    -- verify_experiment_permission must have thrown an error, so pass it on
    SELECT @US3_LAST_ERRNO AS status;

  ELSE

    -- This is either an admin, or a person inquiring about his own experiment
    SELECT @OK AS status;

    SELECT data
    FROM   rawData
    WHERE  rawDataID = p_rawDataID;

  END IF;

END$$

