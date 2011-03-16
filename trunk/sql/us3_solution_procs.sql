--
-- us3_solution_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are procedures related to the solution data
-- Run as us3admin
--

DELIMITER $$

-- Routines that deal with the solutions that are in a channel

-- Verifies that the user has permission to view or modify
--  the specified solution
DROP FUNCTION IF EXISTS verify_solution_permission$$
CREATE FUNCTION verify_solution_permission( p_personGUID   CHAR(36),
                                            p_password     VARCHAR(80),
                                            p_solutionID   INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE count_solutions   INT;
  DECLARE count_permissions INT;
  DECLARE status            INT;

  CALL config();
  SET status   = @ERROR;
  SET @US3_LAST_ERROR = 'MySQL: error verifying solution permission';

  SELECT COUNT(*)
  INTO   count_solutions
  FROM   solution
  WHERE  solutionID = p_solutionID;

  SELECT COUNT(*)
  INTO   count_permissions
  FROM   solutionPerson
  WHERE  solutionID = p_solutionID
  AND    personID = @US3_ID;

  IF ( count_solutions = 0 ) THEN
    SET @US3_LAST_ERRNO = @NO_SOLUTION;
    SET @US3_LAST_ERROR = 'MySQL: the specified solution does not exist';

    SET status = @NO_SOLUTION;

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
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view or modify this solution';

    SET status = @NOTPERMITTED;

  END IF;

  RETURN( status );

END$$

-- Get a list of solutionID's that correspond with an experiment
DROP PROCEDURE IF EXISTS get_solutionIDs$$
CREATE PROCEDURE get_solutionIDs ( p_personGUID   CHAR(36),
                                   p_password     VARCHAR(80),
                                   p_experimentID INT )
  READS SQL DATA

BEGIN
  DECLARE count_solution INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_solution
  FROM       experimentSolutionChannel
  WHERE      experimentID = p_experimentID;

  IF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) THEN
    IF ( count_solution = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   esc.solutionID, s.description 
      FROM     experimentSolutionChannel esc, solution s
      WHERE    esc.experimentID = p_experimentID
      AND      esc.solutionID   = s.solutionID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- Translate a solutionGUID into a solutionID
DROP PROCEDURE IF EXISTS get_solutionID_from_GUID$$
CREATE PROCEDURE get_solutionID_from_GUID ( p_personGUID   CHAR(36),
                                            p_password     VARCHAR(80),
                                            p_solutionGUID  CHAR(36) )
  READS SQL DATA

BEGIN
  DECLARE count_solution  INT;
  DECLARE l_solutionID    INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_solution
  FROM       solution
  WHERE      solutionGUID = p_solutionGUID;

  IF ( count_solution = 0 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    -- Let's get the solutionID so we can verify permission
    SELECT solutionID
    INTO   l_solutionID
    FROM   solution
    WHERE  solutionGUID = p_solutionGUID
    LIMIT  1;                           -- should be only 1

    IF ( verify_solution_permission( p_personGUID, p_password, l_solutionID ) = @OK ) THEN
      SELECT @OK AS status;

      SELECT l_solutionID AS solutionID;

    ELSE
      SELECT @US3_LAST_ERRNO AS status;

    END IF;

  END IF;

END$$

-- Returns the count of solutions associated with an
-- experiment; should be the same as the number of triples,
-- or c/c/w combinations
DROP FUNCTION IF EXISTS count_solutions_by_experiment$$
CREATE FUNCTION count_solutions_by_experiment( p_personGUID   CHAR(36),
                                              p_password     VARCHAR(80),
                                              p_experimentID INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_solutions INT;

  CALL config();
  SET count_solutions = 0;

  IF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) THEN
    -- This is either an admin, or a person inquiring about his own experiment
    SELECT     COUNT(*)
    INTO       count_solutions
    FROM       experimentSolutionChannel
    WHERE      experimentID = p_experimentID;

  END IF;

  RETURN( count_solutions );

END$$

-- Returns the count of solutions associated with a user
DROP FUNCTION IF EXISTS count_solutions$$
CREATE FUNCTION count_solutions( p_personGUID   CHAR(36),
                                 p_password     VARCHAR(80),
                                 p_ID           INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_solutions INT;

  CALL config();
  SET count_solutions = 0;

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( p_ID > 0 ) THEN
      SELECT     COUNT(*)
      INTO       count_solutions
      FROM       solutionPerson
      WHERE      personID = p_ID;

    ELSE
      SELECT     COUNT(*)
      INTO       count_solutions
      FROM       solution;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view those solutions';
     
    ELSE
      -- This person is asking about his own experiments
      -- Ignore p_ID and return user's own
      SELECT     COUNT(*)
      INTO       count_solutions
      FROM       solutionPerson
      WHERE      personID = @US3_ID;

    END IF;
    
  END IF;

  RETURN( count_solutions );

END$$

-- Get a list of all solutionID's that belong to this user
DROP PROCEDURE IF EXISTS all_solutionIDs$$
CREATE PROCEDURE all_solutionIDs ( p_personGUID   CHAR(36),
                                   p_password     VARCHAR(80),
                                   p_ID           INT )
  READS SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( count_solutions( p_personGUID, p_password, p_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;
  
      IF ( p_ID > 0 ) THEN
        SELECT     s.solutionID, description
        FROM       solutionPerson sp, solution s
        WHERE      sp.personID = p_ID
        AND        sp.solutionID = s.solutionID
        ORDER BY   s.description;

      ELSE
        SELECT     s.solutionID, description
        FROM       solutionPerson sp, solution s
        WHERE      sp.solutionID = s.solutionID
        ORDER BY   s.description;

      END IF;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this solution';
     
      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( count_solutions( p_personGUID, p_password, @US3_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      -- Ok, user wants his own info
      SELECT @OK AS status;

      SELECT     s.solutionID, description
      FROM       solutionPerson sp, solution s
      WHERE      sp.personID = @US3_ID
      AND        sp.solutionID = s.solutionID
      ORDER BY   s.description;

    END IF;

  END IF;

END$$

-- INSERTs new solution information about one c/c/w combination in an experiment
DROP PROCEDURE IF EXISTS new_solution$$
CREATE PROCEDURE new_solution ( p_personGUID   CHAR(36),
                                p_password     VARCHAR(80),
                                p_solutionGUID CHAR(36),
                                p_description  VARCHAR(80),
                                p_commonVbar20 DOUBLE,
                                p_storageTemp  FLOAT,
                                p_notes        TEXT,
                                p_experimentID INT,
                                p_channelID    INT,
                                p_ownerID      INT )
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
 
    -- A user is creating a solution
    INSERT INTO solution SET
      solutionGUID   = p_solutionGUID,    -- this needs to be generated by caller
      description    = p_description,
      commonVbar20   = p_commonVbar20,
      storageTemp    = p_storageTemp,
      notes          = p_notes;
   
    IF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: Attempt to insert NULL value in the solution table";

    ELSE
      SET @LAST_INSERT_ID  = LAST_INSERT_ID();

      -- Establish ownership of this solution
      INSERT INTO solutionPerson SET
        solutionID   = @LAST_INSERT_ID,
        personID     = p_ownerID;

      -- Now associate solution with experiment, if we can
      IF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) THEN
        INSERT INTO experimentSolutionChannel SET
          experimentID = p_experimentID,
          solutionID   = @LAST_INSERT_ID,
          channelID    = p_channelID;

      ELSE
        -- just use the default experimentID
        INSERT INTO experimentSolutionChannel SET
          experimentID = 1,
          solutionID   = @LAST_INSERT_ID,
          channelID    = p_channelID;

      END IF;

      -- Don't update @LAST_INSERT_ID, because the user is not interested in this one

    END IF;
   
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs the existing solution information about one c/c/w combination in an experiment
DROP PROCEDURE IF EXISTS update_solution$$
CREATE PROCEDURE update_solution ( p_personGUID   CHAR(36),
                                   p_password     VARCHAR(80),
                                   p_solutionID   INT,
                                   p_solutionGUID CHAR(36),
                                   p_description  VARCHAR(80),
                                   p_commonVbar20 DOUBLE,
                                   p_storageTemp  FLOAT,
                                   p_notes        TEXT )
  MODIFIES SQL DATA

BEGIN

  DECLARE null_field    TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR 1048
    SET null_field = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  IF ( ( verify_solution_permission( p_personGUID, p_password, p_solutionID ) = @OK ) &&
       ( check_GUID( p_personGUID, p_password, p_solutionGUID ) = @OK               ) ) THEN
 
    -- A user is updating a solution
    UPDATE solution SET
      solutionGUID   = p_solutionGUID,    -- this needs to be generated by caller
      description    = p_description,
      commonVbar20   = p_commonVbar20,
      storageTemp    = p_storageTemp,
      notes          = p_notes
    WHERE solutionID = p_solutionID;
   
    IF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: Attempt to insert NULL value in the solution table";

    END IF;
   
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- SELECTs solution information about one c/c/w combination in an experiment
DROP PROCEDURE IF EXISTS get_solution$$
CREATE PROCEDURE get_solution ( p_personGUID   CHAR(36),
                                p_password     VARCHAR(80),
                                p_solutionID   INT )
  READS SQL DATA

BEGIN

  DECLARE count_solution   INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
 
  SELECT     COUNT(*)
  INTO       count_solution
  FROM       solution
  WHERE      solutionID = p_solutionID;

  IF ( count_solution = 0 ) THEN
    SET @US3_LAST_ERRNO = @NOROWS;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( verify_solution_permission( p_personGUID, p_password, p_solutionID ) = @OK ) THEN
    -- This is either an admin, or a person inquiring about his own experiment
    SELECT @OK as status;

    SELECT  solutionGUID, description, commonVbar20, storageTemp, notes
    FROM    solution
    WHERE   solutionID = p_solutionID;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- Creates a new relationship between an experiment and a solution
DROP PROCEDURE IF EXISTS new_experiment_solution$$
CREATE PROCEDURE new_experiment_solution ( p_personGUID   CHAR(36),
                                           p_password     VARCHAR(80),
                                           p_experimentID INT,
                                           p_solutionID   INT,
                                           p_channelID    INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) &&
       ( verify_solution_permission  ( p_personGUID, p_password, p_solutionID   ) = @OK ) ) THEN

    INSERT INTO experimentSolutionChannel SET
      experimentID  = p_experimentID,
      solutionID    = p_solutionID,
      channelID     = p_channelID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- DELETEs all the relationships between any solutions and an experiment
-- Most likely used if you're updating an experiment 
DROP PROCEDURE IF EXISTS delete_experiment_solutions$$
CREATE PROCEDURE delete_experiment_solutions ( p_personGUID   CHAR(36),
                                               p_password     VARCHAR(80),
                                               p_experimentID INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) THEN

    DELETE FROM experimentSolutionChannel
    WHERE       experimentID = p_experimentID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- DELETEs an individual solution, plus information in related tables
DROP PROCEDURE IF EXISTS delete_solution$$
CREATE PROCEDURE delete_solution ( p_personGUID   CHAR(36),
                                   p_password     VARCHAR(80),
                                   p_solutionID   INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_solution_permission( p_personGUID, p_password, p_solutionID ) = @OK ) THEN

    -- Make sure records match if they have related tables or not
    -- Have to do it in a couple of stages because of the constraints
    DELETE FROM solutionBuffer
    WHERE       solutionID   = p_solutionID;

    DELETE FROM solutionAnalyte
    WHERE       solutionID   = p_solutionID;
    
    DELETE FROM solutionPerson
    WHERE       solutionID   = p_solutionID;
    
    DELETE FROM experimentSolutionChannel
    WHERE       solutionID   = p_solutionID;

    DELETE FROM solution
    WHERE       solutionID   = p_solutionID;
    
  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Routines that add buffer and analyte information

-- Adds or replaces the buffer associated with a solution
DROP PROCEDURE IF EXISTS new_solutionBuffer$$
CREATE PROCEDURE new_solutionBuffer( p_personGUID   CHAR(36),
                                     p_password     VARCHAR(80),
                                     p_solutionID   INT,
                                     p_bufferID     INT,
                                     p_bufferGUID   CHAR(36) )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_bufferID        INT;
  DECLARE l_bufferCount     INT;
  DECLARE l_bufferGUIDCount INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT COUNT(*)
  INTO   l_bufferGUIDCount
  FROM   buffer
  WHERE  bufferGUID = p_bufferGUID
  LIMIT  1;                         -- should be exactly 1, if p_bufferGUID is supplied

  SELECT COUNT(*)
  INTO   l_bufferCount
  FROM   buffer
  WHERE  bufferID = p_bufferID
  LIMIT  1;                         -- should be exactly 1, if p_bufferID is supplied

  IF ( verify_solution_permission( p_personGUID, p_password, p_solutionID ) != @OK ) THEN
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( l_bufferGUIDCount = 0 && l_bufferCount = 0 ) THEN
    SET @US3_LAST_ERROR = "MySQL: the buffer was not found in the database";
    SET @US3_LAST_ERRNO = @NO_BUFFER;

    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    IF ( l_bufferGUIDCount = 1 ) THEN
      -- prefer GUID to find record
      SELECT bufferID
      INTO   l_bufferID
      FROM   buffer
      WHERE  bufferGUID = p_bufferGUID
      LIMIT  1;
  
    ELSE
      SET l_bufferID = p_bufferID;

    END IF;

    -- Now that we know we have a buffer, let's delete the old one
    DELETE FROM solutionBuffer
    WHERE       solutionID = p_solutionID;

    -- Adding the new buffer
    INSERT INTO solutionBuffer SET
      solutionID = p_solutionID,
      bufferID   = l_bufferID;

    SELECT @OK AS status;

  END IF;

END$$

-- Adds a new analyte to a solution
DROP PROCEDURE IF EXISTS new_solutionAnalyte$$
CREATE PROCEDURE new_solutionAnalyte( p_personGUID   CHAR(36),
                                      p_password     VARCHAR(80),
                                      p_solutionID   INT,
                                      p_analyteID    INT,
                                      p_analyteGUID  CHAR(36),
                                      p_amount       FLOAT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_analyteID        INT;
  DECLARE l_analyteCount     INT;
  DECLARE l_analyteGUIDCount INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT COUNT(*)
  INTO   l_analyteGUIDCount
  FROM   analyte
  WHERE  analyteGUID = p_analyteGUID
  LIMIT  1;                         -- should be exactly 1, if p_analyteGUID is supplied

  SELECT COUNT(*)
  INTO   l_analyteCount
  FROM   analyte
  WHERE  analyteID = p_analyteID
  LIMIT  1;                         -- should be exactly 1, if p_analyteID is supplied

  IF ( verify_solution_permission( p_personGUID, p_password, p_solutionID ) != @OK ) THEN
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( l_analyteGUIDCount = 0 && l_analyteCount = 0 ) THEN
    SET @US3_LAST_ERROR = "MySQL: the analyte was not found in the database";
    SET @US3_LAST_ERRNO = @NO_ANALYTE;

    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    IF ( l_analyteGUIDCount = 1 ) THEN
      -- prefer GUID to find record
      SELECT analyteID
      INTO   l_analyteID
      FROM   analyte
      WHERE  analyteGUID = p_analyteGUID
      LIMIT  1;
  
    ELSE
      SET l_analyteID = p_analyteID;

    END IF;

    -- First double check if we are permitted to use this analyte
    IF ( verify_analyte_permission( p_personGUID, p_password, l_analyteID ) = @OK ) THEN
      -- Add the new analyte
      INSERT INTO solutionAnalyte SET
        solutionID = p_solutionID,
        analyteID  = l_analyteID,
        amount     = p_amount;
  
      SELECT @OK AS status;

    ELSE
      SET @US3_LAST_ERROR = "MySQL: you are not permitted to use this analyte";
      SET @US3_LAST_ERRNO = @NOTPERMITTED;

      SELECT @US3_LAST_ERRNO AS status;
      
    END IF;

  END IF;

END$$

-- DELETEs all associations between analytes and solutions (not the analytes 
--  themselves). Most likely used if you want to redo the analyte-solutions
DROP PROCEDURE IF EXISTS delete_solutionAnalytes$$
CREATE PROCEDURE delete_solutionAnalytes ( p_personGUID   CHAR(36),
                                           p_password     VARCHAR(80),
                                           p_solutionID   INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_solution_permission( p_personGUID, p_password, p_solutionID ) = @OK ) THEN

    DELETE FROM solutionAnalyte
    WHERE       solutionID = p_solutionID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Retrieves the buffer associated with a solution
DROP PROCEDURE IF EXISTS get_solutionBuffer$$
CREATE PROCEDURE get_solutionBuffer( p_personGUID   CHAR(36),
                                     p_password     VARCHAR(80),
                                     p_solutionID   INT )
  READS SQL DATA

BEGIN
  DECLARE l_bufferCount     INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT COUNT(*)
  INTO   l_bufferCount
  FROM   solutionBuffer
  WHERE  solutionID = p_solutionID
  LIMIT  1;                         -- should be exactly 1

  IF ( verify_solution_permission( p_personGUID, p_password, p_solutionID ) != @OK ) THEN
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( l_bufferCount = 0 ) THEN
    SET @US3_LAST_ERROR = "MySQL: the buffer association was not found in the database";
    SET @US3_LAST_ERRNO = @NO_BUFFER;

    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    
    SELECT @OK AS status;

    SELECT   buffer.bufferID, bufferGUID, description
    FROM     solutionBuffer, buffer
    WHERE    solutionBuffer.solutionID = p_solutionID
    AND      solutionBuffer.bufferID   = buffer.bufferID
    ORDER BY description
    LIMIT    1;                     -- should be exactly 1

  END IF;

END$$

-- Retrieves the analyte associated with a solution
DROP PROCEDURE IF EXISTS get_solutionAnalyte$$
CREATE PROCEDURE get_solutionAnalyte( p_personGUID   CHAR(36),
                                      p_password     VARCHAR(80),
                                      p_solutionID   INT )
  READS SQL DATA

BEGIN
  DECLARE l_analyteCount     INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT COUNT(*)
  INTO   l_analyteCount
  FROM   solutionAnalyte
  WHERE  solutionID = p_solutionID
  LIMIT  1;                         -- should be exactly 1

  IF ( verify_solution_permission( p_personGUID, p_password, p_solutionID ) != @OK ) THEN
    SELECT @US3_LAST_ERRNO AS status;

  ELSEIF ( l_analyteCount = 0 ) THEN
    SET @US3_LAST_ERROR = "MySQL: the analyte association was not found in the database";
    SET @US3_LAST_ERRNO = @NO_ANALYTE;

    SELECT @US3_LAST_ERRNO AS status;

  ELSE
    
    SELECT @OK AS status;

    SELECT   analyte.analyteID, analyteGUID, description, amount, molecularWeight, vbar
    FROM     solutionAnalyte, analyte
    WHERE    solutionAnalyte.solutionID = p_solutionID
    AND      solutionAnalyte.analyteID   = analyte.analyteID
    ORDER BY description;

  END IF;

END$$

