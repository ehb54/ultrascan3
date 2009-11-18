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
  SET @OK             = 0;
  SET @ERROR          = 1;

END$$

-- Returns the most recent error number
DROP FUNCTION IF EXISTS last_errno$$
CREATE FUNCTION last_errno()
  RETURNS INT

BEGIN
  RETURN( $US3_LAST_ERRNO );

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


-- Finds user with same username as mysql user,
--  returns ID, status, firstName, lastName, phone,
--  email and userLevel
DROP PROCEDURE IF EXISTS check_user$$
CREATE PROCEDURE check_user()
  READS SQL DATA

BEGIN
  /* At this point we know that the user has been set up as a mysql user or we
     wouldn't even be here. However, we need to interface with our database
     records too.
  */

  DECLARE count_user INT;
  DECLARE id INT;
  DECLARE l_fname VARCHAR(30); 
  DECLARE l_lname VARCHAR(30);
  DECLARE l_phone VARCHAR(24);
  DECLARE l_email VARCHAR(63);
  DECLARE l_userlevel INT;
  DECLARE l_activated INT;

  call config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;

  SET id              = NULL;
  SET l_fname         = NULL;
  SET l_lname         = NULL;
  SET l_phone         = NULL;
  SET l_email         = NULL;
  SET l_userlevel     = NULL;

  SELECT COUNT(*)
  INTO count_user
  FROM people
  WHERE username = @USERNAME;

  IF ( count_user > 1 ) THEN
    SET @US3_LAST_ERRNO = @ERROR;
    SET @US3_LAST_ERROR = CONCAT( 'MySQL: There was a problem with duplicate user names. ',
                                  'Please contact the administrator: ',
                                  @ADMIN_EMAIL );

  ELSEIF ( count_user < 1 ) THEN
    SET @US3_LAST_ERRNO = @ERROR;
    SET @US3_LAST_ERROR = CONCAT( 'MySQL: The account for ',
                                  @USERNAME,
                                  ' is not set up correctly. ',
                                  'Please contact the administrator: ',
                                  @ADMIN_EMAIL );

  ELSE
    /* At this point we should have exactly 1 record */
    SELECT personID, fname, lname, phone, email, userlevel, activated
    INTO   id, l_fname, l_lname, l_phone, l_email, l_userlevel, l_activated
    FROM people
    WHERE username = @USERNAME;

    IF ( l_activated = false ) THEN
      SET @US3_LAST_ERRNO = @ERROR;
      SET @US3_LAST_ERROR = CONCAT( 'MySQL: This account has not been activated yet. ',
                                    'Please activate your account first. ',
                                    'The activation code was sent to your e-mail address: ',
                                     l_email);
      SET id          = NULL;
      SET l_fname     = NULL;
      SET l_lname     = NULL;
      SET l_phone     = NULL;
      SET l_email     = NULL;
      SET l_userlevel = NULL;

    ELSE
      /* Successful login */
      UPDATE people
      SET lastLogin = NOW()
      WHERE personID = id;

      SELECT id AS ID, 
             @US3_LAST_ERRNO AS status, 
             l_fname AS firstName, 
             l_lname AS lastName, 
             l_phone AS phone, 
             l_email AS email, 
             l_userlevel AS userLevel;

    END IF;

  END IF;

END$$

-- Verifies that user with id is the same as the
--  mysql logged in user
DROP FUNCTION IF EXISTS verify_user$$
CREATE FUNCTION verify_user( id INT )
  RETURNS TINYINT
  READS SQL DATA

BEGIN
  
  DECLARE count_user   INT;
  DECLARE status       TINYINT;

  CALL config();

  SELECT COUNT(*)
  INTO count_user
  FROM people
  WHERE personID = id
  AND username = @USERNAME;

  SET status = false;
  IF ( count_user = 1 ) THEN
    SET status = true;
  END IF;

  RETURN( status );

END$$

-- Verifies that the user with id is associated with
--  the specified buffer
DROP FUNCTION IF EXISTS verify_buffer_owner$$
CREATE FUNCTION verify_buffer_owner( id INT,
                                     l_bufferID INT )
  RETURNS TINYINT
  READS SQL DATA

BEGIN
  
  DECLARE count_buffer   INT;
  DECLARE status         TINYINT;

  CALL config();

  SELECT COUNT(*)
  INTO   count_buffer
  FROM   bufferPerson
  WHERE  bufferID = l_bufferID
  AND    personID = id;

  SET status = false;
  IF ( count_buffer = 1 ) THEN
    SET status = true;
  END IF;

  RETURN( status );

END$$

-- Returns the count of buffers associated with id
DROP FUNCTION IF EXISTS count_buffers$$
CREATE FUNCTION count_buffers( id INT )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE l_count_buffers   INT;

  CALL config();

  SELECT COUNT(*)
  INTO l_count_buffers
  FROM bufferPerson
  WHERE personID   = id;

  RETURN( l_count_buffers );

END$$

-- INSERTs a new buffer with the specified information
DROP PROCEDURE IF EXISTS save_buffer$$
CREATE PROCEDURE save_buffer ( id INT,
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

  IF ( ! verify_user( id ) ) THEN
    SET @US3_LAST_ERRNO = @ERROR;
    SET @US3_LAST_ERROR = 'MySQL: user name does not match ID';

  ELSE
    INSERT INTO buffer SET
      description = l_description,
      density     = l_density,
      viscosity   = l_viscosity;

    SET @LAST_INSERT_ID = LAST_INSERT_ID();

    INSERT INTO bufferPerson SET
      bufferID    = @LAST_INSERT_ID,
      personID    = id;

  END IF;

END$$

-- UPDATEs an existing buffer with the specified information
DROP PROCEDURE IF EXISTS update_buffer$$
CREATE PROCEDURE update_buffer ( id INT,
                                 l_bufferID INT,
                                 l_description TEXT,
                                 l_density FLOAT,
                                 l_viscosity FLOAT )
  MODIFIES SQL DATA

BEGIN

--  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( ! verify_user( id ) ) THEN
    SET @US3_LAST_ERRNO = @ERROR;
    SET @US3_LAST_ERROR = 'MySQL: user name does not match ID';

  ELSEIF ( ! verify_buffer_owner( id, l_bufferID ) ) THEN
    SET @US3_LAST_ERRNO = @ERROR;
    SET @US3_LAST_ERROR = 'MySQL: you do not have permission to modify this buffer';

  ELSE
    UPDATE buffer SET
      description = l_description,
      density     = l_density,
      viscosity   = l_viscosity
    WHERE bufferID = l_bufferID;

  END IF;
      
END$$

-- SELECTs all buffers associated with the id
DROP PROCEDURE IF EXISTS get_buffer$$
CREATE PROCEDURE get_buffer ( id INT )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( ! verify_user( id ) ) THEN
    SET @US3_LAST_ERRNO = @ERROR;
    SET @US3_LAST_ERROR = 'MySQL: user name does not match ID';

  ELSEIF ( count_buffers( id ) < 1 ) THEN
    SET @US3_LAST_ERRNO = @ERROR;
    SET @US3_LAST_ERROR = 'MySQL: no rows returned';

  ELSE
    SELECT buffer.bufferID, description, density, viscosity
    FROM buffer, bufferPerson
    WHERE buffer.bufferID = bufferPerson.bufferID
    AND bufferPerson.personID = id;

  END IF;

END$$

DELIMITER ;
