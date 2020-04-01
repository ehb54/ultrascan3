--
-- us3_autoflow_procs.sql
--
-- Script to set up the MySQL stored procedures for the US3 system
--   These are related to various tables pertaining to autoflow
-- Run as root
--

DELIMITER $$

--
-- Autoflow procedures
--


-- Returns the count of autoflow records in db
DROP FUNCTION IF EXISTS count_autoflow_records$$
CREATE FUNCTION count_autoflow_records ( p_personGUID CHAR(36),
                                       p_password   VARCHAR(80) )
                                       
  RETURNS INT
  READS SQL DATA

BEGIN

  DECLARE count_records INT;

  CALL config();
  SET count_records = 0;

       
  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    SELECT    COUNT(*)
    INTO      count_records
    FROM      autoflow;
    
  END IF;

  RETURN( count_records );

END$$


-- Returns the count of editedData profiles for a givel label ---
DROP FUNCTION IF EXISTS count_editprofiles$$
CREATE FUNCTION count_editprofiles ( p_personGUID CHAR(36),
                                     p_password   VARCHAR(80),
				     p_label      VARCHAR(80) )
                                       
  RETURNS INT
  READS SQL DATA

BEGIN

  DECLARE count_records INT;

  CALL config();
  SET count_records = 0;

       
  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    SELECT    COUNT(*)
    INTO      count_records
    FROM      editedData
    WHERE     label = p_label;
    
  END IF;

  RETURN( count_records );

END$$





-- adds autoflow record
DROP PROCEDURE IF EXISTS add_autoflow_record$$
CREATE PROCEDURE add_autoflow_record ( p_personGUID  CHAR(36),
                                     p_password      VARCHAR(80),
                                     p_protname      VARCHAR(80),
                                     p_cellchnum     INT,
                                     p_triplenum     INT,
				     p_duration      INT,
				     p_runname       VARCHAR(80),
				     p_expid         INT,
				     p_optimaname    VARCHAR(80),
				     p_invID         INT,
				     p_label         VARCHAR(80),
				     p_gmprun        VARCHAR(80),
				     p_aprofileguid  VARCHAR(80) )
                                    
  MODIFIES SQL DATA

BEGIN
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';
  SET @LAST_INSERT_ID = 0;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    INSERT INTO autoflow SET
      protname          = p_protname,
      cellChNum         = p_cellchnum,
      tripleNum         = p_triplenum,
      duration          = p_duration,
      runName           = p_runname,
      expID             = p_expid,
      optimaName        = p_optimaname,
      invID             = p_invID,
      label		= p_label,
      created           = NOW(),
      gmpRun            = p_gmprun,
      aprofileGUID      = p_aprofileguid;

    SET @LAST_INSERT_ID = LAST_INSERT_ID();

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$



-- DELETE  autoflow record ( when Optima run aborted manually )
DROP PROCEDURE IF EXISTS delete_autoflow_record$$
CREATE PROCEDURE delete_autoflow_record ( p_personGUID    CHAR(36),
                                     	p_password      VARCHAR(80),
                			p_runID         INT )
  MODIFIES SQL DATA

BEGIN
  DECLARE count_records INT;	
	
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    
    -- Find out if record exists for associated runID 
    SELECT COUNT(*) INTO count_records 
    FROM autoflow 
    WHERE runID = p_runID;

    IF ( count_records = 0 ) THEN
      SET @US3_LAST_ERRNO = @NO_AUTOFLOW_RECORD;
      SET @US3_LAST_ERROR = 'Record cannot be deleted as it does not exist for current experiment run';   

    ELSE
      DELETE FROM autoflow
      WHERE runID = p_runID;
    
    END IF;  

  END IF;
      
  SELECT @US3_LAST_ERRNO AS status;

END$$



-- DELETE  autoflow record by ID 
DROP PROCEDURE IF EXISTS delete_autoflow_record_by_id$$
CREATE PROCEDURE delete_autoflow_record_by_id ( p_personGUID    CHAR(36),
                                     	      p_password      VARCHAR(80),
                			      p_ID            INT )
  MODIFIES SQL DATA

BEGIN
  DECLARE count_records INT;	
	
  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    
    -- Find out if record exists for associated runID 
    SELECT COUNT(*) INTO count_records 
    FROM autoflow 
    WHERE ID = p_ID;

    IF ( count_records = 0 ) THEN
      SET @US3_LAST_ERRNO = @NO_AUTOFLOW_RECORD;
      SET @US3_LAST_ERROR = 'Record cannot be deleted as it does not exist for current experiment run';   

    ELSE
      DELETE FROM autoflow
      WHERE ID = p_ID;
    
    END IF;  

  END IF;
      
  SELECT @US3_LAST_ERRNO AS status;

END$$



-- Returns complete information about autoflow record
DROP PROCEDURE IF EXISTS read_autoflow_record$$
CREATE PROCEDURE read_autoflow_record ( p_personGUID    CHAR(36),
                                       	p_password      VARCHAR(80),
                                       	p_autoflowID  INT )
  READS SQL DATA

