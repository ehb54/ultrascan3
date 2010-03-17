--
-- us3_exp_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are procedures related to the experiment
-- Run as us3admin
--

DELIMITER $$

-- Verifies that the user has permission to view or modify
--  the specified experiment
DROP FUNCTION IF EXISTS verify_experiment_permission$$
CREATE FUNCTION verify_experiment_permission( p_guid         CHAR(36),
                                              p_password     VARCHAR(80),
                                              p_experimentID INT )
  RETURNS TINYINT
  READS SQL DATA

BEGIN
  DECLARE count_experiments INT;
  DECLARE status            TINYINT;

  CALL config();
  SET status   = @ERROR;

  SELECT COUNT(*)
  INTO   count_experiments
  FROM   experimentPerson
  WHERE  experimentID = p_experimentID
  AND    personID = @US3_ID;
 
  IF ( verify_user( p_guid, p_password ) = @OK &&
       count_experiments > 0 ) THEN
    SET status = @OK;

  ELSEIF ( verify_userlevel( p_guid, p_password, @US3_ADMIN ) = @OK ) THEN
    SET status = @OK;

  ELSE
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view or modify this experiment';

    SET status = @NOTPERMITTED;

  END IF;

  RETURN( status );

END$$

-- Returns the count of experiments associated with p_ID
--  If p_ID = 0, retrieves count of all experiments in db
--  Regular user can only get count of his own experiments
DROP FUNCTION IF EXISTS count_experiments$$
CREATE FUNCTION count_experiments( p_guid         CHAR(36),
                                   p_password     VARCHAR(80),
                                   p_ID           INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_experiments INT;

  CALL config();
  SET count_experiments = 0;

  IF ( verify_userlevel( p_guid, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( p_ID > 0 ) THEN
      SELECT COUNT(*)
      INTO   count_experiments
      FROM   experimentPerson
      WHERE  personID = p_ID;

    ELSE
      SELECT COUNT(*)
      INTO   count_experiments
      FROM   experimentPerson;

    END IF;

  ELSEIF ( verify_user( p_guid, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view those experiments';
     
    ELSE
      -- This person is asking about his own experiments
      -- Ignore p_experimentID and return user's own
      SELECT COUNT(*)
      INTO   count_experiments
      FROM   experimentPerson
      WHERE  personID = @US3_ID;

    END IF;
    
  END IF;

  RETURN( count_experiments );

END$$

-- INSERTs a new experiment with the specified information
DROP PROCEDURE IF EXISTS new_experiment$$
CREATE PROCEDURE new_experiment ( p_guid         CHAR(36),
                                  p_password     VARCHAR(80),
                                  p_projectID    INT,
                                  p_labID        INT,
                                  p_instrumentID INT,
                                  p_operatorID   INT,
                                  p_rotorID      INT,
                                  p_type         ENUM('velocity', 'equilibrium', 'other'),
                                  p_runTemp      FLOAT,
                                  p_label        VARCHAR(80),
                                  p_comment      TEXT,
                                  p_centrifugeProtocol TEXT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_experimentID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    INSERT INTO experiment SET
      projectID          = p_projectID,
      labID              = p_labID,
      instrumentID       = p_instrumentID,
      operatorID         = p_operatorID,
      rotorID            = p_rotorID,
      type               = p_type,
      runTemp            = p_runTemp,
      label              = p_label,
      comment            = p_comment,
      centrifugeProtocol = p_centrifugeProtocol ;

    SET @LAST_INSERT_ID  = LAST_INSERT_ID();

    INSERT INTO experimentPerson SET
      experimentID = @LAST_INSERT_ID,
      personID     = @US3_ID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing experiment with the specified information
DROP PROCEDURE IF EXISTS update_experiment$$
CREATE PROCEDURE update_experiment ( p_guid         CHAR(36),
                                     p_password     VARCHAR(80),
                                     p_experimentID INT,
                                     p_projectID    INT,
                                     p_labID        INT,
                                     p_instrumentID INT,
                                     p_operatorID   INT,
                                     p_rotorID      INT,
                                     p_type         ENUM('velocity', 'equilibrium', 'other'),
                                     p_runTemp      FLOAT,
                                     p_label        VARCHAR(80),
                                     p_comment      TEXT,
                                     p_centrifugeProtocol TEXT )
  MODIFIES SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_experiment_permission( p_guid, p_password, p_experimentID ) = @OK ) THEN
    UPDATE experiment SET
      projectID          = p_projectID,
      labID              = p_labID,
      instrumentID       = p_instrumentID,
      operatorID         = p_operatorID,
      rotorID            = p_rotorID,
      type               = p_type,
      runTemp            = p_runTemp,
      label              = p_label,
      comment            = p_comment,
      centrifugeProtocol = p_centrifugeProtocol,
      dateUpdated        = NOW()
    WHERE experimentID   = p_experimentID;

  END IF;
      
  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns the experimentID, label, and type of all experiments associated with p_ID
--  If p_ID = 0, retrieves information about all experiments in db
--  Regular user can only get info about his own experiments
DROP PROCEDURE IF EXISTS get_experiment_desc$$
CREATE PROCEDURE get_experiment_desc ( p_guid     CHAR(36),
                                       p_password VARCHAR(80),
                                       p_ID       INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_userlevel( p_guid, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( count_experiments( p_guid, p_password, p_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;
  
      IF ( p_ID > 0 ) THEN
        SELECT   e.experimentID, label, type
        FROM     experiment e, experimentPerson p
        WHERE    e.experimentID = p.experimentID
        AND      p.personID = p_ID
        ORDER BY label;
   
      ELSE
        SELECT   e.experimentID, label, type
        FROM     experiment e, experimentPerson p
        WHERE    e.experimentID = p.experimentID
        ORDER BY label;

      END IF;

    END IF;

  ELSEIF ( verify_user( p_guid, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this experiment';
     
      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( count_experiments( p_guid, p_password, @US3_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      -- Ok, user wants his own info
      SELECT @OK AS status;

      SELECT   e.experimentID, label, type
      FROM     experiment e, experimentPerson p
      WHERE    e.experimentID = p.experimentID
      AND      p.personID = @US3_ID
      ORDER BY label;

    END IF;

  END IF;

END$$

-- Returns a more complete list of information about one experiment
DROP PROCEDURE IF EXISTS get_experiment_info$$
CREATE PROCEDURE get_experiment_info ( p_guid         CHAR(36),
                                       p_password     VARCHAR(80),
                                       p_experimentID INT )
  READS SQL DATA

BEGIN
  DECLARE count_experiments INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_experiments
  FROM       experiment
  WHERE      experimentID = p_experimentID;

  IF ( verify_experiment_permission( p_guid, p_password, p_experimentID ) = @OK ) THEN
    IF ( count_experiments = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   projectID, labID, instrumentID, operatorID, rotorID, type,
               runTemp, label, comment, centrifugeProtocol, personID
      FROM     experiment e, experimentPerson ep
      WHERE    e.experimentID = ep.experimentID
      AND      e.experimentID = p_experimentID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- DELETEs an experiment, plus information in related tables
DROP PROCEDURE IF EXISTS delete_experiment$$
CREATE PROCEDURE delete_experiment ( p_guid      CHAR(36),
                                     p_password  VARCHAR(80),
                                     p_experimentID INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_experiment_permission( p_guid, p_password, p_experimentID ) = @OK ) THEN

    DELETE FROM experimentPerson
    WHERE experimentID = p_experimentID;

    DELETE FROM experiment
    WHERE experimentID = p_experimentID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

