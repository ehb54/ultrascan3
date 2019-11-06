--
-- Alter the editedData table to permit NULL 'data' column
--

ALTER TABLE editedData
  MODIFY COLUMN data LONGBLOB NULL DEFAULT NULL;

