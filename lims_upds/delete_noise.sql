--
-- Update HPC table constraints to allow underlying data to be deleted,
--  while HPC tables are not
--

SET FOREIGN_KEY_CHECKS=0;

ALTER TABLE HPCDataset
  DROP FOREIGN KEY fk_HPCDataset_editedDataID;

ALTER TABLE HPCDataset
  DROP INDEX ndx_HPCDataset_editedDataID;

ALTER TABLE HPCDataset
  MODIFY COLUMN editedDataID INT(11) DEFAULT NULL;

ALTER TABLE HPCDataset
  ADD FOREIGN KEY fk_HPCDataset_editedDataID (editedDataID)
  REFERENCES editedData ( editedDataID )
  ON DELETE SET NULL
  ON UPDATE NO ACTION;

ALTER TABLE HPCDataset
  ADD INDEX ndx_HPCDataset_editedDataID (editedDataID ASC);

ALTER TABLE HPCRequestData
  DROP FOREIGN KEY fk_HPCRequestData_noiseID;

ALTER TABLE HPCRequestData
  MODIFY COLUMN noiseID INT(11) DEFAULT NULL;

ALTER TABLE HPCRequestData
  ADD FOREIGN KEY fk_HPCRequestData_noiseID (noiseID)
  REFERENCES noise (noiseID)
  ON DELETE SET NULL
  ON UPDATE NO ACTION;

ALTER TABLE HPCRequestData
  ADD INDEX ndx_HPCRequestData_noiseID ( noiseID ASC );

SET FOREIGN_KEY_CHECKS=1;

DELETE noise
FROM noise LEFT JOIN model
ON ( noise.modelID = model.modelID )
WHERE model.modelID IS NULL;
