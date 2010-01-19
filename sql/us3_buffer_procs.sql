--
-- us3_buffer_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are related to the buffer table
-- Run as us3admin
--

DELIMITER $$

-- Verifies that the user with id is associated with
--  the specified buffer
DROP FUNCTION IF EXISTS verify_buffer_owner$$
CREATE FUNCTION verify_buffer_owner( p_email VARCHAR(63),
                                     p_password VARCHAR(80),
                                     p_bufferID INT )
  RETURNS TINYINT
  READS SQL DATA

BEGIN
  
  DECLARE count_buffer   INT;
  DECLARE status         TINYINT;

  CALL config();

  IF ( verify_user( p_email, p_password ) ) THEN
    SELECT COUNT(*)
    INTO   count_buffer
    FROM   bufferPerson
    WHERE  bufferID = p_bufferID
    AND    personID = @US3_ID;
 
    SET status = @ERROR;
    IF ( count_buffer = 1 ) THEN
      SET status = @OK;

    ELSE
      SET @US3_LAST_ERRNO = @NOT_PERMITTED;
      SET @US3_LAST_ERROR = 'MySQL: you do not have permission to modify this buffer';

    END IF;

  END IF;

  RETURN( status );

END$$

-- Returns the count of buffers associated with id
DROP FUNCTION IF EXISTS count_buffers$$
CREATE FUNCTION count_buffers( p_email VARCHAR(63),
                               p_password VARCHAR(80) )
  RETURNS INT
  READS SQL DATA

BEGIN
  
  DECLARE count_buffers INT;

  CALL config();
  SET count_buffers = 0;

  IF ( verify_user( p_email, p_password ) ) THEN
    SELECT COUNT(*)
    INTO count_buffers
    FROM bufferPerson
    WHERE personID   = @US3_ID;
  END IF;

  RETURN( count_buffers );

END$$

-- INSERTs a new buffer with the specified information
DROP PROCEDURE IF EXISTS new_buffer$$
CREATE PROCEDURE new_buffer ( p_email VARCHAR(63),
                              p_password VARCHAR(80),
                              p_description TEXT,
                              p_density FLOAT,
                              p_viscosity FLOAT )
  MODIFIES SQL DATA

BEGIN
  DECLARE l_bufferID INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;
 
  IF ( verify_user( p_email, p_password ) ) THEN
    INSERT INTO buffer SET
      description = p_description,
      density     = p_density,
      viscosity   = p_viscosity;

    SET @LAST_INSERT_ID = LAST_INSERT_ID();

    INSERT INTO bufferPerson SET
      bufferID    = @LAST_INSERT_ID,
      personID    = @US3_ID;

  END IF;

END$$

-- UPDATEs an existing buffer with the specified information
DROP PROCEDURE IF EXISTS update_buffer$$
CREATE PROCEDURE update_buffer ( p_email VARCHAR(63),
                                 p_password VARCHAR(80),
                                 p_bufferID INT,
                                 p_description TEXT,
                                 p_density FLOAT,
                                 p_viscosity FLOAT )
  MODIFIES SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_buffer_owner( p_email, p_password, p_bufferID ) ) THEN
    UPDATE buffer SET
      description = p_description,
      density     = p_density,
      viscosity   = p_viscosity
    WHERE bufferID = p_bufferID;

  END IF;
      
END$$

-- SELECTs all buffers associated with the id
DROP PROCEDURE IF EXISTS get_buffer_desc$$
CREATE PROCEDURE get_buffer_desc ( p_email VARCHAR(63),
                                   p_password VARCHAR(80) )
  READS SQL DATA

BEGIN

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_email, p_password ) ) THEN
    IF ( count_buffers( p_email, p_password ) < 1 ) THEN
      SET @US3_LAST_ERRNO = @NOROWS;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';
 
    ELSE
      SELECT b.bufferID, description
      FROM buffer b, bufferPerson
      WHERE b.bufferID = bufferPerson.bufferID
      AND bufferPerson.personID = @US3_ID
      ORDER BY b.bufferID DESC;
 
    END IF;

  END IF;

END$$

DROP PROCEDURE IF EXISTS get_buffers$$
CREATE PROCEDURE get_buffers ( p_email VARCHAR(63),
                               p_password VARCHAR(80),
                               p_ID VARCHAR(64) )
  READS SQL DATA

BEGIN
  SELECT "Hello";

END$$

