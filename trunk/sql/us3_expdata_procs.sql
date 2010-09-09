--
-- us3_expdata_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are procedures related to the experiment data
-- Run as us3admin
--

DELIMITER $$

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

      SELECT   rawDataID, label, filename
      FROM     rawData
      WHERE    experimentID = p_experimentID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- Translate a rawDataGUID into a rawDataID
DROP PROCEDURE IF EXISTS get_rawDataID_from_GUID$$
CREATE PROCEDURE get_rawDataID_from_GUID ( p_personGUID   CHAR(36),
                                           p_password     VARCHAR(80),
                                           p_rawDataGUID  CHAR(36) )
  READS SQL DATA

BEGIN
  DECLARE count_rawData  INT;
  DECLARE l_rawDataID    INT;
  DECLARE l_experimentID INT;
  DECLARE l_solutionID   INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_rawData
  FROM       rawData
  WHERE      rawDataGUID = p_rawDataGUID;

  IF ( count_rawData = 0 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    -- Let's get both the rawDataID and the experimentID so we can verify permission
    SELECT rawDataID, experimentID, solutionID
    INTO   l_rawDataID, l_experimentID, l_solutionID
    FROM   rawData
    WHERE  rawDataGUID = p_rawDataGUID
    LIMIT  1;                           -- should be only 1

    IF ( verify_experiment_permission( p_personGUID, p_password, l_experimentID ) = @OK ) THEN
      SELECT @OK AS status;

      SELECT l_rawDataID AS rawDataID, 
             l_experimentID AS experimentID,
             l_solutionID AS solutionID;

    ELSE
      SELECT @US3_LAST_ERRNO AS status;

    END IF;

  END IF;

END$$

-- Returns the count of raw data entries associated with an
-- experiment; should be the same as the number of triples,
-- or c/c/w combinations
DROP FUNCTION IF EXISTS count_rawData_by_experiment$$
CREATE FUNCTION count_rawData_by_experiment( p_personGUID   CHAR(36),
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

-- Returns the count of raw data entries associated with a user
DROP FUNCTION IF EXISTS count_rawData$$
CREATE FUNCTION count_rawData( p_personGUID   CHAR(36),
                               p_password     VARCHAR(80),
                               p_ID           INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_rawData INT;

  CALL config();
  SET count_rawData = 0;

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( p_ID > 0 ) THEN
      SELECT     COUNT(*)
      INTO       count_rawData
      FROM       rawData, experiment, experimentPerson
      WHERE      experimentPerson.personID = p_ID
      AND        experiment.experimentID = experimentPerson.experimentID
      AND        rawData.experimentID = experiment.experimentID;

    ELSE
      SELECT     COUNT(*)
      INTO       count_rawData
      FROM       rawData;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view those experiments';
     
    ELSE
      -- This person is asking about his own experiments
      -- Ignore p_ID and return user's own
      SELECT     COUNT(*)
      INTO       count_rawData
      FROM       rawData, experiment, experimentPerson
      WHERE      experimentPerson.personID = @US3_ID
      AND        experiment.experimentID = experimentPerson.experimentID
      AND        rawData.experimentID = experiment.experimentID;

    END IF;
    
  END IF;

  RETURN( count_rawData );

END$$

-- Get a list of all rawDataID's that belong to this user
DROP PROCEDURE IF EXISTS all_rawDataIDs$$
CREATE PROCEDURE all_rawDataIDs ( p_personGUID   CHAR(36),
                                  p_password     VARCHAR(80),
                                  p_ID           INT )
  READS SQL DATA

BEGIN
  DECLARE count_rawData INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( count_rawData( p_personGUID, p_password, p_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;
  
      IF ( p_ID > 0 ) THEN
        SELECT     rawDataID, rawData.label, rawData.filename,
                   rawData.experimentID, rawData.solutionID, rawData.lastUpdated
        FROM       rawData, experiment, experimentPerson
        WHERE      experimentPerson.personID = p_ID
        AND        experiment.experimentID = experimentPerson.experimentID
        AND        rawData.experimentID = experiment.experimentID
        ORDER BY   rawData.lastUpdated DESC;

      ELSE
        SELECT     rawDataID, rawData.label, rawData.filename,
                   rawData.experimentID, rawData.solutionID, rawData.lastUpdated
        FROM       rawData, experiment, experimentPerson
        WHERE      experiment.experimentID = experimentPerson.experimentID
        AND        rawData.experimentID = experiment.experimentID
        ORDER BY   rawData.lastUpdated DESC;

      END IF;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this experiment';
     
      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( count_experiments( p_personGUID, p_password, @US3_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      -- Ok, user wants his own info
      SELECT @OK AS status;

      SELECT     rawDataID, rawData.label, rawData.filename,
                 rawData.experimentID, rawData.solutionID, rawData.lastUpdated
      FROM       rawData, experiment, experimentPerson
      WHERE      experimentPerson.personID = @US3_ID
      AND        experiment.experimentID = experimentPerson.experimentID
      AND        rawData.experimentID = experiment.experimentID
      ORDER BY   rawData.lastUpdated DESC;

    END IF;

  END IF;

END$$

-- INSERTs new rawData information about one c/c/w combination in an experiment
DROP PROCEDURE IF EXISTS new_rawData$$
CREATE PROCEDURE new_rawData ( p_personGUID   CHAR(36),
                               p_password     VARCHAR(80),
                               p_rawDataGUID  CHAR(36),
                               p_label        VARCHAR(80),
                               p_filename     VARCHAR(255),
                               p_comment      TEXT,
                               p_experimentID INT,
                               p_solutionID   INT,
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
      rawDataGUID   = p_rawDataGUID,    -- this needs to be generated by caller
      label         = p_label,
      filename      = p_filename,
      comment       = p_comment,
      experimentID  = p_experimentID,
      solutionID    = p_solutionID,
      channelID     = p_channelID,
      lastUpdated   = NOW();
   
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

    SELECT  rawDataGUID, label, filename, comment, experimentID, solutionID, channelID, lastUpdated
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
    -- Have to do it in a couple of stages because of the constraints
    DELETE      model, noise, modelPerson
    FROM        rawData
    LEFT JOIN   editedData  ON ( editedData.rawDataID = rawData.rawDataID )
    LEFT JOIN   model       ON ( model.editedDataID   = editedData.editedDataID )
    LEFT JOIN   noise       ON ( noise.modelID        = model.modelID )
    LEFT JOIN   modelPerson ON ( modelPerson.modelID  = model.modelID )
    WHERE       rawData.experimentID = p_experimentID;

    DELETE      editedData
    FROM        rawData
    LEFT JOIN   editedData  ON ( editedData.rawDataID = rawData.rawDataID )
    WHERE       rawData.experimentID = p_experimentID;

    DELETE FROM rawData
    WHERE       experimentID = p_experimentID;

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
  DECLARE not_found      TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR NOT FOUND
    SET not_found = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
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

    IF ( not_found = 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_RAWDATA;
      SET @US3_LAST_ERROR = "MySQL: No raw data with that ID exists";

    END IF;

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

-- A series of procedures dealing with editedData

-- Verifies that the user has permission to view or modify
--  the specified edit profile
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
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT COUNT(*)
  INTO   count_experiments
  FROM   editedData, rawData
  WHERE  editedDataID = p_editedDataID
  AND    editedData.rawDataID = rawData.rawDataID;
 
  IF ( count_experiments = 1 ) THEN           -- this is what it should be
    -- Find experimentID so we can check permissions on that
    SELECT experimentID
    INTO   l_experimentID
    FROM   editedData, rawData
    WHERE  editedDataID = p_editedDataID
    AND    editedData.rawDataID = rawData.rawDataID;

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

-- Get a list of all editedDataID's that belong to this raw data
DROP PROCEDURE IF EXISTS get_editedDataIDs$$
CREATE PROCEDURE get_editedDataIDs ( p_personGUID   CHAR(36),
                                     p_password     VARCHAR(80),
                                     p_rawDataID INT )
  READS SQL DATA

BEGIN
  DECLARE count_editedData INT;
  DECLARE l_experimentID   INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_editedData
  FROM       editedData
  WHERE      rawDataID = p_rawDataID;

  -- Get the experimentID so we can verify permission on this data
  SELECT     experimentID
  INTO       l_experimentID
  FROM       rawData
  WHERE      rawDataID = p_rawDataID;

  IF ( verify_experiment_permission( p_personGUID, p_password, l_experimentID ) = @OK ) THEN
    IF ( count_editedData = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   editedDataID, label, filename
      FROM     editedData
      WHERE    rawDataID = p_rawDataID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- Returns the count of edit profiles associated with a user
DROP FUNCTION IF EXISTS count_editedData$$
CREATE FUNCTION count_editedData( p_personGUID   CHAR(36),
                                  p_password     VARCHAR(80),
                                  p_ID           INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_editedData INT;

  CALL config();
  SET count_editedData = 0;

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( p_ID > 0 ) THEN
      SELECT     COUNT(*)
      INTO       count_editedData
      FROM       editedData, rawData, experiment, experimentPerson
      WHERE      experimentPerson.personID = p_ID
      AND        experiment.experimentID = experimentPerson.experimentID
      AND        rawData.experimentID = experiment.experimentID
      AND        editedData.rawDataID = rawData.rawDataID;

    ELSE
      SELECT     COUNT(*)
      INTO       count_editedData
      FROM       editedData;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view those experiments';
     
    ELSE
      -- This person is asking about his own experiments
      -- Ignore p_ID and return user's own
      SELECT     COUNT(*)
      INTO       count_editedData
      FROM       editedData, rawData, experiment, experimentPerson
      WHERE      experimentPerson.personID = @US3_ID
      AND        experiment.experimentID = experimentPerson.experimentID
      AND        rawData.experimentID = experiment.experimentID
      AND        editedData.rawDataID = rawData.rawDataID;

    END IF;
    
  END IF;

  RETURN( count_editedData );

END$$

-- Returns the count of edit profiles associated with an
-- raw data set
DROP FUNCTION IF EXISTS count_editedData_by_rawData$$
CREATE FUNCTION count_editedData_by_rawData( p_personGUID   CHAR(36),
                                             p_password     VARCHAR(80),
                                             p_rawDataID    INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_editedData INT;
  DECLARE l_experimentID   INT;

  CALL config();
  SET count_editedData = 0;

  -- Get the experimentID so we can verify permission on this data
  SELECT     experimentID
  INTO       l_experimentID
  FROM       rawData
  WHERE      rawDataID = p_rawDataID;

  IF ( verify_experiment_permission( p_personGUID, p_password, l_experimentID ) = @OK ) THEN
    -- This is either an admin, or a person inquiring about his own experiment
    SELECT COUNT(*)
    INTO   count_editedData
    FROM   editedData
    WHERE  rawDataID = p_rawDataID;

  END IF;

  RETURN( count_editedData );

END$$

-- Get a list of all editDataID's that belong to this user
DROP PROCEDURE IF EXISTS all_editedDataIDs$$
CREATE PROCEDURE all_editedDataIDs ( p_personGUID   CHAR(36),
                                     p_password     VARCHAR(80),
                                     p_ID           INT )
  READS SQL DATA

BEGIN
  DECLARE count_editData INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( count_editData( p_personGUID, p_password, p_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;
  
      IF ( p_ID > 0 ) THEN
        SELECT     editedDataID, editedData.label, editedData.filename,
                   editedData.rawDataID, rawData.experimentID, editedData.lastUpdated
        FROM       editedData, rawData, experiment, experimentPerson
        WHERE      experimentPerson.personID = p_ID
        AND        experiment.experimentID = experimentPerson.experimentID
        AND        rawData.experimentID = experiment.experimentID
        AND        editedData.rawDataID = rawData.rawDataID
        ORDER BY   editedData.lastUpdated DESC;

      ELSE
        SELECT     editedDataID, editedData.label, editedData.filename,
                   editedData.rawDataID, rawData.experimentID, editedData.lastUpdated
        FROM       editedData, rawData, experiment, experimentPerson
        WHERE      experiment.experimentID = experimentPerson.experimentID
        AND        rawData.experimentID = experiment.experimentID
        AND        editedData.rawDataID = rawData.rawDataID
        ORDER BY   editedData.lastUpdated DESC;

      END IF;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this experiment';
     
      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( count_experiments( p_personGUID, p_password, @US3_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      -- Ok, user wants his own info
      SELECT @OK AS status;

      SELECT     editedDataID, editedData.label, editedData.filename,
                 editedData.rawDataID, rawData.experimentID, editedData.lastUpdated
      FROM       editedData, rawData, experiment, experimentPerson
      WHERE      experimentPerson.personID = @US3_ID
      AND        experiment.experimentID = experimentPerson.experimentID
      AND        rawData.experimentID = experiment.experimentID
      AND        editedData.rawDataID = rawData.rawDataID
      ORDER BY   editedData.lastUpdated DESC;

    END IF;

  END IF;

END$$

-- INSERTs new editedData information about one c/c/w combination in an experiment
DROP PROCEDURE IF EXISTS new_editedData$$
CREATE PROCEDURE new_editedData ( p_personGUID   CHAR(36),
                                  p_password     VARCHAR(80),
                                  p_rawDataID    INT,
                                  p_editGUID     CHAR(36),
                                  p_label        VARCHAR(80),
                                  p_filename     VARCHAR(255),
                                  p_comment      TEXT )
  MODIFIES SQL DATA

BEGIN

  DECLARE null_field     TINYINT DEFAULT 0;
  DECLARE l_experimentID INT;

  DECLARE CONTINUE HANDLER FOR 1048
    SET null_field = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  -- Get the experimentID so we can verify permission on this data
  SELECT     experimentID
  INTO       l_experimentID
  FROM       rawData
  WHERE      rawDataID = p_rawDataID;

  IF ( verify_experiment_permission( p_personGUID, p_password, l_experimentID ) = @OK ) THEN
 
    -- This is either an admin, or a person inquiring about his own experiment
    INSERT INTO editedData SET
      rawDataID     = p_rawDataID,
      editGUID      = p_editGUID,
      label         = p_label,
      filename      = p_filename,
      comment       = p_comment,
      lastUpdated   = NOW();
   
    IF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: Attempt to insert NULL value in the editedData table";

    ELSE
      SET @LAST_INSERT_ID  = LAST_INSERT_ID();

    END IF;
   
   END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- SELECTs editedData information about one edit profile
DROP PROCEDURE IF EXISTS get_editedData$$
CREATE PROCEDURE get_editedData ( p_personGUID   CHAR(36),
                                  p_password     VARCHAR(80),
                                  p_editedDataID INT )
  READS SQL DATA

BEGIN

  DECLARE count_editedData   INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  SELECT     COUNT(*)
  INTO       count_editedData
  FROM       editedData
  WHERE      editedDataID = p_editedDataID;

  IF ( count_editedData = 0 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( verify_editData_permission( p_personGUID, p_password, p_editedDataID ) = @OK ) THEN
    -- This is either an admin, or a person inquiring about his own experiment
    SELECT @OK as status;

    SELECT  rawDataID, editGUID, label, filename, comment, lastUpdated
    FROM    editedData
    WHERE   editedDataID = p_editedDataID;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- UPDATEs existing editedData information about one c/c/w combination in an experiment
DROP PROCEDURE IF EXISTS update_editedData$$
CREATE PROCEDURE update_editedData ( p_personGUID   CHAR(36),
                                     p_password     VARCHAR(80),
                                     p_editedDataID INT,
                                     p_rawDataID    INT,
                                     p_editGUID     CHAR(36),
                                     p_label        VARCHAR(80),
                                     p_filename     VARCHAR(255),
                                     p_comment      TEXT )
  MODIFIES SQL DATA

BEGIN

  DECLARE not_found      TINYINT DEFAULT 0;
  DECLARE null_field     TINYINT DEFAULT 0;
  DECLARE l_experimentID INT;

  DECLARE CONTINUE HANDLER FOR NOT FOUND
    SET not_found = 1;

  DECLARE CONTINUE HANDLER FOR 1048
    SET null_field = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  -- Get the experimentID so we can verify permission on this data
  SELECT     experimentID
  INTO       l_experimentID
  FROM       rawData
  WHERE      rawDataID = p_rawDataID;

  IF ( verify_experiment_permission( p_personGUID, p_password, l_experimentID ) = @OK ) THEN
 
    -- This is either an admin, or a person inquiring about his own experiment
    UPDATE editedData SET
      rawDataID     = p_rawDataID,
      editGUID      = p_editGUID,
      label         = p_label,
      filename      = p_filename,
      comment       = p_comment,
      lastUpdated   = NOW()
    WHERE editedDataID = p_editedDataID;
   
    IF ( not_found = 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_EDITDATA;
      SET @US3_LAST_ERROR = "MySQL: No edit profile with that ID exists";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: Attempt to update with a NULL value in the editedData table";

    END IF;
   
   END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- DELETEs all editedData related to an edit profile, plus information in related tables
DROP PROCEDURE IF EXISTS delete_editedData$$
CREATE PROCEDURE delete_editedData ( p_personGUID   CHAR(36),
                                     p_password     VARCHAR(80),
                                     p_editedDataID INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_editData_permission( p_personGUID, p_password, p_editedDataID ) = @OK ) THEN

    -- Make sure records match if they have related tables or not
    DELETE      editedData, model, noise, modelPerson
    FROM        editedData
    LEFT JOIN   model       ON ( model.editedDataID   = editedData.editedDataID )
    LEFT JOIN   noise       ON ( noise.modelID        = model.modelID )
    LEFT JOIN   modelPerson ON ( modelPerson.modelID  = model.modelID )
    WHERE       editedData.editedDataID = p_editedDataID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an editData record with the edit profile data
DROP PROCEDURE IF EXISTS upload_editData$$
CREATE PROCEDURE upload_editData ( p_personGUID   CHAR(36),
                                   p_password     VARCHAR(80),
                                   p_editedDataID INT,
                                   p_data         LONGBLOB )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_experimentID INT;
  DECLARE not_found      TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR NOT FOUND
    SET not_found = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  IF ( verify_editData_permission( p_personGUID, p_password, p_editedDataID ) = @OK ) THEN
 
    -- This is either an admin, or a person inquiring about his own experiment
    UPDATE editedData SET
           data         = p_data
    WHERE  editedDataID = p_editedDataID;

    IF ( not_found = 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_EDITDATA;
      SET @US3_LAST_ERROR = "MySQL: No edit profile with that ID exists";

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- SELECTs an editedData record of xml data previously saved with upload_aucData
DROP PROCEDURE IF EXISTS download_editData$$
CREATE PROCEDURE download_editData ( p_personGUID   CHAR(36),
                                     p_password     VARCHAR(80),
                                     p_editedDataID INT )
  READS SQL DATA

BEGIN
  DECLARE l_count_editData INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  -- Get information to verify that there are records
  SELECT COUNT(*)
  INTO   l_count_editData
  FROM   editedData
  WHERE  editedDataID = p_editedDataID;

SET @DEBUG = CONCAT('Edited data ID = ', p_editedDataID,
                    'Count = ', l_count_editData );
  IF ( l_count_editData != 1 ) THEN
    -- Probably no rows
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows exist with that ID (or too many rows)';

    SELECT @NOROWS AS status;
    
  ELSEIF ( verify_editData_permission( p_personGUID, p_password, p_editedDataID ) != @OK ) THEN
 
    -- verify_experiment_permission must have thrown an error, so pass it on
    SELECT @US3_LAST_ERRNO AS status;

  ELSE

    -- This is either an admin, or a person inquiring about his own experiment
    SELECT @OK AS status;

    SELECT data
    FROM   editedData
    WHERE  editedDataID = p_editedDataID;

  END IF;

END$$

