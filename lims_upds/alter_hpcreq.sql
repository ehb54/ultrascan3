--
-- Alter HPCAnalysisRequest for requestXMLFile and method columns
--

ALTER TABLE HPCAnalysisRequest
  ADD COLUMN analType text NULL;

