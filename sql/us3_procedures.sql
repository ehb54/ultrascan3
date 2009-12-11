--
-- us3_procedures.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
-- Run as us3admin
--

DELIMITER $$

-- Called by other procedures to set up environment
DROP PROCEDURE IF EXISTS config$$
CREATE PROCEDURE config()
BEGIN
  SET @SQL_MODE       = 'traditional';
  SET @ADMIN_EMAIL    = 'dzollars@gmail.com';
  SET @USERNAME       = LEFT( USER(), LOCATE( '@', USER() ) - 1 );

-- Some error codes
  SET @OK             = 0;
  SET @ERROR          = 1;

  SET @DUP_EMAIL      = 101;
  SET @NO_ACCT        = 102;
  SET @INACTIVE       = 103;
  SET @BADPASS        = 104;

  SET @NOTPERMITTED   = 201;

  SET @NOROWS         = 301;

END$$

-- Returns the most recent error number
DROP FUNCTION IF EXISTS last_errno$$
CREATE FUNCTION last_errno()
  RETURNS INT

BEGIN
  RETURN( @US3_LAST_ERRNO );

END$$

-- Returns the most recent error text message
DROP FUNCTION IF EXISTS last_error$$
CREATE FUNCTION last_error()
  RETURNS TEXT

BEGIN
  RETURN( @US3_LAST_ERROR );

END$$

-- Returns the first automatically generated value successfully
--  inserted for an AUTO_INCREMENT column in the most recent
--  INSERT statement, or 0 if no rows were inserted
DROP FUNCTION IF EXISTS last_insert_id$$
CREATE FUNCTION last_insert_id()
  RETURNS TEXT

BEGIN
  RETURN( @LAST_INSERT_ID );

END$$

-- Checks the user with the passed email and password
DROP FUNCTION IF EXISTS check_user$$
CREATE FUNCTION check_user( l_email VARCHAR(63),
                            p_password VARCHAR(80) )
  RETURNS TINYINT
  READS SQL DATA

BEGIN
  DECLARE count_user INT;
  DECLARE md5_pw VARCHAR(80);
  DECLARE l_password VARCHAR(80);
  DECLARE l_activated INT;
  DECLARE status TINYINT;

  call config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
  SET @US3_ID         = NULL;
  SET @FNAME          = NULL;
  SET @LNAME          = NULL;
  SET @PHONE          = NULL;
  SET @EMAIL          = NULL;
  SET @USERLEVEL      = NULL;

  SET md5_pw          = MD5( p_password );
  SET status          = false;

  SELECT COUNT(*)
  INTO   count_user
  FROM   people
  WHERE  email = l_email;

  IF ( count_user > 1 ) THEN
    SET @US3_LAST_ERRNO = @DUP_EMAIL;
    SET @US3_LAST_ERROR = CONCAT( 'MySQL: There was a problem with duplicate email addresses. ',
                                  'Please contact the administrator: ',
                                  @ADMIN_EMAIL );

  ELSEIF ( count_user < 1 ) THEN
    SET @US3_LAST_ERRNO = @NO_ACCT;
    SET @US3_LAST_ERROR = CONCAT( 'MySQL: The account for ',
                                  l_email,
                                  ' is not set up correctly. ',
                                  'Please contact the administrator: ',
                                  @ADMIN_EMAIL );

  ELSE
    /* At this point we should have exactly 1 record */
    SELECT personID, password, fname, lname, phone, userlevel, activated
    INTO   @US3_ID, l_password, @FNAME, @LNAME, @PHONE, @USERLEVEL, l_activated
    FROM   people
    WHERE  email = l_email;

    SET @EMAIL        = l_email;

    IF ( l_password != md5_pw ) THEN
      SET @US3_LAST_ERRNO = @BADPASS;
      SET @US3_LAST_ERROR = 'MySQL: Invalid password';

      SET @US3_ID     = NULL;
      SET @FNAME      = NULL;
      SET @LNAME      = NULL;
      SET @PHONE      = NULL;
      SET @EMAIL      = NULL;
      SET @USERLEVEL  = NULL;

    ELSEIF ( l_activated = false ) THEN
      SET @US3_LAST_ERRNO = @INACTIVE;
      SET @US3_LAST_ERROR = CONCAT( 'MySQL: This account has not been activated yet. ',
                                    'Please activate your account first. ',
                                    'The activation code was sent to your e-mail address: ',
                                     l_email);
      SET @US3_ID     = NULL;
      SET @FNAME      = NULL;
      SET @LNAME      = NULL;
      SET @PHONE      = NULL;
      SET @EMAIL      = NULL;
      SET @USERLEVEL  = NULL;

    ELSE
      /* Successful login */
      UPDATE people
      SET    lastLogin = NOW()
      WHERE  personID = @US3_ID;

      SET status      = true;

    END IF;

  END IF;

  RETURN( status );

END$$

-- Verifies that user with id is the same as the
--  logged in user
DROP FUNCTION IF EXISTS verify_user$$
CREATE FUNCTION verify_user( l_email VARCHAR(63),
                             p_password VARCHAR(80) )
  RETURNS TINYINT
  READS SQL DATA

