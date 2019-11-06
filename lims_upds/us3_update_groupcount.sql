--
-- Update the HPCAnalysisResult table with the mgroupcount field
--

ALTER TABLE HPCAnalysisResult
  ADD COLUMN mgroupcount int(11) NOT NULL DEFAULT 1
  AFTER CPUCount;
