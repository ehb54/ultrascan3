-- MySQL dump 10.10
--
-- Host: localhost    Database: cauma
-- ------------------------------------------------------
-- Server version	5.0.22-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `EquilCellResult`
--

DROP TABLE IF EXISTS `EquilCellResult`;
CREATE TABLE `EquilCellResult` (
  `CellRstID` int(11) NOT NULL default '0',
  `EquilRstID` int(11) default NULL,
  `EED1ID` int(11) default NULL,
  `EED2ID` int(11) default NULL,
  `EED3ID` int(11) default NULL,
  `EED4ID` int(11) default NULL,
  `Description` varchar(250) default NULL,
  PRIMARY KEY  (`CellRstID`),
  KEY `eed1` (`EED1ID`),
  KEY `eed2` (`EED2ID`),
  KEY `eed3` (`EED3ID`),
  KEY `eed4` (`EED4ID`),
  CONSTRAINT `0_1223` FOREIGN KEY (`EED1ID`) REFERENCES `EquilExpData` (`EEDID`) ON DELETE SET NULL,
  CONSTRAINT `0_1224` FOREIGN KEY (`EED2ID`) REFERENCES `EquilExpData` (`EEDID`) ON DELETE SET NULL,
  CONSTRAINT `0_1225` FOREIGN KEY (`EED3ID`) REFERENCES `EquilExpData` (`EEDID`) ON DELETE SET NULL,
  CONSTRAINT `0_1226` FOREIGN KEY (`EED4ID`) REFERENCES `EquilExpData` (`EEDID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `EquilExpData`
--

DROP TABLE IF EXISTS `EquilExpData`;
CREATE TABLE `EquilExpData` (
  `EEDID` int(11) NOT NULL default '0',
  `EEDtableID_WL1` int(11) default NULL,
  `EEDtableID_WL2` int(11) default NULL,
  `EEDtableID_WL3` int(11) default NULL,
  PRIMARY KEY  (`EEDID`),
  KEY `eed_wl1` (`EEDtableID_WL1`),
  KEY `eed_wl2` (`EEDtableID_WL2`),
  KEY `eed_wl3` (`EEDtableID_WL3`),
  CONSTRAINT `0_1228` FOREIGN KEY (`EEDtableID_WL1`) REFERENCES `EquilExpDataTable` (`EEDtableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1229` FOREIGN KEY (`EEDtableID_WL2`) REFERENCES `EquilExpDataTable` (`EEDtableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1230` FOREIGN KEY (`EEDtableID_WL3`) REFERENCES `EquilExpDataTable` (`EEDtableID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `EquilExpDataTable`
--

DROP TABLE IF EXISTS `EquilExpDataTable`;
CREATE TABLE `EquilExpDataTable` (
  `EEDtableID` int(11) NOT NULL default '0',
  `PepFile` blob,
  `BufFile` blob,
  `EDPlot` longblob,
  `F_EDPlot` longblob,
  PRIMARY KEY  (`EEDtableID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `EquilProjectData`
--

DROP TABLE IF EXISTS `EquilProjectData`;
CREATE TABLE `EquilProjectData` (
  `tableID` int(11) NOT NULL default '0',
  `HeadFile` text,
  `Report_Tar` longblob,
  `Result_Tar` longblob,
  PRIMARY KEY  (`tableID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `EquilProjectResult`
--

DROP TABLE IF EXISTS `EquilProjectResult`;
CREATE TABLE `EquilProjectResult` (
  `EquilProjectID` int(11) NOT NULL default '0',
  `ProjectName` varchar(250) default NULL,
  `InvestigatorID` int(11) default NULL,
  `RunRequestID` int(11) default NULL,
  `DataID` int(11) default NULL,
  PRIMARY KEY  (`EquilProjectID`),
  KEY `invid` (`InvestigatorID`),
  KEY `dataid` (`DataID`),
  CONSTRAINT `0_1234` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE,
  CONSTRAINT `0_1235` FOREIGN KEY (`DataID`) REFERENCES `EquilProjectData` (`tableID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `EquilResult`
--

DROP TABLE IF EXISTS `EquilResult`;
CREATE TABLE `EquilResult` (
  `EquilRstID` int(11) NOT NULL default '0',
  `InvID` int(11) default NULL,
  `Date` varchar(15) default NULL,
  `Description` varchar(250) default NULL,
  `Cell1ID` int(11) default NULL,
  `Cell2ID` int(11) default NULL,
  `Cell3ID` int(11) default NULL,
  `Cell4ID` int(11) default NULL,
  `Cell5ID` int(11) default NULL,
  `Cell6ID` int(11) default NULL,
  `Cell7ID` int(11) default NULL,
  `Cell8ID` int(11) default NULL,
  `RunRequestID` int(11) default NULL,
  PRIMARY KEY  (`EquilRstID`),
  KEY `cell1id` (`Cell1ID`),
  KEY `cell2id` (`Cell2ID`),
  KEY `cell3id` (`Cell3ID`),
  KEY `cell4id` (`Cell4ID`),
  KEY `cell5id` (`Cell5ID`),
  KEY `cell6id` (`Cell6ID`),
  KEY `cell7id` (`Cell7ID`),
  KEY `cell8id` (`Cell8ID`),
  KEY `InvId` (`InvID`),
  CONSTRAINT `0_1237` FOREIGN KEY (`Cell1ID`) REFERENCES `EquilCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1238` FOREIGN KEY (`Cell2ID`) REFERENCES `EquilCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1239` FOREIGN KEY (`Cell3ID`) REFERENCES `EquilCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1240` FOREIGN KEY (`Cell4ID`) REFERENCES `EquilCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1241` FOREIGN KEY (`Cell5ID`) REFERENCES `EquilCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1242` FOREIGN KEY (`Cell6ID`) REFERENCES `EquilCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1243` FOREIGN KEY (`Cell7ID`) REFERENCES `EquilCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1244` FOREIGN KEY (`Cell8ID`) REFERENCES `EquilCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1245` FOREIGN KEY (`InvID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `EquilResultFile`
--

DROP TABLE IF EXISTS `EquilResultFile`;
CREATE TABLE `EquilResultFile` (
  `EquilRstID` int(11) NOT NULL default '0',
  `RunInfo1` longblob,
  `RunInfo2` longblob,
  `RunInfo3` longblob,
  `RunInfo4` longblob,
  `RunInfo5` longblob,
  `RunInfo6` longblob,
  `DataFile` longblob,
  PRIMARY KEY  (`EquilRstID`),
  KEY `equilrstid` (`EquilRstID`),
  CONSTRAINT `0_1247` FOREIGN KEY (`EquilRstID`) REFERENCES `EquilResult` (`EquilRstID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `MonteCarloData`
--

DROP TABLE IF EXISTS `MonteCarloData`;
CREATE TABLE `MonteCarloData` (
  `montecarloID` int(11) NOT NULL default '0',
  `report` blob,
  `parameterFile` blob,
  `data` longblob,
  PRIMARY KEY  (`montecarloID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `MonteCarloResult`
--

DROP TABLE IF EXISTS `MonteCarloResult`;
CREATE TABLE `MonteCarloResult` (
  `montecarloID` int(11) NOT NULL default '0',
  `InvestigatorID` int(11) default NULL,
  `RunRequestID` int(11) default NULL,
  `parameter_number` smallint(6) default NULL,
  `projectName` varchar(255) default NULL,
  PRIMARY KEY  (`montecarloID`),
  KEY `invid` (`InvestigatorID`),
  CONSTRAINT `0_1250` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocCS`
--

DROP TABLE IF EXISTS `VelocCS`;
CREATE TABLE `VelocCS` (
  `CSID` int(11) NOT NULL default '0',
  `CStableID_WL1` int(11) default NULL,
  `CStableID_WL2` int(11) default NULL,
  `CStableID_WL3` int(11) default NULL,
  PRIMARY KEY  (`CSID`),
  KEY `cs_wl1` (`CStableID_WL1`),
  KEY `cs_wl2` (`CStableID_WL2`),
  KEY `cs_wl3` (`CStableID_WL3`),
  CONSTRAINT `0_1252` FOREIGN KEY (`CStableID_WL1`) REFERENCES `VelocCSTable` (`CStableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1253` FOREIGN KEY (`CStableID_WL2`) REFERENCES `VelocCSTable` (`CStableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1254` FOREIGN KEY (`CStableID_WL3`) REFERENCES `VelocCSTable` (`CStableID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocCSTable`
--

DROP TABLE IF EXISTS `VelocCSTable`;
CREATE TABLE `VelocCSTable` (
  `CStableID` int(11) NOT NULL default '0',
  `Report` blob,
  `DistrPlot` longblob,
  `MW_DistrPlot` longblob,
  `ResidPlot` longblob,
  `ResidPixmap` longblob,
  `F_DistrPlot` longblob,
  `EditedPlot` longblob,
  PRIMARY KEY  (`CStableID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocCellResult`
--

DROP TABLE IF EXISTS `VelocCellResult`;
CREATE TABLE `VelocCellResult` (
  `CellRstID` int(11) NOT NULL default '0',
  `VelocRstID` int(11) default NULL,
  `EDID` int(11) default NULL,
  `VHWID` int(11) default NULL,
  `FEID` int(11) default NULL,
  `SMID` int(11) default NULL,
  `TDID` int(11) default NULL,
  `RDID` int(11) default NULL,
  `Description` varchar(250) default NULL,
  `CSID` int(11) default NULL,
  PRIMARY KEY  (`CellRstID`),
  KEY `ed` (`EDID`),
  KEY `vhw` (`VHWID`),
  KEY `fe` (`FEID`),
  KEY `sm` (`SMID`),
  KEY `td` (`TDID`),
  KEY `rd` (`RDID`),
  KEY `cs` (`CSID`),
  CONSTRAINT `0_1257` FOREIGN KEY (`CSID`) REFERENCES `VelocCS` (`CSID`) ON DELETE SET NULL,
  CONSTRAINT `0_1258` FOREIGN KEY (`EDID`) REFERENCES `VelocExpData` (`EDID`) ON DELETE SET NULL,
  CONSTRAINT `0_1259` FOREIGN KEY (`VHWID`) REFERENCES `VelocVHW` (`VHWID`) ON DELETE SET NULL,
  CONSTRAINT `0_1260` FOREIGN KEY (`FEID`) REFERENCES `VelocFiniteElement` (`FEID`) ON DELETE SET NULL,
  CONSTRAINT `0_1261` FOREIGN KEY (`SMID`) REFERENCES `VelocSecondMoment` (`SMID`) ON DELETE SET NULL,
  CONSTRAINT `0_1262` FOREIGN KEY (`TDID`) REFERENCES `VelocTimeDerivative` (`TDID`) ON DELETE SET NULL,
  CONSTRAINT `0_1263` FOREIGN KEY (`RDID`) REFERENCES `VelocRadialDerivative` (`RDID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocExpData`
--

DROP TABLE IF EXISTS `VelocExpData`;
CREATE TABLE `VelocExpData` (
  `EDID` int(11) NOT NULL default '0',
  `EDtableID_WL1` int(11) default NULL,
  `EDtableID_WL2` int(11) default NULL,
  `EDtableID_WL3` int(11) default NULL,
  PRIMARY KEY  (`EDID`),
  KEY `ed_wl1` (`EDtableID_WL1`),
  KEY `ed_wl2` (`EDtableID_WL2`),
  KEY `ed_wl3` (`EDtableID_WL3`),
  CONSTRAINT `0_1265` FOREIGN KEY (`EDtableID_WL1`) REFERENCES `VelocExpDataTable` (`EDtableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1266` FOREIGN KEY (`EDtableID_WL2`) REFERENCES `VelocExpDataTable` (`EDtableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1267` FOREIGN KEY (`EDtableID_WL3`) REFERENCES `VelocExpDataTable` (`EDtableID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocExpDataTable`
--

DROP TABLE IF EXISTS `VelocExpDataTable`;
CREATE TABLE `VelocExpDataTable` (
  `EDtableID` int(11) NOT NULL default '0',
  `PepFile` blob,
  `BufFile` blob,
  `EDPlot` longblob,
  `F_EDPlot` longblob,
  PRIMARY KEY  (`EDtableID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocFiniteElement`
--

DROP TABLE IF EXISTS `VelocFiniteElement`;
CREATE TABLE `VelocFiniteElement` (
  `FEID` int(11) NOT NULL default '0',
  `FEtableID_WL1` int(11) default NULL,
  `FEtableID_WL2` int(11) default NULL,
  `FEtableID_WL3` int(11) default NULL,
  PRIMARY KEY  (`FEID`),
  KEY `fe_wl1` (`FEtableID_WL1`),
  KEY `fe_wl2` (`FEtableID_WL2`),
  KEY `fe_wl3` (`FEtableID_WL3`),
  CONSTRAINT `0_1270` FOREIGN KEY (`FEtableID_WL1`) REFERENCES `VelocFiniteElementTable` (`FEtableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1271` FOREIGN KEY (`FEtableID_WL2`) REFERENCES `VelocFiniteElementTable` (`FEtableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1272` FOREIGN KEY (`FEtableID_WL3`) REFERENCES `VelocFiniteElementTable` (`FEtableID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocFiniteElementTable`
--

DROP TABLE IF EXISTS `VelocFiniteElementTable`;
CREATE TABLE `VelocFiniteElementTable` (
  `FEtableID` int(11) NOT NULL default '0',
  `Report` blob,
  `Residuals` longblob,
  `Overlays` longblob,
  `F_Residuals` longblob,
  `F_Raw` longblob,
  `F_Solution` longblob,
  PRIMARY KEY  (`FEtableID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocRadialDerivative`
--

DROP TABLE IF EXISTS `VelocRadialDerivative`;
CREATE TABLE `VelocRadialDerivative` (
  `RDID` int(11) NOT NULL default '0',
  `RDtableID_WL1` int(11) default NULL,
  `RDtableID_WL2` int(11) default NULL,
  `RDtableID_WL3` int(11) default NULL,
  PRIMARY KEY  (`RDID`),
  KEY `rd_wl1` (`RDtableID_WL1`),
  KEY `rd_wl2` (`RDtableID_WL2`),
  KEY `rd_wl3` (`RDtableID_WL3`),
  CONSTRAINT `0_1275` FOREIGN KEY (`RDtableID_WL1`) REFERENCES `VelocRadialDerivativeTable` (`RDtableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1276` FOREIGN KEY (`RDtableID_WL2`) REFERENCES `VelocRadialDerivativeTable` (`RDtableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1277` FOREIGN KEY (`RDtableID_WL3`) REFERENCES `VelocRadialDerivativeTable` (`RDtableID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocRadialDerivativeTable`
--

DROP TABLE IF EXISTS `VelocRadialDerivativeTable`;
CREATE TABLE `VelocRadialDerivativeTable` (
  `RDtableID` int(11) NOT NULL default '0',
  `Report` blob,
  `RDPlot` longblob,
  `F_ScanPlot` longblob,
  `F_SvaluePlot` longblob,
  `AvgPlot` longblob,
  `F_AvgPlot` longblob,
  `EditedPlot` longblob,
  PRIMARY KEY  (`RDtableID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocResult`
--

DROP TABLE IF EXISTS `VelocResult`;
CREATE TABLE `VelocResult` (
  `VelocRstID` int(11) NOT NULL default '0',
  `InvID` int(11) default NULL,
  `Date` varchar(15) default NULL,
  `Description` varchar(250) default NULL,
  `Cell1ID` int(11) default NULL,
  `Cell2ID` int(11) default NULL,
  `Cell3ID` int(11) default NULL,
  `Cell4ID` int(11) default NULL,
  `Cell5ID` int(11) default NULL,
  `Cell6ID` int(11) default NULL,
  `Cell7ID` int(11) default NULL,
  `Cell8ID` int(11) default NULL,
  `RunRequestID` int(11) default NULL,
  PRIMARY KEY  (`VelocRstID`),
  KEY `cell1_id` (`Cell1ID`),
  KEY `cell2_id` (`Cell2ID`),
  KEY `cell3_id` (`Cell3ID`),
  KEY `cell4_id` (`Cell4ID`),
  KEY `cell5_id` (`Cell5ID`),
  KEY `cell6_id` (`Cell6ID`),
  KEY `cell7_id` (`Cell7ID`),
  KEY `cell8_id` (`Cell8ID`),
  KEY `Invid` (`InvID`),
  CONSTRAINT `0_1280` FOREIGN KEY (`Cell1ID`) REFERENCES `VelocCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1281` FOREIGN KEY (`Cell2ID`) REFERENCES `VelocCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1282` FOREIGN KEY (`Cell3ID`) REFERENCES `VelocCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1283` FOREIGN KEY (`Cell4ID`) REFERENCES `VelocCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1284` FOREIGN KEY (`Cell5ID`) REFERENCES `VelocCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1285` FOREIGN KEY (`Cell6ID`) REFERENCES `VelocCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1286` FOREIGN KEY (`Cell7ID`) REFERENCES `VelocCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1287` FOREIGN KEY (`Cell8ID`) REFERENCES `VelocCellResult` (`CellRstID`) ON DELETE SET NULL,
  CONSTRAINT `0_1288` FOREIGN KEY (`InvID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocResultFile`
--

DROP TABLE IF EXISTS `VelocResultFile`;
CREATE TABLE `VelocResultFile` (
  `VelocRstID` int(11) NOT NULL default '0',
  `RunInfo1` blob,
  `RunInfo2` blob,
  `RunInfo3` blob,
  `VHW1` blob,
  `VHW2` blob,
  `VHW3` blob,
  `DataFile` longblob,
  `MW1` blob,
  `MW2` blob,
  `MW3` blob,
  PRIMARY KEY  (`VelocRstID`),
  KEY `velocid` (`VelocRstID`),
  CONSTRAINT `0_1290` FOREIGN KEY (`VelocRstID`) REFERENCES `VelocResult` (`VelocRstID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocSecondMoment`
--

DROP TABLE IF EXISTS `VelocSecondMoment`;
CREATE TABLE `VelocSecondMoment` (
  `SMID` int(11) NOT NULL default '0',
  `SMtableID_WL1` int(11) default NULL,
  `SMtableID_WL2` int(11) default NULL,
  `SMtableID_WL3` int(11) default NULL,
  PRIMARY KEY  (`SMID`),
  KEY `sm_wl1` (`SMtableID_WL1`),
  KEY `sm_wl2` (`SMtableID_WL2`),
  KEY `sm_wl3` (`SMtableID_WL3`),
  CONSTRAINT `0_1292` FOREIGN KEY (`SMtableID_WL1`) REFERENCES `VelocSecondMomentTable` (`SMtableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1293` FOREIGN KEY (`SMtableID_WL2`) REFERENCES `VelocSecondMomentTable` (`SMtableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1294` FOREIGN KEY (`SMtableID_WL3`) REFERENCES `VelocSecondMomentTable` (`SMtableID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocSecondMomentTable`
--

DROP TABLE IF EXISTS `VelocSecondMomentTable`;
CREATE TABLE `VelocSecondMomentTable` (
  `SMtableID` int(11) NOT NULL default '0',
  `Report` blob,
  `SMPlot` longblob,
  `F_SMPlot` longblob,
  `EditedPlot` longblob,
  PRIMARY KEY  (`SMtableID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocTimeDerivative`
--

DROP TABLE IF EXISTS `VelocTimeDerivative`;
CREATE TABLE `VelocTimeDerivative` (
  `TDID` int(11) NOT NULL default '0',
  `TDtableID_WL1` int(11) default NULL,
  `TDtableID_WL2` int(11) default NULL,
  `TDtableID_WL3` int(11) default NULL,
  PRIMARY KEY  (`TDID`),
  KEY `td_wl1` (`TDtableID_WL1`),
  KEY `td_wl2` (`TDtableID_WL2`),
  KEY `td_wl3` (`TDtableID_WL3`),
  CONSTRAINT `0_1297` FOREIGN KEY (`TDtableID_WL1`) REFERENCES `VelocTimeDerivativeTable` (`TDtableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1298` FOREIGN KEY (`TDtableID_WL2`) REFERENCES `VelocTimeDerivativeTable` (`TDtableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1299` FOREIGN KEY (`TDtableID_WL3`) REFERENCES `VelocTimeDerivativeTable` (`TDtableID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocTimeDerivativeTable`
--

DROP TABLE IF EXISTS `VelocTimeDerivativeTable`;
CREATE TABLE `VelocTimeDerivativeTable` (
  `TDtableID` int(11) NOT NULL default '0',
  `Report` blob,
  `TDPlot` longblob,
  `F_ScanPlot` longblob,
  `F_SvaluePlot` longblob,
  `AvgPlot` longblob,
  `F_AvgPlot` longblob,
  `EditedPlot` longblob,
  PRIMARY KEY  (`TDtableID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocVHW`
--

DROP TABLE IF EXISTS `VelocVHW`;
CREATE TABLE `VelocVHW` (
  `VHWID` int(11) NOT NULL default '0',
  `VHWtableID_WL1` int(11) default NULL,
  `VHWtableID_WL2` int(11) default NULL,
  `VHWtableID_WL3` int(11) default NULL,
  PRIMARY KEY  (`VHWID`),
  KEY `vhw_wl1` (`VHWtableID_WL1`),
  KEY `vhw_wl2` (`VHWtableID_WL2`),
  KEY `vhw_wl3` (`VHWtableID_WL3`),
  CONSTRAINT `0_1302` FOREIGN KEY (`VHWtableID_WL1`) REFERENCES `VelocVHWTable` (`VHWtableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1303` FOREIGN KEY (`VHWtableID_WL2`) REFERENCES `VelocVHWTable` (`VHWtableID`) ON DELETE SET NULL,
  CONSTRAINT `0_1304` FOREIGN KEY (`VHWtableID_WL3`) REFERENCES `VelocVHWTable` (`VHWtableID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `VelocVHWTable`
--

DROP TABLE IF EXISTS `VelocVHWTable`;
CREATE TABLE `VelocVHWTable` (
  `VHWtableID` int(11) NOT NULL default '0',
  `Report` blob,
  `ExtraPlot` longblob,
  `F_ExtraPlot` longblob,
  `DistrPlot` longblob,
  `F_DistrPlot` longblob,
  `EditedPlot` longblob,
  `HisPlot` longblob,
  `F_HisPlot` longblob,
  PRIMARY KEY  (`VHWtableID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tbl2DSA_MW_Settings`
--

DROP TABLE IF EXISTS `tbl2DSA_MW_Settings`;
CREATE TABLE `tbl2DSA_MW_Settings` (
  `2DSA_MW_Settings_ID` int(11) NOT NULL auto_increment,
  `mw_min` double NOT NULL default '100',
  `mw_max` double NOT NULL default '1000',
  `grid_resolution` int(11) NOT NULL default '10',
  `oligomer` int(11) NOT NULL default '4',
  `ff0_min` double NOT NULL default '1',
  `ff0_max` double NOT NULL default '4',
  `ff0_resolution` int(11) NOT NULL default '10',
  `uniform_grid` int(11) NOT NULL default '6',
  `montecarlo_value` int(11) NOT NULL default '0',
  `tinoise_option` tinyint(1) NOT NULL default '0',
  `regularization` int(11) NOT NULL default '0',
  `meniscus_option` tinyint(1) NOT NULL default '0',
  `meniscus_value` double NOT NULL default '0.01',
  `meniscus_points` int(11) NOT NULL default '3',
  `iterations_option` tinyint(1) NOT NULL default '0',
  `iterations_value` int(11) NOT NULL default '3',
  `rinoise_option` tinyint(1) NOT NULL default '0',
  `simpoints` int(11) default NULL,
  `band_volume` double default NULL,
  `radial_grid` tinyint(4) default NULL,
  `time_grid` tinyint(4) default NULL,
  `HPCAnalysis_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`2DSA_MW_Settings_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tbl2DSA_Settings`
--

DROP TABLE IF EXISTS `tbl2DSA_Settings`;
CREATE TABLE `tbl2DSA_Settings` (
  `2DSA_Settings_ID` int(11) NOT NULL auto_increment,
  `s_min` double NOT NULL default '1',
  `s_max` double NOT NULL default '10',
  `s_resolution` double NOT NULL default '10',
  `ff0_min` double NOT NULL default '1',
  `ff0_max` double NOT NULL default '4',
  `ff0_resolution` double NOT NULL default '10',
  `uniform_grid` int(11) NOT NULL default '6',
  `montecarlo_value` int(11) NOT NULL default '0',
  `tinoise_option` tinyint(1) NOT NULL default '0',
  `regularization` int(11) NOT NULL default '0',
  `meniscus_option` tinyint(1) NOT NULL default '0',
  `meniscus_value` double NOT NULL default '0.01',
  `meniscus_points` double NOT NULL default '3',
  `iterations_option` tinyint(1) NOT NULL default '0',
  `iterations_value` int(11) NOT NULL default '3',
  `rinoise_option` tinyint(1) NOT NULL default '0',
  `simpoints` int(11) default NULL,
  `band_volume` double default NULL,
  `radial_grid` tinyint(4) default NULL,
  `time_grid` tinyint(4) default NULL,
  `HPCAnalysis_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`2DSA_Settings_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblAuth`
--

DROP TABLE IF EXISTS `tblAuth`;
CREATE TABLE `tblAuth` (
  `Password` varchar(80) NOT NULL default '',
  `InvestigatorID` int(11) NOT NULL default '0',
  `Classification` varchar(25) default NULL,
  `Status` varchar(20) default 'Zero Balance',
  `Balance` float(8,2) unsigned zerofill default '00000.00',
  `activated` tinyint(4) NOT NULL default '0',
  `Signup` datetime NOT NULL default '0000-00-00 00:00:00',
  `LastLogin` datetime NOT NULL default '0000-00-00 00:00:00',
  `Userlevel` tinyint(4) NOT NULL default '0',
  `ClusterAuth` varchar(255) NOT NULL default 'bcf:alamo:laredo:lonestar',
  `account_activity_changed` date default NULL,
  PRIMARY KEY  (`InvestigatorID`),
  KEY `Invid` (`InvestigatorID`),
  CONSTRAINT `0_1307` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE,
  CONSTRAINT `0_1308` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`),
  CONSTRAINT `0_1309` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblBuffer`
--

DROP TABLE IF EXISTS `tblBuffer`;
CREATE TABLE `tblBuffer` (
  `BuffID` int(11) NOT NULL auto_increment,
  `InvestigatorID` int(11) default NULL,
  `BufferDescription` varchar(80) NOT NULL default '',
  `BufferData` text NOT NULL,
  `RefractiveIndex20` float(6,4) unsigned zerofill default '0.0000',
  `Density20` float(6,4) unsigned zerofill default '0.0000',
  `Viscosity20` float(6,4) unsigned zerofill default '0.0000',
  PRIMARY KEY  (`BuffID`),
  KEY `Invid` (`InvestigatorID`),
  CONSTRAINT `0_1311` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE,
  CONSTRAINT `0_1312` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`),
  CONSTRAINT `0_1313` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblCell`
--

DROP TABLE IF EXISTS `tblCell`;
CREATE TABLE `tblCell` (
  `CellID` int(11) NOT NULL auto_increment,
  `CenterpieceID` int(11) default NULL,
  `Position` tinyint(1) default NULL,
  `InvestigatorID` int(11) default NULL,
  `Description` varchar(255) default NULL,
  `Wavelength_1` int(10) default NULL,
  `Wavelength_2` int(10) default NULL,
  `Wavelength_3` int(10) default NULL,
  `Wl_Scans_1` int(10) default NULL,
  `Wl_Scans_2` int(10) default NULL,
  `Wl_Scans_3` int(10) default NULL,
  `DNAID_1` int(11) default NULL,
  `DNA2ID_1` int(11) default NULL,
  `DNA3ID_1` int(11) default NULL,
  `BufferID_1` int(11) default NULL,
  `PeptideID_1` int(11) default NULL,
  `Peptide2ID_1` int(11) default NULL,
  `Peptide3ID_1` int(11) default NULL,
  `DNAID_2` int(11) default NULL,
  `DNA2ID_2` int(11) default NULL,
  `DNA3ID_2` int(11) default NULL,
  `BufferID_2` int(11) default NULL,
  `PeptideID_2` int(11) default NULL,
  `Peptide2ID_2` int(11) default NULL,
  `Peptide3ID_2` int(11) default NULL,
  `DNAID_3` int(11) default NULL,
  `DNA2ID_3` int(11) default NULL,
  `DNA3ID_3` int(11) default NULL,
  `BufferID_3` int(11) default NULL,
  `PeptideID_3` int(11) default NULL,
  `Peptide2ID_3` int(11) default NULL,
  `Peptide3ID_3` int(11) default NULL,
  `DNAID_4` int(11) default NULL,
  `DNA2ID_4` int(11) default NULL,
  `DNA3ID_4` int(11) default NULL,
  `BufferID_4` int(11) default NULL,
  `PeptideID_4` int(11) default NULL,
  `Peptide2ID_4` int(11) default NULL,
  `Peptide3ID_4` int(11) default NULL,
  `Num_Channel` tinyint(1) default NULL,
  `ExperimentID` int(11) default NULL,
  PRIMARY KEY  (`CellID`),
  KEY `bufid1` (`BufferID_1`),
  KEY `bufid2` (`BufferID_2`),
  KEY `bufid3` (`BufferID_3`),
  KEY `bufid4` (`BufferID_4`),
  KEY `pepid1` (`PeptideID_1`),
  KEY `pepid21` (`Peptide2ID_1`),
  KEY `pepid31` (`Peptide3ID_1`),
  KEY `pepid2` (`PeptideID_2`),
  KEY `pepid22` (`Peptide2ID_2`),
  KEY `pepid32` (`Peptide3ID_2`),
  KEY `pepid3` (`PeptideID_3`),
  KEY `pepid23` (`Peptide2ID_3`),
  KEY `pepid33` (`Peptide3ID_3`),
  KEY `pepid4` (`PeptideID_4`),
  KEY `pepid24` (`Peptide2ID_4`),
  KEY `pepid34` (`Peptide3ID_4`),
  KEY `dnaid1` (`DNAID_1`),
  KEY `dnaid21` (`DNA2ID_1`),
  KEY `dnaid31` (`DNA3ID_1`),
  KEY `dnaid2` (`DNAID_2`),
  KEY `dnaid22` (`DNA2ID_2`),
  KEY `dnaid32` (`DNA3ID_2`),
  KEY `dnaid3` (`DNAID_3`),
  KEY `dnaid23` (`DNA2ID_3`),
  KEY `dnaid33` (`DNA3ID_3`),
  KEY `dnaid4` (`DNAID_4`),
  KEY `dnaid24` (`DNA2ID_4`),
  KEY `dnaid34` (`DNA3ID_4`),
  KEY `Invid` (`InvestigatorID`),
  CONSTRAINT `0_1315` FOREIGN KEY (`BufferID_1`) REFERENCES `tblBuffer` (`BuffID`) ON DELETE SET NULL,
  CONSTRAINT `0_1316` FOREIGN KEY (`BufferID_2`) REFERENCES `tblBuffer` (`BuffID`) ON DELETE SET NULL,
  CONSTRAINT `0_1317` FOREIGN KEY (`BufferID_3`) REFERENCES `tblBuffer` (`BuffID`) ON DELETE SET NULL,
  CONSTRAINT `0_1318` FOREIGN KEY (`BufferID_4`) REFERENCES `tblBuffer` (`BuffID`) ON DELETE SET NULL,
  CONSTRAINT `0_1319` FOREIGN KEY (`PeptideID_1`) REFERENCES `tblPeptide` (`PepID`) ON DELETE SET NULL,
  CONSTRAINT `0_1320` FOREIGN KEY (`Peptide2ID_1`) REFERENCES `tblPeptide` (`PepID`) ON DELETE SET NULL,
  CONSTRAINT `0_1321` FOREIGN KEY (`Peptide3ID_1`) REFERENCES `tblPeptide` (`PepID`) ON DELETE SET NULL,
  CONSTRAINT `0_1322` FOREIGN KEY (`PeptideID_2`) REFERENCES `tblPeptide` (`PepID`) ON DELETE SET NULL,
  CONSTRAINT `0_1323` FOREIGN KEY (`Peptide2ID_2`) REFERENCES `tblPeptide` (`PepID`) ON DELETE SET NULL,
  CONSTRAINT `0_1324` FOREIGN KEY (`Peptide3ID_2`) REFERENCES `tblPeptide` (`PepID`) ON DELETE SET NULL,
  CONSTRAINT `0_1325` FOREIGN KEY (`PeptideID_3`) REFERENCES `tblPeptide` (`PepID`) ON DELETE SET NULL,
  CONSTRAINT `0_1326` FOREIGN KEY (`Peptide2ID_3`) REFERENCES `tblPeptide` (`PepID`) ON DELETE SET NULL,
  CONSTRAINT `0_1327` FOREIGN KEY (`Peptide3ID_3`) REFERENCES `tblPeptide` (`PepID`) ON DELETE SET NULL,
  CONSTRAINT `0_1328` FOREIGN KEY (`PeptideID_4`) REFERENCES `tblPeptide` (`PepID`) ON DELETE SET NULL,
  CONSTRAINT `0_1329` FOREIGN KEY (`Peptide2ID_4`) REFERENCES `tblPeptide` (`PepID`) ON DELETE SET NULL,
  CONSTRAINT `0_1330` FOREIGN KEY (`Peptide3ID_4`) REFERENCES `tblPeptide` (`PepID`) ON DELETE SET NULL,
  CONSTRAINT `0_1331` FOREIGN KEY (`DNAID_1`) REFERENCES `tblDNA` (`DNAID`) ON DELETE SET NULL,
  CONSTRAINT `0_1332` FOREIGN KEY (`DNA2ID_1`) REFERENCES `tblDNA` (`DNAID`) ON DELETE SET NULL,
  CONSTRAINT `0_1333` FOREIGN KEY (`DNA3ID_1`) REFERENCES `tblDNA` (`DNAID`) ON DELETE SET NULL,
  CONSTRAINT `0_1334` FOREIGN KEY (`DNAID_2`) REFERENCES `tblDNA` (`DNAID`) ON DELETE SET NULL,
  CONSTRAINT `0_1335` FOREIGN KEY (`DNA2ID_2`) REFERENCES `tblDNA` (`DNAID`) ON DELETE SET NULL,
  CONSTRAINT `0_1336` FOREIGN KEY (`DNA3ID_2`) REFERENCES `tblDNA` (`DNAID`) ON DELETE SET NULL,
  CONSTRAINT `0_1337` FOREIGN KEY (`DNAID_3`) REFERENCES `tblDNA` (`DNAID`) ON DELETE SET NULL,
  CONSTRAINT `0_1338` FOREIGN KEY (`DNA2ID_3`) REFERENCES `tblDNA` (`DNAID`) ON DELETE SET NULL,
  CONSTRAINT `0_1339` FOREIGN KEY (`DNA3ID_3`) REFERENCES `tblDNA` (`DNAID`) ON DELETE SET NULL,
  CONSTRAINT `0_1340` FOREIGN KEY (`DNAID_4`) REFERENCES `tblDNA` (`DNAID`) ON DELETE SET NULL,
  CONSTRAINT `0_1341` FOREIGN KEY (`DNA2ID_4`) REFERENCES `tblDNA` (`DNAID`) ON DELETE SET NULL,
  CONSTRAINT `0_1342` FOREIGN KEY (`DNA3ID_4`) REFERENCES `tblDNA` (`DNAID`) ON DELETE SET NULL,
  CONSTRAINT `0_1343` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblDNA`
--

DROP TABLE IF EXISTS `tblDNA`;
CREATE TABLE `tblDNA` (
  `DNAID` int(11) NOT NULL auto_increment,
  `DNAFileName` varchar(50) default NULL,
  `Description` varchar(255) default NULL,
  `Sequence` text,
  `InvestigatorID` int(11) default NULL,
  `vbar` float(7,5) default '0.00000',
  `e260` float(12,3) default '0.000',
  `e280` float(12,3) default '0.000',
  PRIMARY KEY  (`DNAID`),
  KEY `Invid` (`InvestigatorID`),
  CONSTRAINT `0_1345` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE,
  CONSTRAINT `0_1346` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`),
  CONSTRAINT `0_1347` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblEquilResult`
--

DROP TABLE IF EXISTS `tblEquilResult`;
CREATE TABLE `tblEquilResult` (
  `EquilRstID` int(11) NOT NULL default '0',
  `Date` varchar(15) default NULL,
  `RunID` varchar(250) default NULL,
  `InvestigatorID` int(11) default NULL,
  `RunRequestID` int(11) default NULL,
  `DataID` int(11) default NULL,
  PRIMARY KEY  (`EquilRstID`),
  KEY `invid` (`InvestigatorID`),
  KEY `dataid` (`DataID`),
  CONSTRAINT `0_1349` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE,
  CONSTRAINT `0_1350` FOREIGN KEY (`DataID`) REFERENCES `tblEquilResultData` (`tableID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblEquilResultData`
--

DROP TABLE IF EXISTS `tblEquilResultData`;
CREATE TABLE `tblEquilResultData` (
  `tableID` int(11) NOT NULL default '0',
  `Report_Tar` longblob,
  `Result_Tar` longblob,
  `Rawdata_Tar` longblob,
  PRIMARY KEY  (`tableID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblExpData`
--

DROP TABLE IF EXISTS `tblExpData`;
CREATE TABLE `tblExpData` (
  `ExpdataID` int(11) NOT NULL auto_increment,
  `Invid` int(11) default NULL,
  `Path` varchar(250) default NULL,
  `Date` varchar(15) default NULL,
  `Temperature` float(6,4) default NULL,
  `Duration` float(6,4) default NULL,
  `Edit_type` tinyint(1) default NULL,
  `Rotor` tinyint(1) default NULL,
  `Runid` varchar(100) default NULL,
  `Description` varchar(250) default NULL,
  `Cell1` varchar(250) default NULL,
  `Cell2` varchar(250) default NULL,
  `Cell3` varchar(250) default NULL,
  `Cell4` varchar(250) default NULL,
  `Cell5` varchar(250) default NULL,
  `Cell6` varchar(250) default NULL,
  `Cell7` varchar(250) default NULL,
  `Cell8` varchar(250) default NULL,
  `Cell1ID` int(11) default NULL,
  `Cell2ID` int(11) default NULL,
  `Cell3ID` int(11) default NULL,
  `Cell4ID` int(11) default NULL,
  `Cell5ID` int(11) default NULL,
  `Cell6ID` int(11) default NULL,
  `Cell7ID` int(11) default NULL,
  `Cell8ID` int(11) default NULL,
  `File` longblob,
  PRIMARY KEY  (`ExpdataID`),
  KEY `Invid` (`Invid`),
  KEY `cell1` (`Cell1ID`),
  KEY `cell2` (`Cell2ID`),
  KEY `cell3` (`Cell3ID`),
  KEY `cell4` (`Cell4ID`),
  KEY `cell5` (`Cell5ID`),
  KEY `cell6` (`Cell6ID`),
  KEY `cell7` (`Cell7ID`),
  KEY `cell8` (`Cell8ID`),
  CONSTRAINT `0_1353` FOREIGN KEY (`Invid`) REFERENCES `tblInvestigators` (`InvID`),
  CONSTRAINT `0_1354` FOREIGN KEY (`Cell1ID`) REFERENCES `tblCell` (`CellID`),
  CONSTRAINT `0_1355` FOREIGN KEY (`Cell2ID`) REFERENCES `tblCell` (`CellID`),
  CONSTRAINT `0_1356` FOREIGN KEY (`Cell3ID`) REFERENCES `tblCell` (`CellID`),
  CONSTRAINT `0_1357` FOREIGN KEY (`Cell4ID`) REFERENCES `tblCell` (`CellID`),
  CONSTRAINT `0_1358` FOREIGN KEY (`Cell5ID`) REFERENCES `tblCell` (`CellID`),
  CONSTRAINT `0_1359` FOREIGN KEY (`Cell6ID`) REFERENCES `tblCell` (`CellID`),
  CONSTRAINT `0_1360` FOREIGN KEY (`Cell7ID`) REFERENCES `tblCell` (`CellID`),
  CONSTRAINT `0_1361` FOREIGN KEY (`Cell8ID`) REFERENCES `tblCell` (`CellID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblExpDataFile`
--

DROP TABLE IF EXISTS `tblExpDataFile`;
CREATE TABLE `tblExpDataFile` (
  `ExpdataID` int(11) NOT NULL default '0',
  `File` longblob,
  PRIMARY KEY  (`ExpdataID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblGA_MW_Settings`
--

DROP TABLE IF EXISTS `tblGA_MW_Settings`;
CREATE TABLE `tblGA_MW_Settings` (
  `GA_MW_Settings_ID` int(11) NOT NULL auto_increment,
  `mw_min` double NOT NULL default '100',
  `mw_max` double NOT NULL default '1000',
  `oligomer` int(11) NOT NULL default '4',
  `ff0_min` double NOT NULL default '1',
  `ff0_max` double NOT NULL default '4',
  `montecarlo_value` int(11) NOT NULL default '0',
  `tinoise_option` tinyint(1) NOT NULL default '0',
  `demes_value` int(11) NOT NULL default '31',
  `genes_value` int(11) NOT NULL default '100',
  `generations_value` int(11) NOT NULL default '100',
  `crossover_value` int(11) NOT NULL default '50',
  `mutation_value` int(11) NOT NULL default '50',
  `plague_value` int(11) NOT NULL default '4',
  `elitism_value` int(11) NOT NULL default '2',
  `migration_value` int(11) NOT NULL default '3',
  `regularization_value` double NOT NULL default '5',
  `seed_value` int(11) NOT NULL default '0',
  `rinoise_option` tinyint(1) NOT NULL default '0',
  `simpoints` int(11) default NULL,
  `band_volume` double default NULL,
  `radial_grid` tinyint(4) default NULL,
  `time_grid` tinyint(4) default NULL,
  `meniscus_option` tinyint(1) NOT NULL default '0',
  `meniscus_value` double NOT NULL default '0',
  `HPCAnalysis_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`GA_MW_Settings_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblGA_SC_Settings`
--

DROP TABLE IF EXISTS `tblGA_SC_Settings`;
CREATE TABLE `tblGA_SC_Settings` (
  `GA_SC_Settings_ID` int(11) NOT NULL auto_increment,
  `montecarlo_value` int(11) NOT NULL default '0',
  `demes_value` int(11) NOT NULL default '31',
  `genes_value` int(11) NOT NULL default '100',
  `generations_value` int(11) NOT NULL default '100',
  `crossover_value` int(11) NOT NULL default '50',
  `mutation_value` int(11) NOT NULL default '50',
  `plague_value` int(11) NOT NULL default '4',
  `elitism_value` int(11) NOT NULL default '2',
  `migration_value` int(11) NOT NULL default '3',
  `regularization_value` double NOT NULL default '5',
  `seed_value` int(11) NOT NULL default '0',
  `tinoise_option` tinyint(1) NOT NULL default '0',
  `rinoise_option` tinyint(1) NOT NULL default '0',
  `meniscus_option` tinyint(1) NOT NULL default '0',
  `meniscus_value` double NOT NULL default '0',
  `constraint_data` mediumtext,
  `HPCAnalysis_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`GA_SC_Settings_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblGA_Settings`
--

DROP TABLE IF EXISTS `tblGA_Settings`;
CREATE TABLE `tblGA_Settings` (
  `GA_Settings_ID` int(11) NOT NULL auto_increment,
  `montecarlo_value` int(11) NOT NULL default '0',
  `demes_value` int(11) NOT NULL default '31',
  `genes_value` int(11) NOT NULL default '100',
  `generations_value` int(11) NOT NULL default '100',
  `crossover_value` int(11) NOT NULL default '50',
  `mutation_value` int(11) NOT NULL default '50',
  `plague_value` int(11) NOT NULL default '4',
  `elitism_value` int(11) NOT NULL default '2',
  `migration_value` int(11) NOT NULL default '3',
  `regularization_value` double NOT NULL default '5',
  `seed_value` int(11) NOT NULL default '0',
  `tinoise_option` tinyint(1) NOT NULL default '0',
  `rinoise_option` tinyint(1) NOT NULL default '0',
  `simpoints` int(11) default NULL,
  `band_volume` double default NULL,
  `radial_grid` tinyint(4) default NULL,
  `time_grid` tinyint(4) default NULL,
  `meniscus_option` tinyint(1) NOT NULL default '0',
  `meniscus_value` double NOT NULL default '0',
  `solute_value` int(11) NOT NULL default '3',
  `solute_data` mediumtext,
  `HPCAnalysis_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`GA_Settings_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblHPCAnalysis`
--

DROP TABLE IF EXISTS `tblHPCAnalysis`;
CREATE TABLE `tblHPCAnalysis` (
  `HPCAnalysis_ID` int(11) NOT NULL auto_increment,
  `DateTime` datetime NOT NULL default '0000-00-00 00:00:00',
  `EndDateTime` datetime default NULL,
  `Email` varchar(80) NOT NULL default '',
  `Walltime` int(11) NOT NULL default '0',
  `CPUTime` double NOT NULL default '0',
  `Architecture` varchar(80) default NULL,
  `Cluster_Name` varchar(80) default NULL,
  `CPU_Number` int(11) default '0',
  `Datapoints` int(11) default '0',
  `RI_Noise_Fitted` tinyint(1) NOT NULL default '0',
  `TI_Noise_Fitted` tinyint(1) NOT NULL default '0',
  `max_rss` int(11) default '0',
  `Method` enum('2DSA','2DSA_MW','GA','GA_MW','GA_SC') NOT NULL default '2DSA',
  `queue_email` varchar(80) NOT NULL default '',
  `queue_name` varchar(80) NOT NULL default '',
  `Investigator_ID` int(11) NOT NULL default '0',
  `Submitter_ID` int(11) NOT NULL default '0',
  `HPCAnalysisGroup_ID` int(11) default NULL,
  PRIMARY KEY  (`HPCAnalysis_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblHPCAnalysisData`
--

DROP TABLE IF EXISTS `tblHPCAnalysisData`;
CREATE TABLE `tblHPCAnalysisData` (
  `HPCAnalysisData_ID` int(11) NOT NULL auto_increment,
  `file_name` varchar(64) default NULL,
  `path` varchar(128) default NULL,
  `cell` int(11) NOT NULL default '0',
  `channel` int(11) NOT NULL default '0',
  `wavelength` int(11) NOT NULL default '0',
  `ExpdataID` int(11) NOT NULL default '0',
  `HPCAnalysis_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`HPCAnalysisData_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblHPCAnalysisGroup`
--

DROP TABLE IF EXISTS `tblHPCAnalysisGroup`;
CREATE TABLE `tblHPCAnalysisGroup` (
  `HPCAnalysisGroup_ID` int(11) NOT NULL auto_increment,
  `DateTime` datetime NOT NULL default '0000-00-00 00:00:00',
  `Email` varchar(80) NOT NULL default '',
  `queue_email` varchar(80) NOT NULL default '',
  `Investigator_ID` int(11) NOT NULL default '0',
  `Submitter_ID` int(11) NOT NULL default '0',
  `HPCFirst_ID` int(11) NOT NULL,
  PRIMARY KEY  (`HPCAnalysisGroup_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblHPCModel`
--

DROP TABLE IF EXISTS `tblHPCModel`;
CREATE TABLE `tblHPCModel` (
  `HPCModel_ID` int(11) NOT NULL auto_increment,
  `meniscus` double NOT NULL default '0',
  `model` mediumblob,
  `ti_noise` blob,
  `ri_noise` blob,
  `model_filename` text,
  `ti_noise_filename` text,
  `ri_noise_filename` text,
  `RMSD` double NOT NULL default '0',
  `HPCAnalysis_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`HPCModel_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblHPCSoluteData`
--

DROP TABLE IF EXISTS `tblHPCSoluteData`;
CREATE TABLE `tblHPCSoluteData` (
  `HPCSoluteData_ID` int(11) NOT NULL auto_increment,
  `s_min` double NOT NULL default '0',
  `s_max` double NOT NULL default '0',
  `ff0_min` double NOT NULL default '0',
  `ff0_max` double NOT NULL default '0',
  `HPCAnalysis_ID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`HPCSoluteData_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblImage`
--

DROP TABLE IF EXISTS `tblImage`;
CREATE TABLE `tblImage` (
  `GelID` int(11) NOT NULL auto_increment,
  `GelPicture` longblob NOT NULL,
  `Date` datetime default NULL,
  `Description` varchar(80) NOT NULL default 'No Description was entered for this image',
  `InvestigatorID` int(11) NOT NULL default '0',
  PRIMARY KEY  (`GelID`),
  KEY `Invid` (`InvestigatorID`),
  CONSTRAINT `0_1363` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE,
  CONSTRAINT `0_1364` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`),
  CONSTRAINT `0_1365` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblInvestigators`
--

DROP TABLE IF EXISTS `tblInvestigators`;
CREATE TABLE `tblInvestigators` (
  `InvID` int(11) NOT NULL auto_increment,
  `FirstName` varchar(30) default NULL,
  `LastName` varchar(30) default NULL,
  `Address` varchar(255) default NULL,
  `City` varchar(30) default NULL,
  `State` char(2) default NULL,
  `Zip` varchar(10) default NULL,
  `Phone` varchar(100) default NULL,
  `Email` varchar(100) default NULL,
  `Organization` varchar(50) default NULL,
  PRIMARY KEY  (`InvID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblLaser`
--

DROP TABLE IF EXISTS `tblLaser`;
CREATE TABLE `tblLaser` (
  `LaserID` int(11) NOT NULL default '0',
  `SampleName` varchar(255) default NULL,
  `Operator` varchar(100) default NULL,
  `Date` varchar(15) default NULL,
  `InvestigatorID` int(11) default NULL,
  `BufferID` int(11) default NULL,
  `PeptideID` int(11) default NULL,
  `DNAID` int(11) default NULL,
  PRIMARY KEY  (`LaserID`),
  KEY `Invid` (`InvestigatorID`),
  KEY `bufid` (`BufferID`),
  KEY `pepid` (`PeptideID`),
  KEY `dnaid` (`DNAID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblLaserData`
--

DROP TABLE IF EXISTS `tblLaserData`;
CREATE TABLE `tblLaserData` (
  `LaserID` int(11) NOT NULL default '0',
  `DataFile` longblob,
  PRIMARY KEY  (`LaserID`),
  KEY `laserid` (`LaserID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblLog`
--

DROP TABLE IF EXISTS `tblLog`;
CREATE TABLE `tblLog` (
  `LogID` int(11) NOT NULL auto_increment,
  `InvestigatorID` int(11) default NULL,
  `Date` datetime default NULL,
  `LogEntry` text,
  PRIMARY KEY  (`LogID`),
  KEY `Invid` (`InvestigatorID`),
  CONSTRAINT `0_1368` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE,
  CONSTRAINT `0_1369` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`),
  CONSTRAINT `0_1370` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblPeptide`
--

DROP TABLE IF EXISTS `tblPeptide`;
CREATE TABLE `tblPeptide` (
  `PepID` int(11) NOT NULL auto_increment,
  `PepFileName` varchar(60) default NULL,
  `Description` varchar(255) default NULL,
  `Sequence` text,
  `InvestigatorID` int(11) default NULL,
  `vbar` float(7,5) default '0.00000',
  `e280` float(12,3) default '0.000',
  PRIMARY KEY  (`PepID`),
  KEY `Invid` (`InvestigatorID`),
  CONSTRAINT `0_1372` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE,
  CONSTRAINT `0_1373` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`),
  CONSTRAINT `0_1374` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblPermits`
--

DROP TABLE IF EXISTS `tblPermits`;
CREATE TABLE `tblPermits` (
  `providerID` int(11) default NULL,
  `collaboratorID` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblRawExpData`
--

DROP TABLE IF EXISTS `tblRawExpData`;
CREATE TABLE `tblRawExpData` (
  `ExpdataID` int(11) NOT NULL default '0',
  `RawData` longblob,
  PRIMARY KEY  (`ExpdataID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblRequest`
--

DROP TABLE IF EXISTS `tblRequest`;
CREATE TABLE `tblRequest` (
  `RequestID` int(11) NOT NULL auto_increment,
  `InvestigatorID` int(11) default NULL,
  `Goals` text,
  `Molecules` text,
  `Purity` varchar(10) default NULL,
  `Expense` text,
  `BufferComponents` text,
  `SaltInformation` text,
  `AUCQuestions` text,
  `Notes` text,
  `ProjectDescription` text,
  `Status` varchar(32) default 'submitted',
  PRIMARY KEY  (`RequestID`),
  KEY `Invid` (`InvestigatorID`),
  CONSTRAINT `0_1376` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE,
  CONSTRAINT `0_1377` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`),
  CONSTRAINT `0_1378` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblResult`
--

DROP TABLE IF EXISTS `tblResult`;
CREATE TABLE `tblResult` (
  `ResultID` int(11) NOT NULL default '0',
  `RunRequestID` int(11) default NULL,
  `VelocDataID` int(11) default NULL,
  `EquilDataID` int(11) default NULL,
  `EquilProjectID` int(11) default NULL,
  `MontecarloID` int(11) default NULL,
  `InvestigatorID` int(11) default NULL,
  PRIMARY KEY  (`ResultID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblRunRequest`
--

DROP TABLE IF EXISTS `tblRunRequest`;
CREATE TABLE `tblRunRequest` (
  `RunRequestID` int(11) NOT NULL auto_increment,
  `RunDescription` varchar(80) default NULL,
  `SampleID1` int(11) default NULL,
  `SampleID2` int(11) default NULL,
  `SampleID3` int(11) default NULL,
  `SampleID4` int(11) default NULL,
  `SampleID5` int(11) default NULL,
  `SampleID6` int(11) default NULL,
  `SampleID7` int(11) default NULL,
  `SampleID8` int(11) default NULL,
  `RunType` varchar(50) default NULL,
  `InvestigatorID` int(11) default NULL,
  `RequestID` int(11) default NULL,
  PRIMARY KEY  (`RunRequestID`),
  KEY `sampleid1` (`SampleID1`),
  KEY `sampleid2` (`SampleID2`),
  KEY `sampleid3` (`SampleID3`),
  KEY `sampleid4` (`SampleID4`),
  KEY `sampleid5` (`SampleID5`),
  KEY `sampleid6` (`SampleID6`),
  KEY `sampleid7` (`SampleID7`),
  KEY `sampleid8` (`SampleID8`),
  KEY `Invid` (`InvestigatorID`),
  KEY `reqid` (`RequestID`),
  CONSTRAINT `0_1380` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE,
  CONSTRAINT `0_1381` FOREIGN KEY (`SampleID1`) REFERENCES `tblSample` (`SampleID`),
  CONSTRAINT `0_1382` FOREIGN KEY (`SampleID2`) REFERENCES `tblSample` (`SampleID`),
  CONSTRAINT `0_1383` FOREIGN KEY (`SampleID3`) REFERENCES `tblSample` (`SampleID`),
  CONSTRAINT `0_1384` FOREIGN KEY (`SampleID4`) REFERENCES `tblSample` (`SampleID`),
  CONSTRAINT `0_1385` FOREIGN KEY (`SampleID5`) REFERENCES `tblSample` (`SampleID`),
  CONSTRAINT `0_1386` FOREIGN KEY (`SampleID6`) REFERENCES `tblSample` (`SampleID`),
  CONSTRAINT `0_1387` FOREIGN KEY (`SampleID7`) REFERENCES `tblSample` (`SampleID`),
  CONSTRAINT `0_1388` FOREIGN KEY (`SampleID8`) REFERENCES `tblSample` (`SampleID`),
  CONSTRAINT `0_1389` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`),
  CONSTRAINT `0_1390` FOREIGN KEY (`RequestID`) REFERENCES `tblRequest` (`RequestID`),
  CONSTRAINT `0_1391` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblSample`
--

DROP TABLE IF EXISTS `tblSample`;
CREATE TABLE `tblSample` (
  `SampleID` int(11) NOT NULL auto_increment,
  `Description` varchar(80) NOT NULL default '',
  `PeptideID` int(11) default NULL,
  `DNAID` int(11) default NULL,
  `BufferID` int(11) default NULL,
  `StorageTemperature` tinyint(4) default NULL,
  `Vbar` float(7,5) unsigned zerofill default '0.00000',
  `RunTemperature` tinyint(4) default NULL,
  `Notes` text,
  `ImageID` int(11) default NULL,
  `InvestigatorID` int(11) default NULL,
  PRIMARY KEY  (`SampleID`),
  KEY `pep_id` (`PeptideID`),
  KEY `buf_id` (`BufferID`),
  KEY `dna_id` (`DNAID`),
  KEY `img_id` (`ImageID`),
  KEY `Invid` (`InvestigatorID`),
  CONSTRAINT `0_1393` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE,
  CONSTRAINT `0_1394` FOREIGN KEY (`PeptideID`) REFERENCES `tblPeptide` (`PepID`),
  CONSTRAINT `0_1395` FOREIGN KEY (`DNAID`) REFERENCES `tblDNA` (`DNAID`),
  CONSTRAINT `0_1396` FOREIGN KEY (`BufferID`) REFERENCES `tblBuffer` (`BuffID`),
  CONSTRAINT `0_1397` FOREIGN KEY (`ImageID`) REFERENCES `tblImage` (`GelID`),
  CONSTRAINT `0_1398` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`),
  CONSTRAINT `0_1399` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblVelocResult`
--

DROP TABLE IF EXISTS `tblVelocResult`;
CREATE TABLE `tblVelocResult` (
  `VelocRstID` int(11) NOT NULL default '0',
  `Date` varchar(15) default NULL,
  `RunID` varchar(250) default NULL,
  `InvestigatorID` int(11) default NULL,
  `RunRequestID` int(11) default NULL,
  `DataID` int(11) default NULL,
  PRIMARY KEY  (`VelocRstID`),
  KEY `invid` (`InvestigatorID`),
  KEY `dataid` (`DataID`),
  CONSTRAINT `0_1405` FOREIGN KEY (`InvestigatorID`) REFERENCES `tblInvestigators` (`InvID`) ON DELETE CASCADE,
  CONSTRAINT `0_1406` FOREIGN KEY (`DataID`) REFERENCES `tblVelocResultData` (`tableID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `tblVelocResultData`
--

DROP TABLE IF EXISTS `tblVelocResultData`;
CREATE TABLE `tblVelocResultData` (
  `tableID` int(11) NOT NULL default '0',
  `Report_Tar` longblob,
  `Result_Tar` longblob,
  `Rawdata_Tar` longblob,
  PRIMARY KEY  (`tableID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

