--
-- Add a filename field to the gel images. 
--

ALTER TABLE image
  ADD COLUMN filename VARCHAR(255) NOT NULL DEFAULT ''
  AFTER gelPicture;
