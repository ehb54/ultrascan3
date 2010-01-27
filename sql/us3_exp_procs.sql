--
-- us3_exp_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are procedures related to the experiment
-- Run as us3admin
--

DELIMITER $$

-- INSERTs information for a new experiment
DROP PROCEDURE IF EXISTS new_experiment$$
CREATE PROCEDURE new_experiment( p_guid     CHAR(36),
                                 p_password VARCHAR(80),
                                 p_exp_type ENUM('velocity', 'equilibrium', 'other'),
                                 p_rotorID INT,
                                 p_date DATE,
                                 p_label VARCHAR(80),
                                 p_comment TEXT)
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_guid, p_password ) ) THEN
    INSERT INTO experiment SET
      type        = p_exp_type,
      rotorID     = p_rotorID,
      dateBegin   = p_date,
      label       = p_label,
      comment     = p_comment;

      SET @LAST_INSERT_ID = LAST_INSERT_ID();

  END IF;
 
END$$

