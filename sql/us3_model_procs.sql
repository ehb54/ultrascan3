--
-- us3_model_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are related to the model tables
-- Run as us3admin
--

DELIMITER $$

-- Verifies that the user has permission to view or modify
--  the specified model
DROP FUNCTION IF EXISTS verify_model_permission$$
CREATE FUNCTION verify_model_permission( p_guid      CHAR(36),
                                         p_password  VARCHAR(80),
                                         p_modelID   INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE count_models INT;
  DECLARE status       INT;

  CALL config();
  SET status   = @ERROR;

  SELECT COUNT(*)
  INTO   count_models
  FROM   modelPerson
  WHERE  modelID = p_modelID
  AND    personID = @US3_ID;
 
  IF ( verify_user( p_guid, p_password ) = @OK &&
       count_models > 0 ) THEN
    SET status = @OK;

  ELSEIF ( verify_userlevel( p_guid, p_password, @US3_ADMIN ) = @OK ) THEN
    SET status = @OK;

  ELSE
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view or modify this model';

    SET status = @NOTPERMITTED;

  END IF;

  RETURN( status );

END$$

-- Returns the count of models associated with p_ID
--  If p_ID = 0, retrieves count of all models in db
--  Regular user can only get count of his own models
DROP FUNCTION IF EXISTS count_models$$
CREATE FUNCTION count_models( p_guid     CHAR(36),
                              p_password VARCHAR(80),
                              p_ID       INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_models INT;

  CALL config();
  SET count_models = 0;

  IF ( verify_userlevel( p_guid, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( p_ID > 0 ) THEN
      SELECT COUNT(*)
      INTO   count_models
      FROM   modelPerson
      WHERE  personID = p_ID;

    ELSE
      SELECT COUNT(*)
      INTO   count_models
      FROM   modelPerson;

    END IF;

  ELSEIF ( verify_user( p_guid, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view those models';
     
    ELSE
      -- This person is asking about his own models
      -- Ignore p_ID and return user's own
      SELECT COUNT(*)
      INTO   count_models
      FROM   modelPerson
      WHERE  personID = @US3_ID;

    END IF;
    
  END IF;

  RETURN( count_models );

END$$

-- INSERTs a new model with the specified information
DROP PROCEDURE IF EXISTS new_model$$
CREATE PROCEDURE new_model ( p_guid        CHAR(36),
                             p_password    VARCHAR(80),
                             p_modelGUID   CHAR(36),
                             p_description TEXT,
                             p_contents    TEXT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_modelID INT;

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
 
  IF ( ( verify_user( p_guid, p_password ) = @OK ) &&
       ( check_GUID ( p_guid, p_password, p_modelGUID ) = @OK ) ) THEN
    INSERT INTO model SET
      GUID        = p_modelGUID,
      description = p_description,
      contents    = p_contents;

    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for GUID field";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: NULL value for GUID field";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

      INSERT INTO modelPerson SET
        modelID   = @LAST_INSERT_ID,
        personID  = @US3_ID;

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing model with the specified information
DROP PROCEDURE IF EXISTS update_model$$
CREATE PROCEDURE update_model ( p_guid        CHAR(36),
                                p_password    VARCHAR(80),
                                p_modelID     INT,
                                p_description TEXT,
                                p_contents    TEXT )
  MODIFIES SQL DATA

BEGIN
  DECLARE not_found     TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR NOT FOUND
    SET not_found = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_model_permission( p_guid, p_password, p_modelID ) = @OK ) THEN
    UPDATE model SET
      description = p_description,
      contents    = p_contents
    WHERE modelID = p_modelID;

    IF ( not_found = 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_MODEL;
      SET @US3_LAST_ERROR = "MySQL: No model with that ID exists";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;
      
  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns the modelID associated with the given modelGUID
DROP PROCEDURE IF EXISTS get_modelID$$
CREATE PROCEDURE get_modelID ( p_guid        CHAR(36),
                               p_password    VARCHAR(80),
                               p_modelGUID   CHAR(36) )
  READS SQL DATA

BEGIN

  DECLARE count_models INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET count_models    = 0;

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN

    SELECT    COUNT(*)
    INTO      count_models
    FROM      model
    WHERE     GUID = p_modelGUID;

    IF ( TRIM( p_modelGUID ) = '' ) THEN
      SET @US3_LAST_ERRNO = @EMPTY;
      SET @US3_LAST_ERROR = CONCAT( 'MySQL: The modelGUID parameter to the get_modelID ',
                                    'function cannot be empty' );

    ELSEIF ( count_models < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   modelID
      FROM     model
      WHERE    GUID = p_modelGUID;

    END IF;

  END IF;

END$$

-- Returns the modelID and description of all models associated with p_ID
--  If p_ID = 0, retrieves information about all models in db
--  Regular user can only get info about his own models
DROP PROCEDURE IF EXISTS get_model_desc$$
CREATE PROCEDURE get_model_desc ( p_guid     CHAR(36),
                                  p_password VARCHAR(80),
                                  p_ID       INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_userlevel( p_guid, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( count_models( p_guid, p_password, p_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;
  
      IF ( p_ID > 0 ) THEN
        SELECT   m.modelID, GUID, description
        FROM     model m, modelPerson
        WHERE    m.modelID = modelPerson.modelID
        AND      modelPerson.personID = p_ID
        ORDER BY m.modelID DESC;
   
      ELSE
        SELECT   m.modelID, GUID, description
        FROM     model m, modelPerson
        WHERE    m.modelID = modelPerson.modelID
        ORDER BY m.modelID DESC;

      END IF;

    END IF;

  ELSEIF ( verify_user( p_guid, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this model';
     
      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( count_models( p_guid, p_password, @US3_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      -- Ok, user wants his own info
      SELECT @OK AS status;

      SELECT   m.modelID, GUID, description
      FROM     model m, modelPerson
      WHERE    m.modelID = modelPerson.modelID
      AND      modelPerson.personID = @US3_ID
      ORDER BY m.modelID DESC;
      

    END IF;

  END IF;

END$$

-- Returns a more complete list of information about one model
DROP PROCEDURE IF EXISTS get_model_info$$
CREATE PROCEDURE get_model_info ( p_guid      CHAR(36),
                                  p_password  VARCHAR(80),
                                  p_modelID   INT )
  READS SQL DATA

BEGIN
  DECLARE count_models INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_models
  FROM       model
  WHERE      modelID = p_modelID;

  IF ( verify_model_permission( p_guid, p_password, p_modelID ) = @OK ) THEN
    IF ( count_models = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   GUID, description, contents, personID 
      FROM     model m, modelPerson mp
      WHERE    m.modelID = mp.modelID
      AND      m.modelID = p_modelID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- DELETEs a model, plus information in related tables
DROP PROCEDURE IF EXISTS delete_model$$
CREATE PROCEDURE delete_model ( p_guid      CHAR(36),
                                p_password  VARCHAR(80),
                                p_modelID   INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_model_permission( p_guid, p_password, p_modelID ) = @OK ) THEN

    DELETE FROM modelPerson
    WHERE modelID = p_modelID;

    DELETE FROM model
    WHERE modelID = p_modelID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$
