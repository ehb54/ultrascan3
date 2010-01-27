--
-- us3_people_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are procedures related to information about people
-- Run as us3admin
--

DELIMITER $$

-- Finds the user with the passed email and password,
--  returns ID, firstName, lastName, phone, and userLevel
DROP PROCEDURE IF EXISTS get_user_info$$
CREATE PROCEDURE get_user_info( p_guid     CHAR(36),
                                p_password VARCHAR(80) )
  READS SQL DATA

BEGIN
  CALL config();

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    SELECT @OK AS status;

    SELECT @US3_ID    AS ID, 
           @FNAME     AS firstName, 
           @LNAME     AS lastName, 
           @PHONE     AS phone, 
           @EMAIL     AS email,
           @USERLEVEL AS userLevel;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;


  END IF;

END$$

-- Returns of count of all records in the database, optionally
--   matching some text in the last name field
DROP FUNCTION IF EXISTS count_people$$
CREATE FUNCTION count_people( p_guid     CHAR(36),
                              p_password VARCHAR(80),
                              p_template VARCHAR(30) )
  RETURNS INT
  READS SQL DATA

BEGIN

  DECLARE template    VARCHAR(40);
  DECLARE count_names INT;

  CALL config();
  SET p_template = TRIM( p_template );

  IF ( LENGTH(p_template) = 0 ) THEN
    SELECT     COUNT(*)
    INTO       count_names
    FROM       people;

  ELSE
    SET template = CONCAT('%', p_template, '%');

    SELECT     COUNT(*)
    INTO       count_names
    FROM       people
    WHERE      lname LIKE template;

  END IF;

  RETURN( count_names );
END$$

-- Lists all ID's and names in the database, optionally matching
--   some text in the last name field
DROP PROCEDURE IF EXISTS get_people$$
CREATE PROCEDURE get_people( p_guid     CHAR(36),
                             p_password VARCHAR(80),
                             p_template VARCHAR(30) )
  READS SQL DATA

