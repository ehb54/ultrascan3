--
-- Update the model description to be up to 160 characters
--

ALTER TABLE model
  MODIFY COLUMN description VARCHAR(160) NULL DEFAULT NULL;
