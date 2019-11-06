--
-- Alter analysis table for tarfile and status columns
--

ALTER TABLE analysis
  MODIFY COLUMN tarfile longblob NULL;

ALTER TABLE analysis
  MODIFY COLUMN status enum('SUBMITTED','SUBMIT_TIMEOUT','RUNNING','RUN_TIMEOUT','DATA','DATA_TIMEOUT','COMPLETE','CANCELLED','CANCELED','FAILED','FAILED_DATA','ERROR','FINISHED','DONE') DEFAULT NULL;

