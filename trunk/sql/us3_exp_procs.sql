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
CREATE FUNCTION verify_experiment_permission( p_personGUID   CHAR(36),
                                              p_password     VARCHAR(80),
                                              p_experimentID INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE count_experiments INT;
  DECLARE status            INT;

  CALL config();
  SET status   = @ERROR;

  SELECT COUNT(*)
  INTO   count_experiments
  FROM   experimentPerson
  WHERE  experimentID = p_experimentID
  AND    personID = @US3_ID;
 
  IF ( verify_user( p_personGUID, p_password ) = @OK &&
       count_experiments > 0 ) THEN
    SET status = @OK;

  ELSEIF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    SET status = @OK;

  ELSE
    SET @US3_LAST_ERRNO = @NOTPERMITTED;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view or modify this experiment';

    SET status = @NOTPERMITTED;

  END IF;

  RETURN( status );

END$$

-- Verifies that the operator has permission to operate the instrument,
--  and that the instrument is in the right lab
DROP FUNCTION IF EXISTS verify_operator_permission$$
CREATE FUNCTION verify_operator_permission( p_personGUID   CHAR(36),
                                            p_password     VARCHAR(80),
                                            p_labID        INT,
                                            p_instrumentID INT,
                                            p_operatorID   INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE count_instruments INT;
  DECLARE count_labs        INT;
  DECLARE status            INT;

  CALL config();
  SET @US3_LAST_ERRNO = @ERROR;
  SET @US3_LAST_ERROR = '';

  -- Check permits for this instrument and this operator
  SELECT COUNT(*)
  INTO   count_instruments
  FROM   permits
  WHERE  personID     = p_operatorID 
  AND    instrumentID = p_instrumentID;

  -- Check location of instrument
  SELECT COUNT(*)
  INTO   count_labs
  FROM   instrument
  WHERE  instrumentID = p_instrumentID
  AND    labID        = p_labID;
 
  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( count_instruments < 1 ) THEN
      SET @US3_LAST_ERRNO = @BADOPERATOR;
      SET @US3_LAST_ERROR = 'MySQL: operator is not permitted to work on this instrument';

    ELSEIF ( count_labs < 1 ) THEN
      SET @US3_LAST_ERRNO = @BADLABLOCATION;
      SET @US3_LAST_ERROR = 'MySQL: that instrument is not located in that lab';

    ELSE
      SET @US3_LAST_ERRNO = @OK;

    END IF;

  END IF;

  SET status = @US3_LAST_ERRNO;

  RETURN( status );

END$$

-- Returns the count of experiments associated with p_ID
--  If p_ID = 0, retrieves count of all experiments in db
--  Regular user can only get count of his own experiments
DROP FUNCTION IF EXISTS count_experiments$$
CREATE FUNCTION count_experiments( p_personGUID   CHAR(36),
                                   p_password     VARCHAR(80),
                                   p_ID           INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_experiments INT;

  CALL config();
  SET count_experiments = 0;

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
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

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
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
CREATE PROCEDURE new_experiment ( p_personGUID   CHAR(36),
                                  p_password     VARCHAR(80),
                                  p_expGUID      CHAR(36),
                                  p_projectID    INT,
                                  p_runID        VARCHAR(80),
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
  DECLARE l_count_runID  INT;
  DECLARE duplicate_key  TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR 1062
    SET duplicate_key = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  IF ( ( verify_user( p_personGUID, p_password ) = @OK           )   &&
       ( verify_operator_permission( p_personGUID, p_password, 
          p_labID, p_instrumentID, p_operatorID ) = @OK          )   &&
       ( check_GUID( p_personGUID, p_password, p_expGUID ) = @OK ) ) THEN
    -- Can't have duplicate run ID's for this investigator
    SELECT COUNT(*)
    INTO   l_count_runID
    FROM   experimentPerson p, experiment e
    WHERE  p.experimentID = e.experimentID
    AND    e.runID = p_runID
    AND    p.personID = @US3_ID;

    IF ( l_count_runID > 0 ) THEN
      SET @US3_LAST_ERRNO = @DUPFIELD;
      SET @US3_LAST_ERROR = "MySQL: Duplicate runID in experiment table";

    ELSE
      INSERT INTO experiment SET
        projectID          = p_projectID,
        experimentGUID     = p_expGUID,
        runID              = p_runID,
        labID              = p_labID,
        instrumentID       = p_instrumentID,
        operatorID         = p_operatorID,
        rotorID            = p_rotorID,
        type               = p_type,
        runTemp            = p_runTemp,
        label              = p_label,
        comment            = p_comment,
        centrifugeProtocol = p_centrifugeProtocol,
        dateUpdated        = NOW() ;
  
      IF ( duplicate_key = 1 ) THEN
        SET @US3_LAST_ERRNO = @INSERTDUP;
        SET @US3_LAST_ERROR = "MySQL: Duplicate entry for experimentGUID field";

      ELSE
        SET @LAST_INSERT_ID  = LAST_INSERT_ID();
    
        INSERT INTO experimentPerson SET
          experimentID = @LAST_INSERT_ID,
          personID     = @US3_ID;

      END IF;

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing experiment with the specified information
DROP PROCEDURE IF EXISTS update_experiment$$
CREATE PROCEDURE update_experiment ( p_personGUID   CHAR(36),
                                     p_password     VARCHAR(80),
                                     p_experimentID INT,
                                     p_expGUID      CHAR(36),
                                     p_projectID    INT,
                                     p_runID        VARCHAR(80),
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
  DECLARE l_count_runID  INT;
  DECLARE duplicate_key  TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR 1062
    SET duplicate_key = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) &&
       ( verify_operator_permission( p_personGUID, p_password, 
          p_labID, p_instrumentID, p_operatorID ) = @OK )                                 &&
       ( check_GUID( p_personGUID, p_password, p_expGUID ) = @OK ) ) THEN
    -- Let's make sure we don't result in a duplicate run ID's for this investigator
    --  through renaming or some such
    SELECT COUNT(*)
    INTO   l_count_runID
    FROM   experimentPerson p, experiment e
    WHERE  p.experimentID = e.experimentID
    AND    e.runID = p_runID
    AND    e.experimentID != p_experimentID
    AND    p.personID = @US3_ID;

    IF ( l_count_runID > 0 ) THEN
      SET @US3_LAST_ERRNO = @DUPFIELD;
      SET @US3_LAST_ERROR = "MySQL: Duplicate runID in experiment table";

    ELSE
      UPDATE experiment SET
        experimentGUID     = p_expGUID,
        projectID          = p_projectID,
        runID              = p_runID,
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

      IF ( duplicate_key = 1 ) THEN
        SET @US3_LAST_ERRNO = @INSERTDUP;
        SET @US3_LAST_ERROR = "MySQL: Duplicate entry for experimentGUID field";

      END IF;

    END IF;

  END IF;
      
  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns the experimentID, runID, and type of all experiments associated with p_ID
--  If p_ID = 0, retrieves information about all experiments in db
--  Regular user can only get info about his own experiments
DROP PROCEDURE IF EXISTS get_experiment_desc$$
CREATE PROCEDURE get_experiment_desc ( p_personGUID CHAR(36),
                                       p_password   VARCHAR(80),
                                       p_ID         INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( count_experiments( p_personGUID, p_password, p_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;
  
      IF ( p_ID > 0 ) THEN
        SELECT   e.experimentID, runID, type
        FROM     experiment e, experimentPerson p
        WHERE    e.experimentID = p.experimentID
        AND      p.personID = p_ID
        ORDER BY runID;
   
      ELSE
        SELECT   e.experimentID, runID, type
        FROM     experiment e, experimentPerson p
        WHERE    e.experimentID = p.experimentID
        ORDER BY runID;

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

      SELECT   e.experimentID, runID, type
      FROM     experiment e, experimentPerson p
      WHERE    e.experimentID = p.experimentID
      AND      p.personID = @US3_ID
      ORDER BY runID;

    END IF;

  END IF;

END$$

-- Returns a more complete list of information about one experiment
DROP PROCEDURE IF EXISTS get_experiment_info$$
CREATE PROCEDURE get_experiment_info ( p_personGUID   CHAR(36),
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

  IF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) THEN
    IF ( count_experiments = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   experimentGUID, projectID, runID, labID, instrumentID, 
               operatorID, rotorID, type, runTemp, label, comment, 
               centrifugeProtocol, timestamp2UTC( dateUpdated) AS UTC_dateUpdated, 
               personID
      FROM     experiment e, experimentPerson ep
      WHERE    e.experimentID = ep.experimentID
      AND      e.experimentID = p_experimentID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- Returns a more complete list of information about one experiment
-- Differs from previous procedure by searching by runID
DROP PROCEDURE IF EXISTS get_experiment_info_by_runID$$
CREATE PROCEDURE get_experiment_info_by_runID ( p_personGUID CHAR(36),
                                                p_password   VARCHAR(80),
                                                p_runID      VARCHAR(80) )
  READS SQL DATA

BEGIN
  DECLARE count_experiments INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    SELECT COUNT(*)
    INTO   count_experiments
    FROM   experiment e, experimentPerson p
    WHERE  e.experimentID = p.experimentID
    AND    p.personID     = @US3_ID
    AND    e.runID        = p_runID;

    IF ( count_experiments = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT projectID, e.experimentID, experimentGUID, labID, instrumentID, 
             operatorID, rotorID, type, runTemp, label, comment, 
             centrifugeProtocol, dateUpdated, personID
      FROM   experiment e, experimentPerson p
      WHERE  e.experimentID = p.experimentID
      AND    p.personID     = @US3_ID
      AND    e.runID        = p_runID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- DELETEs an experiment, plus information in related tables
DROP PROCEDURE IF EXISTS delete_experiment$$
CREATE PROCEDURE delete_experiment ( p_personGUID   CHAR(36),
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
    DELETE      noise
    FROM        rawData
    LEFT JOIN   editedData  ON ( editedData.rawDataID = rawData.rawDataID )
    LEFT JOIN   model       ON ( model.editedDataID   = editedData.editedDataID )
    LEFT JOIN   noise       ON ( noise.modelID        = model.modelID )
    WHERE       rawData.experimentID = p_experimentID;

    DELETE      model, modelPerson
    FROM        rawData
    LEFT JOIN   editedData  ON ( editedData.rawDataID = rawData.rawDataID )
    LEFT JOIN   model       ON ( model.editedDataID   = editedData.editedDataID )
    LEFT JOIN   modelPerson ON ( modelPerson.modelID  = model.modelID )
    WHERE       rawData.experimentID = p_experimentID;

    DELETE      editedData
    FROM        rawData
    LEFT JOIN   editedData  ON ( editedData.rawDataID = rawData.rawDataID )
    WHERE       rawData.experimentID = p_experimentID;

    DELETE FROM rawData
    WHERE       experimentID = p_experimentID;

    DELETE FROM experimentPerson
    WHERE experimentID = p_experimentID;

    DELETE FROM experiment
    WHERE experimentID = p_experimentID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- adds a new cell table record, relating an experiment to the cell and centerpiece info
DROP PROCEDURE IF EXISTS new_cell_experiment$$
CREATE PROCEDURE new_cell_experiment ( p_personGUID   CHAR(36),
                                       p_password     VARCHAR(80),
                                       p_cellGUID     CHAR(36),
                                       p_name         TEXT,
                                       p_abstractCenterpieceID INT,
                                       p_experimentID INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) THEN
    INSERT INTO cell SET
      cellGUID     = p_cellGUID,
      name         = p_name,
      experimentID = p_experimentID,
      abstractCenterpieceID = p_abstractCenterpieceID,
      dateUpdated  = NOW();

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- SELECTs all cell table records relating to an experiment
DROP PROCEDURE IF EXISTS all_cell_experiments$$
CREATE PROCEDURE all_cell_experiments ( p_personGUID   CHAR(36),
                                        p_password     VARCHAR(80),
                                        p_experimentID INT )
  READS SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) THEN
    SELECT @OK AS status;

    SELECT   cellID, cellGUID, name, abstractCenterpieceID
    FROM     cell
    WHERE    experimentID = p_experimentID
    ORDER BY dateUpdated DESC;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- DELETEs all cell table records associated with an experiment
DROP PROCEDURE IF EXISTS delete_cell_experiments$$
CREATE PROCEDURE delete_cell_experiments ( p_personGUID   CHAR(36),
                                           p_password     VARCHAR(80),
                                           p_experimentID INT )
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_experiment_permission( p_personGUID, p_password, p_experimentID ) = @OK ) THEN
    DELETE FROM cell
    WHERE experimentID = p_experimentID;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

