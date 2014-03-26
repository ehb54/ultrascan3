--
-- us3_timestate_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are procedures related to the timestate data
-- Run as us3admin
--

DELIMITER $$

-- Routines that deal with the timestate for an experiment run

-- INSERTs new timestate information for an experiment
DROP PROCEDURE IF EXISTS new_timestate$$
CREATE PROCEDURE new_timestate ( p_personGUID   CHAR(36),
                                 p_password     VARCHAR(80),
                                 p_experimentID INT,
                                 p_filename     VARCHAR(255),
                                 p_definitions  LONGTEXT )
  MODIFIES SQL DATA

BEGIN

  DECLARE null_field    TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR 1048
    SET null_field = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
 
    -- A user is creating a timestate
    --  filename is the basename of the .tmst binary file
    --  definitions is the accompanying XML as text
    INSERT INTO timestate SET
      experimentID   = p_experimentID,
      filename       = p_filename,
      definitions    = p_definitions,
      lastUpdated    = NOW();
   
    IF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: Attempt to insert NULL value in the timestate table";

    ELSE
      SET @LAST_INSERT_ID  = LAST_INSERT_ID();

      -- Don't update @LAST_INSERT_ID, because the user is not interested in this one

    END IF;
 
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs the existing timestate information for an experiment
DROP PROCEDURE IF EXISTS update_timestate$$
CREATE PROCEDURE update_timestate ( p_personGUID   CHAR(36),
                                    p_password     VARCHAR(80),
                                    p_timestateID  INT,
                                    p_experimentID INT,
                                    p_filename     VARCHAR(255),
                                    p_definitions  LONGTEXT )
  MODIFIES SQL DATA

BEGIN

  DECLARE null_field    TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR 1048
    SET null_field = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  IF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) THEN
 
    -- A user is updating a timestate
    UPDATE timestate SET
      experimentID   = p_experimentID,
      filename       = p_filename,
      definitions    = p_definitions,
      lastUpdated    = NOW()
    WHERE timestateID = p_timestateID;
   
    IF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: Attempt to insert NULL value in the timestate table";

    END IF;
   
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- SELECTs the timestate information for a specified timestate ID
DROP PROCEDURE IF EXISTS get_timestate$$
CREATE PROCEDURE get_timestate ( p_personGUID   CHAR(36),
                                 p_password     VARCHAR(80),
                                 p_timestateID  INT )
  READS SQL DATA

