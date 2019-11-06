SELECT email, submitTime
FROM HPCAnalysisRequest
WHERE STR_TO_DATE(submitTime,'%Y-%m-%d') > STR_TO_DATE('2018-09','%Y-%m-%d');
ORDER BY email, submitTime DESC
