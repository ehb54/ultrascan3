--
-- Alter HPCAnalysisRequest for requestXMLFile and method columns
--

ALTER TABLE HPCAnalysisResultData
  MODIFY COLUMN HPCAnalysisResultType enum('model','noise','job_stats','mrecs','unknown') NOT NULL default 'unknown';

