--
-- add_imageBuffer_table.sql
--
-- creates the table
--

CREATE  TABLE IF NOT EXISTS imageBuffer (
  imageID int(11) NOT NULL ,
  bufferID int(11) NOT NULL ,
  PRIMARY KEY (imageID) ,
  INDEX ndx_imageBuffer_imageID   (imageID ASC) ,
  INDEX ndx_imageBuffer_bufferID (bufferID ASC) ,
  CONSTRAINT fk_imageBuffer_imageID
    FOREIGN KEY (imageID )
    REFERENCES image (imageID )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT fk_imageBuffer_bufferID
    FOREIGN KEY (bufferID )
    REFERENCES buffer (bufferID )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


