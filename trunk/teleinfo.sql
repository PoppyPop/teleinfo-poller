CREATE DATABASE IF NOT EXISTS `teleinfo`;

USE `teleinfo`;

CREATE TABLE IF NOT EXISTS `teleinfo` (
  `cle` bigint(10) NOT NULL AUTO_INCREMENT,
  `datecapture` datetime NOT NULL,
  `ADCO` varchar(12)  NOT NULL,
  `OPTARIF` varchar(4)  NOT NULL,
  `ISOUSC` tinyint(2) NOT NULL,
  
  `BASE` bigint(9) ,
 
  `HCHC` bigint(9) ,
  `HCHP` bigint(9) ,
  
  `EJPHN` bigint(9) ,
  `EJPHPM` bigint(9) ,
  
  `BBRHCJB` bigint(9) ,
  `BBRHPJB` bigint(9) ,
  `BBRHCJW` bigint(9) ,
  `BBRHPJW` bigint(9) ,
  `BBRHCJR` bigint(9) ,
  `BBRHPJR` bigint(9) ,
  
  `PEJP` tinyint(2) ,
  
  `PTEC` varchar(4)  NOT NULL,  
  
  `DEMAIN` varchar(4) ,

  `IINST` tinyint(3) ,
  
  `IINST1` tinyint(3) ,
  `IINST2` tinyint(3) ,
  `IINST3` tinyint(3) ,
  
  `ADPS` tinyint(3) ,
  `ADIR1` tinyint(3) ,
  `ADIR2` tinyint(3) ,
  `ADIR3` tinyint(3) ,
  
  `IMAX` tinyint(3) ,
  
  `IMAX1` tinyint(3) ,
  `IMAX2` tinyint(3) ,
  `IMAX3` tinyint(3) ,
  
  `PAPP` int(5) ,
  
  `HHPHC` varchar(1) ,
  
  `MOTDETAT` varchar(6),
  
  `PPOT` varchar(2),
  
  `NBESSAI` tinyint(3),
  
  PRIMARY KEY (`cle`)
);

CREATE UNIQUE INDEX IDX_TELE_DATECAPTURE ON teleinfo (datecapture);
