--
-- us3_noise_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are related to the noise tables
-- Run as us3admin
--

DELIMITER $$

-- Verifies that the user has permission to view or modify
--  the specified noise
DROP FUNCTION IF EXISTS verify_noise_permission$$
CREATE FUNCTION verify_noise_permission( p_personGUID  CHAR(36),
                                         p_password    VARCHAR(80),
                                         p_noiseID     INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE count_noise       INT;
  DECLARE count_permissions INT;
  DECLARE status            INT;

  CALL config();
  SET status   = @ERROR;
  SET @US3_LAST_ERROR = 'MySQL: error verifying noise permission';

  SELECT COUNT(*)
  INTO   count_noise
  FROM   noise
  WHERE  noiseID = p_noiseID;

  SELECT COUNT(*)
  INTO   count_permissions
  FROM   noise, modelPerson
  WHERE  noise.noiseID = p_noiseID
  AND    noise.modelID = modelPerson.modelID
  AND    personID = @US3_ID;

  IF ( count_noise = 0 ) THEN
    SET @US3_LAST_ERRNO = @NO_NOISE;
    SET @US3_LAST_ERROR = 'MySQL: the specified noise file does not exist';

    SET status = @NO_NOISE;

  ELSEIF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    SET @US3_LAST_ERRNO = @OK;
    SET @US3_LAST_ERROR = '';

    SET status = @OK;

  ELSEIF ( ( verify_user( p_personGUID, p_password ) = @OK ) &&
           ( count_permissions > 0                         ) ) THEN
    SET @US3_LAST_ERRNO = @OK;
    SET @US3_LAST_ERROR = '';

    SET status = @OK;

  ELSE
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view or modify this noise file';

    SET status = @NOTPERMITTED;

  END IF;

  RETURN( status );

END$$

-- Returns the count of noise files associated with p_ID
--  If p_ID = 0, retrieves count of all noise files in db
--  Regular user can only get count of his own noise files
DROP FUNCTION IF EXISTS count_noise$$
CREATE FUNCTION count_noise( p_personGUID CHAR(36),
                              p_password VARCHAR(80),
                              p_ID       INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_noise INT;

  CALL config();
  SET count_noise = 0;

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( p_ID > 0 ) THEN
      SELECT COUNT(*)
      INTO   count_noise
      FROM   modelPerson, noise
      WHERE  personID = p_ID
      AND    modelPerson.modelID = noise.modelID;

    ELSE
      SELECT COUNT(*)
      INTO   count_noise
      FROM   noise;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view those noise files';
     
    ELSE
      -- This person is asking about his own noise files
      -- Ignore p_ID and return user's own
      SELECT COUNT(*)
      INTO   count_noise
      FROM   modelPerson, noise
      WHERE  modelPerson.modelID = noise.modelID
      AND    personID = @US3_ID;

    END IF;
    
  END IF;

  RETURN( count_noise );

END$$

-- Returns the count of noise files associated with p_ID with a given editedDataID
--  p_ID cannot be 0
--  Admin user can get count of noise files that belong to others
DROP FUNCTION IF EXISTS count_noise_by_editID$$
CREATE FUNCTION count_noise_by_editID( p_personGUID CHAR(36),
                                       p_password VARCHAR(80),
                                       p_ID         INT,
                                       p_editID     INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_noise INT;

  CALL config();
  SET count_noise = 0;

  IF ( p_ID <= 0 ) THEN
    -- Gotta have a real ID
    RETURN ( 0 );
    
  ELSEIF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    SELECT COUNT(*)
    INTO   count_noise
    FROM   modelPerson, noise
    WHERE  personID = p_ID
    AND    modelPerson.modelID = noise.modelID
    AND    editedDataID        = p_editID;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view those noise files';
     
    ELSE
      -- This person is asking about his own noise files
      -- Ignore p_ID and return user's own
      SELECT COUNT(*)
      INTO   count_noise
      FROM   modelPerson, noise
      WHERE  personID = @US3_ID
      AND    modelPerson.modelID = noise.modelID
      AND    editedDataID        = p_editID;

    END IF;
    
  END IF;

  RETURN( count_noise );

END$$

-- INSERTs a new noise file with the specified information
DROP PROCEDURE IF EXISTS new_noise$$
CREATE PROCEDURE new_noise ( p_personGUID    CHAR(36),
                             p_password      VARCHAR(80),
                             p_noiseGUID     CHAR(36),
                             p_editedDataID  INT(11),
                             p_modelID       INT(11),
                             p_modelGUID     CHAR(36),
                             p_noiseType     ENUM( 'ri_noise', 'ti_noise' ),
                             p_description   TEXT,
                             p_xml           TEXT )         -- an xml file
  MODIFIES SQL DATA

BEGIN
  DECLARE l_editID          INT;
  DECLARE l_modelID         INT;
  DECLARE l_modelID_count   INT;
  DECLARE l_modelGUID       CHAR(36);
  DECLARE l_modelGUID_count INT;

  DECLARE duplicate_key TINYINT DEFAULT 0;
  DECLARE null_field    TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR 1062
    SET duplicate_key = 1;

  DECLARE CONTINUE HANDLER FOR 1048
    SET null_field = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  -- Verify that the editedDataID exists, and we have permissions
  SET l_editID = 1;         -- default to special "unassigned" record
  IF ( verify_editData_permission( p_personGUID, p_password, p_editedDataID ) = @OK ) THEN
    SET l_editID = p_editedDataID;

  END IF;

  -- User can specify either modelID or modelGUID
  SELECT COUNT(*)
  INTO   l_modelGUID_count
  FROM   model
  WHERE  modelGUID = p_modelGUID
  LIMIT  1;                         -- should be exactly 1, if p_modelGUID is supplied

  SELECT COUNT(*)
  INTO   l_modelID_count
  FROM   model
  WHERE  modelID = p_modelID
  LIMIT  1;                         -- should be exactly 1, if p_modelID is supplied

  IF ( l_modelGUID_count = 1 ) THEN -- prefer the GUID
    SET l_modelGUID = p_modelGUID;

    SELECT modelID
    INTO   l_modelID
    FROM   model
    WHERE  modelGUID = p_modelGUID
    LIMIT  1;

  ELSEIF ( l_modelID_count = 1 ) THEN
    SET l_modelID = p_modelID;

    SELECT modelGUID
    INTO   l_modelGUID
    FROM   model
    WHERE  modelID = p_modelID;

  ELSE
    -- noise file doesn't belong to any model
    SET l_modelID   = 0;
    SET l_modelGUID = p_modelGUID;

  END IF;

  IF ( verify_model_permission( p_personGUID, p_password, l_modelID ) != @OK ) THEN
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( l_modelGUID_count = 0 && l_modelID_count = 0 ) THEN
    SET @US3_LAST_ERROR = "MySQL: the model was not found in the database";
    SET @US3_LAST_ERRNO = @NO_MODEL;

    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( check_GUID ( p_personGUID, p_password, p_noiseGUID ) = @OK ) THEN
    INSERT INTO noise SET
      noiseGUID    = p_noiseGUID,
      editedDataID = l_editID,
      modelID      = l_modelID,
      modelGUID    = l_modelGUID,
      noiseType    = p_noiseType,
      description  = p_description,
      xml          = p_xml,
      timeEntered  = NOW();

    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for noiseGUID field";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: NULL value for noiseGUID field";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing noise with the specified information
DROP PROCEDURE IF EXISTS update_noise$$
CREATE PROCEDURE update_noise ( p_personGUID    CHAR(36),
                                p_password      VARCHAR(80),
                                p_noiseID       INT(11),
                                p_noiseGUID     CHAR(36),
                                p_editedDataID  INT(11),
                                p_modelID       INT(11),
                                p_modelGUID     CHAR(36),
                                p_noiseType     ENUM( 'ri_noise', 'ti_noise' ),
                                p_description   TEXT,
                                p_xml           TEXT )         -- an xml file
  MODIFIES SQL DATA

BEGIN
  DECLARE l_editID          INT;
  DECLARE l_modelID         INT;
  DECLARE l_modelID_count   INT;
  DECLARE l_modelGUID       CHAR(36);
  DECLARE l_modelGUID_count INT;

  DECLARE duplicate_key TINYINT DEFAULT 0;
  DECLARE null_field    TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR 1062
    SET duplicate_key = 1;

  DECLARE CONTINUE HANDLER FOR 1048
    SET null_field = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  -- Verify that the editedDataID exists, and we have permissions
  SET l_editID = 1;         -- default to special "unassigned" record
  IF ( verify_editData_permission( p_personGUID, p_password, p_editedDataID ) = @OK ) THEN
    SET l_editID = p_editedDataID;

  END IF;

  -- User can specify either modelID or modelGUID
  SELECT COUNT(*)
  INTO   l_modelGUID_count
  FROM   model
  WHERE  modelGUID = p_modelGUID
  LIMIT  1;                         -- should be exactly 1, if p_modelGUID is supplied

  SELECT COUNT(*)
  INTO   l_modelID_count
  FROM   model
  WHERE  modelID = p_modelID
  LIMIT  1;                         -- should be exactly 1, if p_modelID is supplied

  IF ( l_modelGUID_count = 1 ) THEN -- prefer the GUID
    SET l_modelGUID = p_modelGUID;

    SELECT modelID
    INTO   l_modelID
    FROM   model
    WHERE  modelGUID = p_modelGUID
    LIMIT  1;

  ELSEIF ( l_modelID_count = 1 ) THEN
    SET l_modelID = p_modelID;

    SELECT modelGUID
    INTO   l_modelGUID
    FROM   model
    WHERE  modelID = p_modelID;

  ELSE
    -- noise file doesn't belong to any model
    SET l_modelID   = 0;
    SET l_modelGUID = p_modelGUID;

  END IF;

  IF ( verify_model_permission( p_personGUID, p_password, l_modelID ) != @OK ) THEN
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( l_modelGUID_count = 0 && l_modelID_count = 0 ) THEN
    SET @US3_LAST_ERROR = "MySQL: the model was not found in the database";
    SET @US3_LAST_ERRNO = @NO_MODEL;

    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( check_GUID ( p_personGUID, p_password, p_noiseGUID ) = @OK ) THEN
    UPDATE noise SET
      noiseGUID    = p_noiseGUID,
      editedDataID = l_editID,
      modelID      = l_modelID,
      modelGUID    = l_modelGUID,
      noiseType    = p_noiseType,
      description  = p_description,
      xml          = p_xml,
      timeEntered  = NOW()
    WHERE noiseID  = p_noiseID;

    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for noiseGUID field";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: NULL value for noiseGUID field";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns the noiseID associated with the given noiseGUID
DROP PROCEDURE IF EXISTS get_noiseID$$
CREATE PROCEDURE get_noiseID ( p_personGUID    CHAR(36),
                               p_password      VARCHAR(80),
                               p_noiseGUID     CHAR(36) )
  READS SQL DATA

BEGIN

  DECLARE count_noise INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET count_noise    = 0;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN

    SELECT    COUNT(*)
    INTO      count_noise
    FROM      noise
    WHERE     noiseGUID = p_noiseGUID;

    IF ( TRIM( p_noiseGUID ) = '' ) THEN
      SET @US3_LAST_ERRNO = @EMPTY;
      SET @US3_LAST_ERROR = CONCAT( 'MySQL: The noiseGUID parameter to the ',
                                    'get_noiseID function cannot be empty' );

    ELSEIF ( count_noise < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   noiseID
      FROM     noise
      WHERE    noiseGUID = p_noiseGUID;

    END IF;

  END IF;

END$$

-- Returns the noiseID and description of all noise files associated with p_ID
--  If p_ID = 0, retrieves information about all noise files in db
--  Regular user can only get info about his own noise files
DROP PROCEDURE IF EXISTS get_noise_desc$$
CREATE PROCEDURE get_noise_desc ( p_personGUID CHAR(36),
                                  p_password   VARCHAR(80),
                                  p_ID         INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( count_noise( p_personGUID, p_password, p_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;
  
      IF ( p_ID > 0 ) THEN
        SELECT   noiseID, noiseGUID, editedDataID, noise.modelID, noiseType, modelGUID,
                 timestamp2UTC( timeEntered ) AS UTC_timeEntered,
                 MD5( xml ) AS checksum, LENGTH( xml ) AS size, description
        FROM     modelPerson, noise
        WHERE    modelPerson.modelID  = noise.modelID
        AND      modelPerson.personID = p_ID
        ORDER BY timeEntered DESC;

      ELSE
        SELECT   noiseID, noiseGUID, editedDataID, noise.modelID, noiseType, modelGUID,
                 timestamp2UTC( timeEntered ) AS UTC_timeEntered,
                 MD5( xml ) AS checksum, LENGTH( xml ) AS size, description
        FROM     modelPerson, noise
        WHERE    modelPerson.modelID  = noise.modelID
        ORDER BY timeEntered DESC;

      END IF;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this noise';
     
      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( count_noise( p_personGUID, p_password, @US3_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      -- Ok, user wants his own info
      SELECT @OK AS status;

      SELECT   noiseID, noiseGUID, editedDataID, noise.modelID, noiseType, modelGUID,
               timestamp2UTC( timeEntered ) AS UTC_timeEntered,
               MD5( xml ) AS checksum, LENGTH( xml ) AS size, description
      FROM     modelPerson, noise
      WHERE    modelPerson.modelID  = noise.modelID
      AND      modelPerson.personID = @US3_ID
      ORDER BY timeEntered DESC;

    END IF;

  END IF;

END$$

-- Returns the noiseID and description of all noise files associated with p_ID and editID
--  Unlike get_noise_desc, in this function p_ID cannot be 0
--  Admin can view p_ID/editID combinations that belong to anyone
DROP PROCEDURE IF EXISTS get_noise_desc_by_editID$$
CREATE PROCEDURE get_noise_desc_by_editID ( p_personGUID CHAR(36),
                                            p_password   VARCHAR(80),
                                            p_ID         INT,
                                            p_editID     INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( p_ID <= 0 ) THEN
    -- Gotta have a real ID
    SET @US3_LAST_ERRNO = @EMPTY;
    SET @US3_LAST_ERROR = 'MySQL: The ID cannot be 0';

    SELECT @US3_LAST_ERRNO AS status;
    
  ELSEIF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( count_noise_by_editID( p_personGUID, p_password, p_ID, p_editID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;
  
      SELECT   noiseID, noiseGUID, editedDataID, noise.modelID, noiseType, modelGUID,
               timestamp2UTC( timeEntered ) AS UTC_timeEntered,
               MD5( xml ) AS checksum, LENGTH( xml ) AS size, description
      FROM     modelPerson, noise
      WHERE    modelPerson.personID = p_ID
      AND      modelPerson.modelID  = noise.modelID
      AND      editedDataID         = p_editID
      ORDER BY timeEntered DESC;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this noise';
     
      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( count_noise_by_editID( p_personGUID, p_password, @US3_ID, p_editID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      -- Ok, user wants his own info
      SELECT @OK AS status;

      SELECT   noiseID, noiseGUID, editedDataID, noise.modelID, noiseType, modelGUID,
               timestamp2UTC( timeEntered ) AS UTC_timeEntered,
               MD5( xml ) AS checksum, LENGTH( xml ) AS size, description
      FROM     modelPerson, noise
      WHERE    modelPerson.personID = @US3_ID
      AND      modelPerson.modelID  = noise.modelID
      AND      editedDataID         = p_editID
      ORDER BY timeEntered DESC;

    END IF;

  END IF;

END$$

-- Returns a more complete list of information about one noise file
DROP PROCEDURE IF EXISTS get_noise_info$$
CREATE PROCEDURE get_noise_info ( p_personGUID  CHAR(36),
                                  p_password    VARCHAR(80),
                                  p_noiseID     INT )
  READS SQL DATA

BEGIN
  DECLARE count_noise INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_noise
  FROM       noise
  WHERE      noiseID = p_noiseID;

  IF ( verify_noise_permission( p_personGUID, p_password, p_noiseID ) = @OK ) THEN
    IF ( count_noise = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   noiseGUID, editedDataID, modelID, modelGUID, noiseType, xml,
               timestamp2UTC( timeEntered ) AS UTC_timeEntered,
               MD5( xml ) AS checksum, LENGTH( xml ) AS size, description
      FROM     noise 
      WHERE    noiseID = p_noiseID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- DELETEs a noise file
DROP PROCEDURE IF EXISTS delete_noise$$
CREATE PROCEDURE delete_noise ( p_personGUID  CHAR(36),
                                p_password    VARCHAR(80),
                                p_noiseID     INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_noise_permission( p_personGUID, p_password, p_noiseID ) = @OK ) THEN

    DELETE FROM noise
    WHERE noiseID = p_noiseID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$
