--
-- Update the model description to be up to 160 characters
--

ALTER TABLE noise
  ADD COLUMN description VARCHAR(160) NULL DEFAULT NULL
  AFTER noiseType;