BEGIN

  DECLARE count_timestate   INT;
  DECLARE l_experimentID    INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  -- Get information we need to verify ownership
  SELECT experimentID
  INTO   l_experimentID
  FROM   timestate
  WHERE  timestateID = p_timestateID;
 
  SELECT COUNT(*)
  INTO   count_timestate
  FROM   timestate
  WHERE  timestateID = p_timestateID;

  IF ( count_timestate = 0 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( verify_experiment_permission( p_personGUID, p_password, l_experimentID ) = @OK ) THEN
    -- This is either an admin, or a person inquiring about his own experiment
    SELECT @OK as status;

    SELECT  experimentID, filename, definitions,
            MD5( data ) AS checksum, LENGTH( data ) AS size,
            timestamp2UTC( lastUpdated ) AS UTC_lastUpdated
    FROM    timestate
    WHERE   timestateID = p_timestateID;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- SELECTs the timestate information for a specified experiment ID
DROP PROCEDURE IF EXISTS get_experiment_timestate$$
CREATE PROCEDURE get_experiment_timestate ( p_personGUID   CHAR(36),
                                            p_password     VARCHAR(80),
                                            p_experimentID INT )
  READS SQL DATA

BEGIN

  DECLARE count_timestate INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  SELECT     COUNT(*)
  INTO       count_timestate
  FROM       timestate
  WHERE      experimentID = p_experimentID;

  IF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) != @OK ) THEN
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( count_timestate < 1 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( count_timestate > 1 ) THEN
    SET @US3_LAST_ERRNO = @MORE_THAN_SINGLE_ROW;
    SET @US3_LAST_ERROR = 'MySQL: more than a single row for an experiment';

    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    -- This is either an admin, or a person inquiring about his own experiment
    SELECT @OK as status;

    SELECT  timestateID, filename, definitions,
            MD5( data ) AS checksum, LENGTH( data ) AS size,
            timestamp2UTC( lastUpdated ) AS UTC_lastUpdated
    FROM    timestate
    WHERE   experimentID = p_experimentID
    ORDER BY timestateID
    LIMIT 1;

  END IF;

END$$

-- DELETEs an individual timestate
DROP PROCEDURE IF EXISTS delete_timestate$$
CREATE PROCEDURE delete_timestate ( p_personGUID   CHAR(36),
                                    p_password     VARCHAR(80),
                                    p_timestateID  INT )
  MODIFIES SQL DATA

BEGIN
  DECLARE count_timestates INT;
  DECLARE l_experimentID   INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  -- Get information we need to verify ownership
  SELECT experimentID
  INTO   l_experimentID
  FROM   timestate
  WHERE  timestateID = p_timestateID;

  IF ( verify_experiment_permission( p_personGUID, p_password, l_experimentID ) = @OK ) THEN

    DELETE FROM timestate
    WHERE       timestateID   = p_timestateID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs a timestate record with the binary data
DROP PROCEDURE IF EXISTS upload_timestate$$
CREATE PROCEDURE upload_timestate ( p_personGUID   CHAR(36),
                                    p_password     VARCHAR(80),
                                    p_timestateID  INT,
                                    p_data         LONGBLOB,
                                    p_checksum     CHAR(33) )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_checksum     CHAR(33);
  DECLARE l_experimentID INT;
  DECLARE not_found      TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR NOT FOUND
    SET not_found = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  -- Compare checksum with calculated checksum
  SET l_checksum = MD5( p_data );
  SET @DEBUG = CONCAT( l_checksum , ' ', p_checksum );

  -- Get information we need to verify ownership
  SELECT experimentID
  INTO   l_experimentID
  FROM   timestate
  WHERE  timestateID = p_timestateID;

  IF ( l_checksum != p_checksum ) THEN

    -- Checksums don't match; abort
    SET @US3_LAST_ERRNO = @BAD_CHECKSUM;
    SET @US3_LAST_ERROR = "MySQL: Transmission error, bad checksum";

  ELSEIF ( verify_experiment_permission( p_personGUID, p_password, l_experimentID ) = @OK ) THEN
 
    -- This is either an admin, or a person inquiring about his own experiment
    UPDATE timestate SET
      data           = p_data
    WHERE  timestateID = p_timestateID;

    IF ( not_found = 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_RAWDATA;
      SET @US3_LAST_ERROR = "MySQL: No raw data with that ID exists";

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- SELECTs a timestate record of binary data previously saved with upload_timestate
DROP PROCEDURE IF EXISTS download_timestate$$
CREATE PROCEDURE download_timestate ( p_personGUID   CHAR(36),
                                      p_password     VARCHAR(80),
                                      p_timestateID  INT )
  READS SQL DATA

BEGIN
  DECLARE l_count_timestate INT;
  DECLARE l_experimentID    INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  -- Get information to verify that there are records
  SELECT COUNT(*)
  INTO   l_count_timestate
  FROM   timestate
  WHERE  timestateID = p_timestateID;

SET @DEBUG = CONCAT('timestateID = ', p_timestateID,
                    'Count = ', l_count_timestate );
  -- Get information we need to verify ownership
  SELECT experimentID
  INTO   l_experimentID
  FROM   timestate
  WHERE  timestateID = p_timestateID;

  IF ( l_count_timestate != 1 ) THEN
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

    SELECT data, MD5( data )
    FROM   timestate
    WHERE  timestateID = p_timestateID;

  END IF;

END$$