BEGIN
  DECLARE count_records INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_records
  FROM       autoflow
  WHERE      ID = p_autoflowID;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( count_records = 0 ) THEN
      SET @US3_LAST_ERRNO = @NO_AUTOFLOW_RECORD;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   protName, cellChNum, tripleNum, duration, runName, expID, 
      	       runID, status, dataPath, optimaName, runStarted, invID, created, 
	       corrRadii, expAborted, label, gmpRun, filename, aprofileGUID  
      FROM     autoflow 
      WHERE    ID = p_autoflowID;

    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$


-- Returns information about autoflow records for listing
DROP PROCEDURE IF EXISTS get_autoflow_desc$$
CREATE PROCEDURE get_autoflow_desc ( p_personGUID    CHAR(36),
                                       	p_password      VARCHAR(80) )
                                     
  READS SQL DATA

BEGIN
  DECLARE count_records INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_records
  FROM       autoflow;


  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( count_records = 0 ) THEN
      SET @US3_LAST_ERRNO = @NO_AUTOFLOW_RECORD;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

      SELECT @US3_LAST_ERRNO AS status;

    ELSE
      SELECT @OK AS status;

      SELECT   ID, protName, cellChNum, tripleNum, duration, runName, expID, 
      	       runID, status, dataPath, optimaName, runStarted, invID, created, gmpRun  
      FROM     autoflow;
     
    END IF;

  ELSE
    SELECT @US3_LAST_ERRNO AS status;

  END IF;

END$$


-- Update autoflow record with Optima's RunID (ONLY once, first time )
DROP PROCEDURE IF EXISTS update_autoflow_runid_starttime$$
CREATE PROCEDURE update_autoflow_runid_starttime ( p_personGUID    CHAR(36),
                                         	 p_password      VARCHAR(80),
                                       	 	 p_expID    	 INT,
					 	 p_runid    	 INT  )
  MODIFIES SQL DATA  

BEGIN
  DECLARE curr_runid INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     runID
  INTO       curr_runid
  FROM       autoflow
  WHERE      expID = p_expID;


  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( curr_runid IS NULL ) THEN
      UPDATE   autoflow
      SET      runID = p_runid, runStarted = NOW()
      WHERE    expID = p_expID;

    END IF;

  END IF;

 -- SELECT @US3_LAST_ERRNO AS status;

END$$


-- Update autoflow record with next stage && curDir at LIVE_UPDATE
DROP PROCEDURE IF EXISTS update_autoflow_at_live_update$$
CREATE PROCEDURE update_autoflow_at_live_update ( p_personGUID    CHAR(36),
                                             	p_password      VARCHAR(80),
                                       	     	p_runID    	 INT,
					  	p_curDir        VARCHAR(80)  )
  MODIFIES SQL DATA  

BEGIN
  DECLARE count_records INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_records
  FROM       autoflow
  WHERE      runID = p_runID;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( count_records = 0 ) THEN
      SET @US3_LAST_ERRNO = @NO_AUTOFLOW_RECORD;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    ELSE
      UPDATE   autoflow
      SET      dataPath = p_curDir, status = 'EDITING'
      WHERE    runID = p_runID;

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$



-- Update autoflow record with corrRadii value at LIVE_UPDATE
DROP PROCEDURE IF EXISTS update_autoflow_at_live_update_radiicorr$$
CREATE PROCEDURE update_autoflow_at_live_update_radiicorr ( p_personGUID    CHAR(36),
                                             	p_password      VARCHAR(80),
                                       	     	p_runID    	 INT )
					 
  MODIFIES SQL DATA  

BEGIN
  DECLARE count_records INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_records
  FROM       autoflow
  WHERE      runID = p_runID;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( count_records = 0 ) THEN
      SET @US3_LAST_ERRNO = @NO_AUTOFLOW_RECORD;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    ELSE
      UPDATE   autoflow
      SET      corrRadii = 'NO'
      WHERE    runID = p_runID;

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$



-- Update autoflow record with expAborted value at LIVE_UPDATE
DROP PROCEDURE IF EXISTS update_autoflow_at_live_update_expaborted$$
CREATE PROCEDURE update_autoflow_at_live_update_expaborted ( p_personGUID    CHAR(36),
                                             	p_password      VARCHAR(80),
                                       	     	p_runID    	 INT )
					 
  MODIFIES SQL DATA  

BEGIN
  DECLARE count_records INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_records
  FROM       autoflow
  WHERE      runID = p_runID;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( count_records = 0 ) THEN
      SET @US3_LAST_ERRNO = @NO_AUTOFLOW_RECORD;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    ELSE
      UPDATE   autoflow
      SET      expAborted = 'YES'
      WHERE    runID = p_runID;

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$