BEGIN
  DECLARE template VARCHAR(40);

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET p_template      = TRIM( p_template );

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    IF ( count_people( p_guid, p_password, p_template )  < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSEIF ( LENGTH(p_template) = 0 ) THEN
      SELECT @OK AS status;

      SELECT   personID,
               lname AS lastName,
               fname AS firstName,
               organization
      FROM     people
      ORDER BY lname;

    ELSE
      SELECT @OK AS status;

      SET template = CONCAT('%', p_template, '%');

      SELECT   personID,
               lname AS lastName,
               fname AS firstName,
               organization
      FROM     people
      WHERE    lname LIKE template
      ORDER BY lname, fname;
    
    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- Returns a more complete list of information about one user
DROP PROCEDURE IF EXISTS get_person_info$$
CREATE PROCEDURE get_person_info( p_guid     CHAR(36),
                                  p_password VARCHAR(80),
                                  p_ID       INT )
  READS SQL DATA

BEGIN
  DECLARE count_person INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_person
  FROM       people
  WHERE      personID = p_ID;

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN
    IF ( count_person = 0 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   fname AS firstName,
               lname AS lastName,
               address,
               city,
               state,
               zip,
               phone,
               organization,
               email
      FROM     people
      WHERE    personID = p_ID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$

-- INSERTs a new person with the specified information
DROP PROCEDURE IF EXISTS new_person$$
CREATE PROCEDURE new_person ( p_guid         CHAR(36),
                              p_password     VARCHAR(80),
                              p_fname        VARCHAR(30),
                              p_lname        VARCHAR(30),
                              p_address      VARCHAR(255),
                              p_city         VARCHAR(30),
                              p_state        CHAR(2),
                              p_zip          VARCHAR(10),
                              p_phone        VARCHAR(24),
                              p_new_email    VARCHAR(63),
                              p_new_guid     VARCHAR(63),
                              p_organization VARCHAR(45),
                              p_new_password VARCHAR(80) )
  MODIFIES SQL DATA

BEGIN

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

  IF ( TRIM( p_new_guid ) = '' ) THEN
    SET @US3_LAST_ERRNO = @EMPTY;
    SET @US3_LAST_ERROR = CONCAT( 'MySQL: The new GUID parameter to the new_person ',
                                  'procedure cannot be empty' );

  ELSEIF ( verify_user( p_guid, p_password ) = @OK ) THEN

    -- Should we also check userlevel or something?
    INSERT INTO people SET
           fname        = p_fname,
           lname        = p_lname,
           address      = p_address,
           city         = p_city,
           state        = p_state,
           zip          = p_zip,
           phone        = p_phone,
           email        = p_new_email,
           GUID         = p_new_guid,
           organization = p_organization,
           password     = MD5(p_new_password),
           activated    = true,
           signup       = NOW(),
           lastLogin    = NOW(),
           clusterAuthorizations = 'bcf:alamo:laredo:lonestar:bigred:steele:queenbee',
           userlevel    = 0;

    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for email or GUID field";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: NULL value for a field that cannot be NULL";

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- UPDATEs a person with the specified information
DROP PROCEDURE IF EXISTS update_person$$
CREATE PROCEDURE update_person ( p_guid         CHAR(36),
                                 p_password     VARCHAR(80),
                                 p_ID           INT,
                                 p_fname        VARCHAR(30),
                                 p_lname        VARCHAR(30),
                                 p_address      VARCHAR(255),
                                 p_city         VARCHAR(30),
                                 p_state        CHAR(2),
                                 p_zip          VARCHAR(10),
                                 p_phone        VARCHAR(24),
                                 p_new_email    VARCHAR(63),
                                 p_organization VARCHAR(45),
                                 p_new_password VARCHAR(80) )
  MODIFIES SQL DATA

BEGIN

  DECLARE duplicate_key TINYINT DEFAULT 0;
  DECLARE null_field    TINYINT DEFAULT 0;
  DECLARE not_found     TINYINT DEFAULT 0;

  DECLARE CONTINUE HANDLER FOR 1062
    SET duplicate_key = 1;

  DECLARE CONTINUE HANDLER FOR 1048
    SET null_field = 1;

  DECLARE CONTINUE HANDLER FOR NOT FOUND
    SET not_found = 1;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN

    -- Should we also check userlevel or something?
    UPDATE people SET
           fname        = p_fname,
           lname        = p_lname,
           address      = p_address,
           city         = p_city,
           state        = p_state,
           zip          = p_zip,
           phone        = p_phone,
           email        = p_new_email,
           organization = p_organization,
           password     = MD5(p_new_password),
           activated    = true,
           lastLogin    = NOW()
    WHERE  personID     = p_ID;

    IF ( duplicate_key = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTDUP;
      SET @US3_LAST_ERROR = "MySQL: Duplicate entry for email field";

    ELSEIF ( null_field = 1 ) THEN
      SET @US3_LAST_ERRNO = @INSERTNULL;
      SET @US3_LAST_ERROR = "MySQL: NULL value for a field that cannot be NULL";

    ELSEIF ( not_found = 1 ) THEN
      SET @US3_LAST_ERRNO = 1001;

    ELSE
      SET @LAST_INSERT_ID = LAST_INSERT_ID();

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

-- DELETEs a person (marks inactive)
DROP PROCEDURE IF EXISTS delete_person$$
CREATE PROCEDURE delete_person ( p_guid         CHAR(36),
                                 p_password     VARCHAR(80),
                                 p_ID           INT )
  MODIFIES SQL DATA

BEGIN
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_guid, p_password ) = @OK ) THEN

    -- Should we also check userlevel or something?
    UPDATE people SET
           activated    = false,
           lastLogin    = NOW()
    WHERE  personID     = p_ID;

    SET @LAST_INSERT_ID = LAST_INSERT_ID();

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$