BEGIN
  DECLARE count_user   INT;
  DECLARE status       TINYINT;

  CALL config();

  SET status = true;
  IF ( @US3_ID IS NULL ) THEN
    SET status = check_user( l_email, p_password );
  END IF;

  RETURN( status );

END$$

-- Finds the user with the passed email and password,
--  returns ID, firstName, lastName, phone, and userLevel
DROP PROCEDURE IF EXISTS get_user_info$$
CREATE PROCEDURE get_user_info( l_email VARCHAR(63),
                                p_password VARCHAR(80) )
  READS SQL DATA

BEGIN
  CALL config();

  IF ( verify_user( l_email, p_password ) ) THEN
    SELECT @US3_ID AS ID, 
           @FNAME AS firstName, 
           @LNAME AS lastName, 
           @PHONE AS phone, 
           @EMAIL AS email,
           @USERLEVEL AS userLevel;
  END IF;

END$$

-- Verifies that the user with id is associated with
--  the specified buffer
DROP FUNCTION IF EXISTS verify_buffer_owner$$
CREATE FUNCTION verify_buffer_owner( l_email VARCHAR(63),
                                     p_password VARCHAR(80),
                                     l_bufferID INT )
  RETURNS TINYINT
  READS SQL DATA

BEGIN
  
  DECLARE count_buffer   INT;
  DECLARE status         TINYINT;

  CALL config();

  IF ( verify_user( l_email, p_password ) ) THEN
    SELECT COUNT(*)
    INTO   count_buffer
    FROM   bufferPerson
    WHERE  bufferID = l_bufferID
    AND    personID = @US3_ID;
 
    SET status = false;
    IF ( count_buffer = 1 ) THEN
      SET status = true;
    ELSE
      SET @US3_LAST_ERRNO = @NOT_PERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to modify this buffer';
    END IF;

  END IF;

  RETURN( status );

END$$

-- Returns the count of buffers associated with id
DROP FUNCTION IF EXISTS count_buffers$$
CREATE FUNCTION count_buffers( l_email VARCHAR(63),
                               p_password VARCHAR(80) )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE l_count_buffers INT;

  CALL config();

  SET l_count_buffers = 0;

  IF ( verify_user( l_email, p_password ) ) THEN
    SELECT COUNT(*)
    INTO l_count_buffers
    FROM bufferPerson
    WHERE personID   = @US3_ID;
  END IF;

  RETURN( l_count_buffers );

END$$

-- INSERTs a new buffer with the specified information
DROP PROCEDURE IF EXISTS new_buffer$$
CREATE PROCEDURE new_buffer ( l_email VARCHAR(63),
                              p_password VARCHAR(80),
                              l_description TEXT,
                              l_density FLOAT,
                              l_viscosity FLOAT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_bufferID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  IF ( verify_user( l_email, p_password ) ) THEN
    INSERT INTO buffer SET
      description = l_description,
      density     = l_density,
      viscosity   = l_viscosity;

    SET @LAST_INSERT_ID = LAST_INSERT_ID();

    INSERT INTO bufferPerson SET
      bufferID    = @LAST_INSERT_ID,
      personID    = @US3_ID;

  END IF;

END$$

-- UPDATEs an existing buffer with the specified information
DROP PROCEDURE IF EXISTS update_buffer$$
CREATE PROCEDURE update_buffer ( l_email VARCHAR(63),
                                 p_password VARCHAR(80),
                                 l_bufferID INT,
                                 l_description TEXT,
                                 l_density FLOAT,
                                 l_viscosity FLOAT )
  MODIFIES SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_buffer_owner( l_email, p_password, l_bufferID ) ) THEN
    UPDATE buffer SET
      description = l_description,
      density     = l_density,
      viscosity   = l_viscosity
    WHERE bufferID = l_bufferID;

  END IF;
      
END$$

-- SELECTs all buffers associated with the id
DROP PROCEDURE IF EXISTS get_buffer$$
CREATE PROCEDURE get_buffer ( l_email VARCHAR(63),
                              p_password VARCHAR(80) )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( l_email, p_password ) ) THEN
    IF ( count_buffers( l_email, p_password ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
    ELSE
      SELECT buffer.bufferID, description, density, viscosity
      FROM buffer, bufferPerson
      WHERE buffer.bufferID = bufferPerson.bufferID
      AND bufferPerson.personID = @US3_ID;
 
    END IF;

  END IF;

END$$

-- INSERTs information for a new experiment
DROP PROCEDURE IF EXISTS new_experiment$$
CREATE PROCEDURE new_experiment( l_email VARCHAR(63),
                                 p_password VARCHAR(80),
                                 l_exp_type ENUM('velocity', 'equilibrium', 'other'),
                                 l_rotorID INT,
                                 l_date DATE,
                                 l_label VARCHAR(80),
                                 l_comment TEXT)
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( l_email, p_password ) ) THEN
    INSERT INTO experiment SET
      type        = l_exp_type,
      rotorID     = l_rotorID,
      dateBegin   = l_date,
      label       = l_label,
      comment     = l_comment;

      SET @LAST_INSERT_ID = LAST_INSERT_ID();

  END IF;
 
END$$

DELIMITER ;
