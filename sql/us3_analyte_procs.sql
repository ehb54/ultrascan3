--
-- us3_analyte_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are related to the analyte tables
-- Run as us3admin
--

DELIMITER $$

-- Verifies that the user has permission to view or modify
--  the specified analyte
DROP FUNCTION IF EXISTS verify_analyte_permission$$
CREATE FUNCTION verify_analyte_permission( p_personGUID CHAR(36),
                                           p_password   VARCHAR(80),
                                           p_analyteID  INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE count_analytes    INT;
  DECLARE count_permissions INT;
  DECLARE status            INT;

  CALL config();
  SET status   = @ERROR;
  SET @US3_LAST_ERROR = 'MySQL: error verifying analyte permission';

  SELECT COUNT(*)
  INTO   count_analytes
  FROM   analyte
  WHERE  analyteID = p_analyteID;

  SELECT COUNT(*)
  INTO   count_permissions
  FROM   analytePerson
  WHERE  analyteID = p_analyteID
  AND    personID = @US3_ID;
 
  IF ( count_analytes = 0 ) THEN
    SET @US3_LAST_ERRNO = @NO_ANALYTE;
    SET @US3_LAST_ERROR = 'MySQL: the specified analyte does not exist';

    SET status = @NO_ANALYTE;

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
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view or modify this analyte';

    SET status = @NOTPERMITTED;

  END IF;

  RETURN( status );

END$$

-- Returns the count of analytes associated with p_ID
--  If p_ID = 0, retrieves count of all analytes in db
--  Regular user can only get count of his own analytes
DROP FUNCTION IF EXISTS count_analytes$$
CREATE FUNCTION count_analytes( p_personGUID     CHAR(36),
                                p_password VARCHAR(80),
                                p_ID       INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_analytes INT;

  CALL config();
  SET count_analytes = 0;

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( p_ID > 0 ) THEN
      SELECT COUNT(*)
      INTO   count_analytes
      FROM   analytePerson
      WHERE  personID = p_ID;

    ELSE
      SELECT COUNT(*)
      INTO   count_analytes
      FROM   analytePerson;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view those analytes';
     
    ELSE
      -- This person is asking about his own analytes
      -- Ignore p_ID and return user's own
      SELECT COUNT(*)
      INTO   count_analytes
      FROM   analytePerson
      WHERE  personID = @US3_ID;

    END IF;
    
  END IF;

  RETURN( count_analytes );

END$$

-- INSERTs a new analyte with the specified information
DROP PROCEDURE IF EXISTS new_analyte$$
CREATE PROCEDURE new_analyte ( p_personGUID  CHAR(36),
                               p_password    VARCHAR(80),
                               p_analyteGUID CHAR(36),
                               p_type        VARCHAR(16),
                               p_sequence    TEXT,
                               p_vbar        FLOAT,
                               p_description TEXT,
                               p_spectrum    TEXT,
                               p_mweight     FLOAT,
                               p_ownerID     INT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_analyteID INT;

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
 
  IF ( ( verify_user( p_personGUID, p_password ) = @OK ) &&
       ( check_GUID ( p_personGUID, p_password, p_analyteGUID ) = @OK ) ) THEN
    INSERT INTO analyte SET
      analyteGUID        = p_analyteGUID,
      type        = p_type,
      sequence    = p_sequence,
      vbar        = p_vbar,
      description = p_description,
      spectrum    = p_spectrum,
      molecularWeight = p_mweight ;

    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for analyteGUID field";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: NULL value for analyteGUID field";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

      INSERT INTO analytePerson SET
        analyteID   = @LAST_INSERT_ID,
        personID    = p_ownerID;

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs an existing analyte with the specified information
DROP PROCEDURE IF EXISTS update_analyte$$
CREATE PROCEDURE update_analyte ( p_personGUID  CHAR(36),
                                  p_password    VARCHAR(80),
                                  p_analyteID   INT,
                                  p_type        VARCHAR(16),
                                  p_sequence    TEXT,
                                  p_vbar        FLOAT,
                                  p_description TEXT,
                                  p_spectrum    TEXT,
                                  p_mweight     FLOAT )
  MODIFIES SQL DATA

BEGIN
  DECLARE not_found     TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR NOT FOUND
    SET not_found = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_analyte_permission( p_personGUID, p_password, p_analyteID ) = @OK ) THEN
    UPDATE analyte SET
      type        = p_type,
      sequence    = p_sequence,
      vbar        = p_vbar,
      description = p_description,
      spectrum    = p_spectrum,
      molecularWeight = p_mweight 
    WHERE analyteID = p_analyteID;

    IF ( not_found = 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_ANALYTE;
      SET @US3_LAST_ERROR = "MySQL: No analyte with that ID exists";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;
      
  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Returns the analyteID associated with the given analyteGUID
DROP PROCEDURE IF EXISTS get_analyteID$$
CREATE PROCEDURE get_analyteID ( p_personGUID  CHAR(36),
                                 p_password    VARCHAR(80),
                                 p_analyteGUID CHAR(36) )
  READS SQL DATA

BEGIN

  DECLARE count_anal INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET count_anal      = 0;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN

    SELECT    COUNT(*)
    INTO      count_anal
    FROM      analyte
    WHERE     analyteGUID = p_analyteGUID;

    IF ( TRIM( p_analyteGUID ) = '' ) THEN
      SET @US3_LAST_ERRNO = @EMPTY;
      SET @US3_LAST_ERROR = CONCAT( 'MySQL: The analyteGUID parameter to the get_analyteID ',
                                    'function cannot be empty' );

    ELSEIF ( count_anal < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   analyteID
      FROM     analyte
      WHERE    analyteGUID = p_analyteGUID;

    END IF;

  END IF;

END$$

-- Returns the analyteID, description, and type of all analytes associated with p_ID
--  If p_ID = 0, retrieves information about all analytes in db
--  Regular user can only get info about his own analytes
DROP PROCEDURE IF EXISTS get_analyte_desc$$
CREATE PROCEDURE get_analyte_desc ( p_personGUID CHAR(36),
                                    p_password   VARCHAR(80),
                                    p_ID         INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_userlevel( p_personGUID, p_password, @US3_ADMIN ) = @OK ) THEN
    -- This is an admin; he can get more info
    IF ( count_analytes( p_personGUID, p_password, p_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;
  
      IF ( p_ID > 0 ) THEN
        SELECT   a.analyteID, description, type
        FROM     analyte a, analytePerson
        WHERE    a.analyteID = analytePerson.analyteID
        AND      analytePerson.personID = p_ID
        ORDER BY a.analyteID DESC;
   
      ELSE
        SELECT   a.analyteID, description, type
        FROM     analyte a, analytePerson
        WHERE    a.analyteID = analytePerson.analyteID
        ORDER BY a.analyteID DESC;

      END IF;

    END IF;

  ELSEIF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( (p_ID != 0) && (p_ID != @US3_ID) ) THEN
      -- Uh oh, can't do that
      SET @US3_LAST_ERRNO = @NOTPERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to view this analyte';
     
      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( count_analytes( p_personGUID, p_password, @US3_ID ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
   
      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      -- Ok, user wants his own info
      SELECT @OK AS status;

      SELECT   a.analyteID, description, type
      FROM     analyte a, analytePerson
      WHERE    a.analyteID = analytePerson.analyteID
      AND      analytePerson.personID = @US3_ID
      ORDER BY a.analyteID DESC;
      

    END IF;

  END IF;

END$$

-- Returns a more complete list of information about one analyte
DROP PROCEDURE IF EXISTS get_analyte_info$$
CREATE PROCEDURE get_analyte_info ( p_personGUID CHAR(36),
                                    p_password   VARCHAR(80),
                                    p_analyteID  INT )
  READS SQL DATA

BEGIN
  DECLARE count_analytes INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_analytes
  FROM       analyte
  WHERE      analyteID = p_analyteID;

  IF ( verify_analyte_permission( p_personGUID, p_password, p_analyteID ) = @OK ) THEN
    IF ( count_analytes = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   analyteGUID, type, sequence, vbar, description, spectrum, molecularWeight, personID 
      FROM     analyte a, analytePerson ap
      WHERE    a.analyteID = ap.analyteID
      AND      a.analyteID = p_analyteID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- DELETEs a analyte, plus information in related tables
DROP PROCEDURE IF EXISTS delete_analyte$$
CREATE PROCEDURE delete_analyte ( p_personGUID CHAR(36),
                                  p_password   VARCHAR(80),
                                  p_analyteID  INT )
  MODIFIES SQL DATA

BEGIN
  DECLARE count_analytes INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_analyte_permission( p_personGUID, p_password, p_analyteID ) = @OK ) THEN

    -- Find out if this analyte is used in any solution first
    SELECT COUNT(*) INTO count_analytes
    FROM solutionAnalyte
    WHERE analyteID = p_analyteID;

    IF ( count_analytes = 0 ) THEN
    
      DELETE FROM analytePerson
      WHERE analyteID = p_analyteID;
      
      DELETE FROM spectrum
      WHERE componentID = p_analyteID
      AND   componentType = 'Analyte';
      
      DELETE FROM analyte
      WHERE analyteID = p_analyteID;

    ELSE
      SET @US3_LAST_ERRNO = @ANALYTE_IN_USE;
      SET @US3_last_ERROR = 'The analyte is in use in a solution';

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- Procedures regarding nucleotide information

-- Returns complete list of nucleotide information about an analyte
DROP PROCEDURE IF EXISTS get_nucleotide_info$$
CREATE PROCEDURE get_nucleotide_info ( p_personGUID CHAR(36),
                                       p_password   VARCHAR(80),
                                       p_analyteID  INT )
  READS SQL DATA

BEGIN
  DECLARE count_analytes INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_analytes
  FROM       analyte
  WHERE      analyteID = p_analyteID;

  IF ( verify_analyte_permission( p_personGUID, p_password, p_analyteID ) = @OK ) THEN
    IF ( count_analytes = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   doubleStranded, complement, _3prime, _5prime, 
               sodium, potassium, lithium, magnesium, calcium 
      FROM     analyte
      WHERE    analyteID = p_analyteID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- UPDATEs an existing analyte with the specified information
DROP PROCEDURE IF EXISTS set_nucleotide_info$$
CREATE PROCEDURE set_nucleotide_info ( p_personGUID  CHAR(36),
                                       p_password    VARCHAR(80),
                                       p_analyteID   INT,
                                       p_doubleStranded TINYINT,
                                       p_complement  TINYINT,
                                       p__3prime     TINYINT,
                                       p__5prime     TINYINT,
                                       p_sodium      DOUBLE,
                                       p_potassium   DOUBLE,
                                       p_lithium     DOUBLE,
                                       p_magnesium   DOUBLE,
                                       p_calcium     DOUBLE )
  MODIFIES SQL DATA

BEGIN
  DECLARE not_found     TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR NOT FOUND
    SET not_found = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_analyte_permission( p_personGUID, p_password, p_analyteID ) = @OK ) THEN
    UPDATE analyte SET
      doubleStranded = p_doubleStranded,
      complement     = p_complement,
      _3prime        = p__3prime,
      _5prime        = p__5prime,
      sodium         = p_sodium,
      potassium      = p_potassium, 
      lithium        = p_lithium, 
      magnesium      = p_magnesium, 
      calcium        = p_calcium 
    WHERE analyteID = p_analyteID;

    IF ( not_found = 1 ) THEN
      SET @US3_LAST_ERRNO = @NO_ANALYTE;
      SET @US3_LAST_ERROR = "MySQL: No analyte with that ID exists";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;
      
  SELECT @US3_LAST_ERRNO AS status;

END$$

