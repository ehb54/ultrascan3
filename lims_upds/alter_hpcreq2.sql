--
-- Alter HPCAnalysisRequest for analType columns
--

UPDATE HPCAnalysisRequest
  SET analType = '2DSA'
  WHERE analType IS NULL
  AND method = '2DSA';

UPDATE HPCAnalysisRequest
  SET analType = '2DSA-CG'
  WHERE analType IS NULL
  AND method = '2DSA_CG';

UPDATE HPCAnalysisRequest
  SET analType = 'GA'
  WHERE analType IS NULL
  AND method = 'GA';

UPDATE HPCAnalysisRequest
  SET analType = 'DMGA'
  WHERE analType IS NULL
  AND method = 'DMGA';

UPDATE HPCAnalysisRequest
  SET analType = 'PCSA'
  WHERE analType IS NULL
  AND method = 'PCSA';

