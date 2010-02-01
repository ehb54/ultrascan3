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
  SET @ERROR          = -1;
  SET @NOT_CONNECTED  = 1;

  SET @DUP_EMAIL      = 101;
  SET @NO_ACCT        = 102;
  SET @INACTIVE       = 103;
  SET @BADPASS        = 104;
  SET @EMPTY          = 105;

  SET @NOTPERMITTED   = 201;

  SET @NOROWS         = 301;

  SET @INSERTNULL     = 401;
  SET @INSERTDUP      = 402;

  SET @NO_BUFFER      = 501;
  SET @NO_COMPONENT   = 502;

END$$

-- Returns the most recent error number
DROP FUNCTION IF EXISTS last_errno$$
CREATE FUNCTION last_errno()
  RETURNS INT
  NO SQL

BEGIN
  RETURN( @US3_LAST_ERRNO );

END$$

-- Returns the most recent error text message
DROP FUNCTION IF EXISTS last_error$$
CREATE FUNCTION last_error()
  RETURNS TEXT
  NO SQL

BEGIN
  RETURN( @US3_LAST_ERROR );

END$$

-- Returns the first automatically generated value successfully
--  inserted for an AUTO_INCREMENT column in the most recent
--  INSERT statement, or 0 if no rows were inserted
DROP FUNCTION IF EXISTS last_insertID$$
CREATE FUNCTION last_insertID()
  RETURNS TEXT
  NO SQL

BEGIN
  RETURN( @LAST_INSERT_ID );

END$$

-- Checks the user with the passed GUID and password
DROP FUNCTION IF EXISTS check_user$$
CREATE FUNCTION check_user( p_guid     CHAR(36),
                            p_password VARCHAR(80) )
  RETURNS TINYINT
  READS SQL DATA

BEGIN
  DECLARE count_user INT;
  DECLARE md5_pw VARCHAR(80);
  DECLARE l_password VARCHAR(80);
  DECLARE activated INT;
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
  SET @GUID           = NULL;
  SET @USERLEVEL      = NULL;

  SET md5_pw          = MD5( p_password );
  SET status          = @ERROR;

  SELECT COUNT(*)
  INTO   count_user
  FROM   people
  WHERE  GUID = p_guid;

  IF ( TRIM( p_guid ) = '' ) THEN
    SET @US3_LAST_ERRNO = @EMPTY;
    SET @US3_LAST_ERROR = CONCAT( 'MySQL: The GUID parameter to the check_user ',
                                  'function cannot be empty' );

  ELSEIF ( count_user = 0 ) THEN
    SET @US3_LAST_ERRNO = @NO_ACCT;
    SET @US3_LAST_ERROR = CONCAT( 'MySQL: The account identified by GUID ',
                                  p_guid,
                                  ' is not set up correctly. ',
                                  'Please contact the administrator: ',
                                  @ADMIN_EMAIL );

  ELSE
    /* At this point we should have exactly 1 record */
    SELECT personID, password, fname, lname, phone, email, userlevel, activated
    INTO   @US3_ID, l_password, @FNAME, @LNAME, @PHONE, @EMAIL, @USERLEVEL, activated
    FROM   people
    WHERE  GUID = p_guid;

    SET @GUID         = p_guid;

    IF ( l_password != md5_pw ) THEN
      SET @US3_LAST_ERRNO = @BADPASS;
      SET @US3_LAST_ERROR = 'MySQL: Invalid password';

      SET @US3_ID     = NULL;
      SET @FNAME      = NULL;
      SET @LNAME      = NULL;
      SET @PHONE      = NULL;
      SET @EMAIL      = NULL;
      SET @GUID       = NULL;
      SET @USERLEVEL  = NULL;

    ELSEIF ( activated = false ) THEN
      SET @US3_LAST_ERRNO = @INACTIVE;
      SET @US3_LAST_ERROR = CONCAT( 'MySQL: This account has not been activated yet. ',
                                    'Please activate your account first. ',
                                    'The activation code was sent to your e-mail address: ',
                                     p_email);
      SET @US3_ID     = NULL;
      SET @FNAME      = NULL;
      SET @LNAME      = NULL;
      SET @PHONE      = NULL;
      SET @EMAIL      = NULL;
      SET @GUID       = NULL;
      SET @USERLEVEL  = NULL;

    ELSE
      -- Successful login
      UPDATE people
      SET    lastLogin = NOW()
      WHERE  personID = @US3_ID;

      SET status      = @OK;

    END IF;

  END IF;

  RETURN( status );

END$$

-- Checks the user with the passed email and password
DROP FUNCTION IF EXISTS check_user_email$$
CREATE FUNCTION check_user_email( p_email VARCHAR(63),
                                  p_password VARCHAR(80) )
  RETURNS TINYINT
  READS SQL DATA