-- Update autoflow record with next stage && filename at EDITING (LIMS IMPORT)
DROP PROCEDURE IF EXISTS update_autoflow_at_lims_import$$
CREATE PROCEDURE update_autoflow_at_lims_import ( p_personGUID    CHAR(36),
                                             	p_password      VARCHAR(80),
                                       	     	p_runID    	INT,
					  	p_filename      VARCHAR(80)  )
  MODIFIES SQL DATA  

BEGIN
  DECLARE count_records INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_records
  FROM       autoflow
  WHERE      runID = p_runID;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( count_records = 0 ) THEN
      SET @US3_LAST_ERRNO = @NO_AUTOFLOW_RECORD;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    ELSE
      UPDATE   autoflow
      SET      filename = p_filename, status = 'EDIT_DATA'
      WHERE    runID = p_runID;

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$




-- Update autoflow record with next stage at EDIT DATA (EDIT DATA to ANALYSIS)
DROP PROCEDURE IF EXISTS update_autoflow_at_edit_data$$
CREATE PROCEDURE update_autoflow_at_edit_data ( p_personGUID    CHAR(36),
                                             	p_password      VARCHAR(80),
                                       	     	p_runID    	INT  )
					 
  MODIFIES SQL DATA  

BEGIN
  DECLARE count_records INT;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_records
  FROM       autoflow
  WHERE      runID = p_runID;

  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( count_records = 0 ) THEN
      SET @US3_LAST_ERRNO = @NO_AUTOFLOW_RECORD;
      SET @US3_LAST_ERROR = 'MySQL: no rows returned';

    ELSE
      UPDATE   autoflow
      SET      status = 'ANALYSIS'
      WHERE    runID = p_runID;

    END IF;

  END IF;

  SELECT @US3_LAST_ERRNO AS status;

END$$



----  get initial elapsed time upon reattachment ----------------------------- 
DROP FUNCTION IF EXISTS read_autoflow_times$$
CREATE FUNCTION read_autoflow_times ( p_personGUID CHAR(36),
                                       p_password   VARCHAR(80), 
				       p_runID      INT )
                                       
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE count_records INT;
  DECLARE l_sec_difference INT; 

  SET l_sec_difference = 0;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_records
  FROM       autoflow
  WHERE      runID = p_runID;

  SELECT TIMESTAMPDIFF( SECOND, runStarted, NOW() ) 
  INTO l_sec_difference 
  FROM autoflow WHERE runID = p_runID AND runStarted IS NOT NULL;


  RETURN( l_sec_difference );

END$$



----  get initial elapsed time upon reattachment ----------------------------- 
DROP FUNCTION IF EXISTS read_autoflow_times_mod$$
CREATE FUNCTION read_autoflow_times_mod ( p_personGUID CHAR(36),
                                       	p_password   VARCHAR(80), 
				       	p_runID      INT )
                                       
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE count_records INT;
  DECLARE l_sec_difference INT;
  DECLARE cur_runStarted TIMESTAMP; 
  	  
  SET l_sec_difference = 0;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_records
  FROM       autoflow
  WHERE      runID = p_runID;

  SELECT     runStarted
  INTO       cur_runStarted
  FROM       autoflow
  WHERE      runID = p_runID;
  
  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( count_records > 0 ) THEN
      IF ( cur_runStarted IS NOT NULL ) THEN 
        	
	SELECT TIMESTAMPDIFF( SECOND, runStarted, NOW() )
	INTO l_sec_difference FROM autoflow WHERE runID = p_runID; 

      END IF;	
    END IF;
  END IF;
    
  RETURN( l_sec_difference );

END$$

----  TEST: TO BE DELETED: get initial elapsed time upon reattachment ----------------------------- 
DROP FUNCTION IF EXISTS read_autoflow_times_mod_test$$
CREATE FUNCTION read_autoflow_times_mod_test ( p_personGUID CHAR(36),
                                       	     p_password   VARCHAR(80) ) 
				       	
                                       
  RETURNS INT
  READS SQL DATA

BEGIN
  DECLARE count_records INT;
  DECLARE l_sec_difference INT;
  DECLARE cur_runStarted TIMESTAMP; 
  	  
  SET l_sec_difference = 0;

  CALL config();
  SET @US3_LAST_ERRNO = @OK;
  SET @US3_LAST_ERROR = '';

  SELECT     COUNT(*)
  INTO       count_records
  FROM       autoflow;
 
  SELECT     created
  INTO       cur_runStarted
  FROM       autoflow LIMIT 1;
   
  IF ( verify_user( p_personGUID, p_password ) = @OK ) THEN
    IF ( count_records > 0 ) THEN
      IF ( cur_runStarted IS NOT NULL ) THEN 
        
	SELECT TIMESTAMPDIFF( SECOND, cur_runStarted, NOW() ) 
 	INTO l_sec_difference; 
  	
      END IF;	
    END IF;
  END IF;
    
  RETURN( l_sec_difference );

END$$