BEGIN
  DECLARE count_user INT;
  DECLARE md5_pw VARCHAR(80);
  DECLARE l_password VARCHAR(80);
  DECLARE activated INT;
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
  SET @GUID           = NULL;
  SET @USERLEVEL      = NULL;

  SET md5_pw          = MD5( p_password );
  SET status          = @ERROR;

  SELECT COUNT(*)
  INTO   count_user
  FROM   people
  WHERE  email = p_email;

  IF ( TRIM( p_email ) = '' ) THEN
    SET @US3_LAST_ERRNO = @EMPTY;
    SET @US3_LAST_ERROR = CONCAT( 'MySQL: The email parameter to the check_user_email ',
                                  'function cannot be empty' );

  ELSEIF ( count_user > 1 ) THEN
    SET @US3_LAST_ERRNO = @DUP_EMAIL;
    SET @US3_LAST_ERROR = CONCAT( 'MySQL: There was a problem with duplicate email addresses. ',
                                  'Please contact the administrator: ',
                                  @ADMIN_EMAIL );

  ELSEIF ( count_user < 1 ) THEN
    SET @US3_LAST_ERRNO = @NO_ACCT;
    SET @US3_LAST_ERROR = CONCAT( 'MySQL: The account for ',
                                  p_email,
                                  ' is not set up correctly. ',
                                  'Please contact the administrator: ',
                                  @ADMIN_EMAIL );

  ELSE
    /* At this point we should have exactly 1 record */
    SELECT personID, password, fname, lname, phone, GUID, userlevel, activated
    INTO   @US3_ID, l_password, @FNAME, @LNAME, @PHONE, @GUID, @USERLEVEL, activated
    FROM   people
    WHERE  email = p_email;

    SET @EMAIL        = p_email;

    IF ( l_password != md5_pw ) THEN
      SET @US3_LAST_ERRNO = @BADPASS;
      SET @US3_LAST_ERROR = 'MySQL: Invalid password';

      SET @US3_ID     = NULL;
      SET @FNAME      = NULL;
      SET @LNAME      = NULL;
      SET @PHONE      = NULL;
      SET @EMAIL      = NULL;
      SET @GUID       = NULL;
      SET @USERLEVEL  = NULL;

    ELSEIF ( activated = false ) THEN
      SET @US3_LAST_ERRNO = @INACTIVE;
      SET @US3_LAST_ERROR = CONCAT( 'MySQL: This account has not been activated yet. ',
                                    'Please activate your account first. ',
                                    'The activation code was sent to your e-mail address: ',
                                     p_email);
      SET @US3_ID     = NULL;
      SET @FNAME      = NULL;
      SET @LNAME      = NULL;
      SET @PHONE      = NULL;
      SET @EMAIL      = NULL;
      SET @GUID       = NULL;
      SET @USERLEVEL  = NULL;

    ELSE
      -- Successful login
      UPDATE people
      SET    lastLogin = NOW()
      WHERE  personID = @US3_ID;

      SET status      = @OK;

    END IF;

  END IF;

  RETURN( status );

END$$

-- Verifies that user identified by guid is the same as the
--  logged in user
DROP FUNCTION IF EXISTS verify_user$$
CREATE FUNCTION verify_user( p_guid     CHAR(36),
                             p_password VARCHAR(80) )
  RETURNS TINYINT
  READS SQL DATA

BEGIN
  DECLARE count_user   INT;
  DECLARE status       INT;

  CALL config();

  SET status = @OK;
  IF ( @US3_ID IS NULL ) THEN
    SET status = check_user( p_guid, p_password );

  END IF;

  RETURN( status );

END$$

-- Verifies that user identified by email is the same as the
--  logged in user
DROP FUNCTION IF EXISTS verify_user_email$$
CREATE FUNCTION verify_user_email( p_email VARCHAR(63),
                             p_password VARCHAR(80) )
  RETURNS TINYINT
  READS SQL DATA

BEGIN
  DECLARE count_user   INT;
  DECLARE status       INT;

  CALL config();

  SET status = @OK;
  IF ( @US3_ID IS NULL ) THEN
    SET status = check_user_email( p_email, p_password );

  END IF;

  RETURN( status );

END$$

-- Validates the user either by GUID or email and password
-- Returns status, GUID and email
DROP PROCEDURE IF EXISTS validate_user$$
CREATE PROCEDURE validate_user( p_guid     CHAR(36),
                                p_email    VARCHAR(63),
                                p_password VARCHAR(80) )
  READS SQL DATA

BEGIN
  DECLARE status       INT;

  call config();

  -- Let's see if we can validate with the guid
  SET status = check_user( p_guid, p_password );
  IF ( @US3_ID IS NOT NULL ) THEN
    -- We've validated with the guid
    SELECT @OK AS status;
    SELECT @GUID AS GUID, @EMAIL AS email;

  ELSE
    -- Can we validate using the email?
    SET status = check_user_email( p_email, p_password );
    IF ( @US3_ID IS NOT NULL ) THEN
      -- We've validated with the email
      SELECT @OK AS status;
      SELECT @GUID AS GUID, @EMAIL AS email;

    ELSE
      -- No, can't validate either way
      SELECT @US3_LAST_ERRNO AS status;
      SELECT p_guid AS GUID, p_email AS email;

    END IF;

  END IF;

END$$

SOURCE us3_people_procs.sql
SOURCE us3_buffer_procs.sql
SOURCE us3_exp_procs.sql

DELIMITER ;
