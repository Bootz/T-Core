/*
MySQL Data Transfer
Source Host: localhost
Source Database: test
Target Host: localhost
Target Database: test
Date: 24.08.2011 07:45:33
*/

SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for map_table
-- ----------------------------
DROP TABLE IF EXISTS `map_table`;
CREATE TABLE `map_table` (
  `MapID` int(11) NOT NULL,
  `NodeID` int(11) NOT NULL,
  `BackupNodeID` int(11) NOT NULL,
  `SubNodeID` int(11) NOT NULL,
  PRIMARY KEY (`MapID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for nodecharacters
-- ----------------------------
DROP TABLE IF EXISTS `nodecharacters`;
CREATE TABLE `nodecharacters` (
  `realmid` int(11) unsigned NOT NULL DEFAULT '0',
  `acctid` bigint(20) unsigned NOT NULL,
  `numchars` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`realmid`,`acctid`),
  KEY `acctid` (`acctid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Realm Character Tracker';

-- ----------------------------
-- Table structure for nodelist
-- ----------------------------
DROP TABLE IF EXISTS `nodelist`;
CREATE TABLE `nodelist` (
  `NodeID` int(11) NOT NULL,
  `Name` varchar(32) NOT NULL,
  `Address` varchar(32) NOT NULL,
  `Port` int(11) NOT NULL,
  `Online` tinyint(3) NOT NULL,
  `allowedSecurityLevel` tinyint(3) NOT NULL,
  PRIMARY KEY (`NodeID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for uptime
-- ----------------------------
DROP TABLE IF EXISTS `uptime`;
CREATE TABLE `uptime` (
  `realmid` int(11) unsigned NOT NULL,
  `starttime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `startstring` varchar(64) NOT NULL DEFAULT '',
  `uptime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `maxplayers` smallint(5) unsigned NOT NULL DEFAULT '0',
  `revision` varchar(255) NOT NULL DEFAULT 'Trilliumcore',
  PRIMARY KEY (`realmid`,`starttime`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Uptime system';

-- ----------------------------
-- Records 
-- ----------------------------
INSERT INTO `map_table` VALUES ('0', '1', '0', '0');
INSERT INTO `map_table` VALUES ('1', '1', '0', '0');
INSERT INTO `map_table` VALUES ('13', '1', '0', '0');
INSERT INTO `map_table` VALUES ('25', '1', '0', '0');
INSERT INTO `map_table` VALUES ('30', '1', '0', '0');
INSERT INTO `map_table` VALUES ('33', '1', '0', '0');
INSERT INTO `map_table` VALUES ('34', '1', '0', '0');
INSERT INTO `map_table` VALUES ('35', '1', '0', '0');
INSERT INTO `map_table` VALUES ('36', '1', '0', '0');
INSERT INTO `map_table` VALUES ('37', '1', '0', '0');
INSERT INTO `map_table` VALUES ('42', '1', '0', '0');
INSERT INTO `map_table` VALUES ('43', '1', '0', '0');
INSERT INTO `map_table` VALUES ('44', '1', '0', '0');
INSERT INTO `map_table` VALUES ('47', '1', '0', '0');
INSERT INTO `map_table` VALUES ('48', '1', '0', '0');
INSERT INTO `map_table` VALUES ('70', '1', '0', '0');
INSERT INTO `map_table` VALUES ('90', '1', '0', '0');
INSERT INTO `map_table` VALUES ('109', '1', '0', '0');
INSERT INTO `map_table` VALUES ('129', '1', '0', '0');
INSERT INTO `map_table` VALUES ('169', '1', '0', '0');
INSERT INTO `map_table` VALUES ('189', '1', '0', '0');
INSERT INTO `map_table` VALUES ('209', '1', '0', '0');
INSERT INTO `map_table` VALUES ('229', '1', '0', '0');
INSERT INTO `map_table` VALUES ('230', '1', '0', '0');
INSERT INTO `map_table` VALUES ('249', '1', '0', '0');
INSERT INTO `map_table` VALUES ('269', '1', '0', '0');
INSERT INTO `map_table` VALUES ('289', '1', '0', '0');
INSERT INTO `map_table` VALUES ('309', '1', '0', '0');
INSERT INTO `map_table` VALUES ('329', '1', '0', '0');
INSERT INTO `map_table` VALUES ('349', '1', '0', '0');
INSERT INTO `map_table` VALUES ('369', '1', '0', '0');
INSERT INTO `map_table` VALUES ('389', '1', '0', '0');
INSERT INTO `map_table` VALUES ('409', '1', '0', '0');
INSERT INTO `map_table` VALUES ('429', '1', '0', '0');
INSERT INTO `map_table` VALUES ('449', '1', '0', '0');
INSERT INTO `map_table` VALUES ('450', '1', '0', '0');
INSERT INTO `map_table` VALUES ('451', '1', '0', '0');
INSERT INTO `map_table` VALUES ('469', '1', '0', '0');
INSERT INTO `map_table` VALUES ('489', '1', '0', '0');
INSERT INTO `map_table` VALUES ('509', '1', '0', '0');
INSERT INTO `map_table` VALUES ('529', '1', '0', '0');
INSERT INTO `map_table` VALUES ('530', '1', '0', '0');
INSERT INTO `map_table` VALUES ('531', '1', '0', '0');
INSERT INTO `map_table` VALUES ('532', '1', '0', '0');
INSERT INTO `map_table` VALUES ('533', '1', '0', '0');
INSERT INTO `map_table` VALUES ('534', '1', '0', '0');
INSERT INTO `map_table` VALUES ('540', '1', '0', '0');
INSERT INTO `map_table` VALUES ('542', '1', '0', '0');
INSERT INTO `map_table` VALUES ('543', '1', '0', '0');
INSERT INTO `map_table` VALUES ('544', '1', '0', '0');
INSERT INTO `map_table` VALUES ('545', '1', '0', '0');
INSERT INTO `map_table` VALUES ('546', '1', '0', '0');
INSERT INTO `map_table` VALUES ('547', '1', '0', '0');
INSERT INTO `map_table` VALUES ('548', '1', '0', '0');
INSERT INTO `map_table` VALUES ('550', '1', '0', '0');
INSERT INTO `map_table` VALUES ('552', '1', '0', '0');
INSERT INTO `map_table` VALUES ('553', '1', '0', '0');
INSERT INTO `map_table` VALUES ('554', '1', '0', '0');
INSERT INTO `map_table` VALUES ('555', '1', '0', '0');
INSERT INTO `map_table` VALUES ('556', '1', '0', '0');
INSERT INTO `map_table` VALUES ('557', '1', '0', '0');
INSERT INTO `map_table` VALUES ('558', '1', '0', '0');
INSERT INTO `map_table` VALUES ('559', '1', '0', '0');
INSERT INTO `map_table` VALUES ('560', '1', '0', '0');
INSERT INTO `map_table` VALUES ('562', '1', '0', '0');
INSERT INTO `map_table` VALUES ('564', '1', '0', '0');
INSERT INTO `map_table` VALUES ('565', '1', '0', '0');
INSERT INTO `map_table` VALUES ('566', '1', '0', '0');
INSERT INTO `map_table` VALUES ('568', '1', '0', '0');
INSERT INTO `map_table` VALUES ('571', '1', '0', '0');
INSERT INTO `map_table` VALUES ('572', '1', '0', '0');
INSERT INTO `map_table` VALUES ('573', '1', '0', '0');
INSERT INTO `map_table` VALUES ('574', '1', '0', '0');
INSERT INTO `map_table` VALUES ('575', '1', '0', '0');
INSERT INTO `map_table` VALUES ('576', '1', '0', '0');
INSERT INTO `map_table` VALUES ('580', '1', '0', '0');
INSERT INTO `map_table` VALUES ('578', '1', '0', '0');
INSERT INTO `map_table` VALUES ('582', '1', '0', '0');
INSERT INTO `map_table` VALUES ('584', '1', '0', '0');
INSERT INTO `map_table` VALUES ('585', '1', '0', '0');
INSERT INTO `map_table` VALUES ('586', '1', '0', '0');
INSERT INTO `map_table` VALUES ('587', '1', '0', '0');
INSERT INTO `map_table` VALUES ('588', '1', '0', '0');
INSERT INTO `map_table` VALUES ('589', '1', '0', '0');
INSERT INTO `map_table` VALUES ('590', '1', '0', '0');
INSERT INTO `map_table` VALUES ('591', '1', '0', '0');
INSERT INTO `map_table` VALUES ('592', '1', '0', '0');
INSERT INTO `map_table` VALUES ('593', '1', '0', '0');
INSERT INTO `map_table` VALUES ('594', '1', '0', '0');
INSERT INTO `map_table` VALUES ('595', '1', '0', '0');
INSERT INTO `map_table` VALUES ('596', '1', '0', '0');
INSERT INTO `map_table` VALUES ('597', '1', '0', '0');
INSERT INTO `map_table` VALUES ('598', '1', '0', '0');
INSERT INTO `map_table` VALUES ('599', '1', '0', '0');
INSERT INTO `map_table` VALUES ('600', '1', '0', '0');
INSERT INTO `map_table` VALUES ('601', '1', '0', '0');
INSERT INTO `map_table` VALUES ('603', '1', '0', '0');
INSERT INTO `map_table` VALUES ('602', '1', '0', '0');
INSERT INTO `map_table` VALUES ('604', '1', '0', '0');
INSERT INTO `map_table` VALUES ('605', '1', '0', '0');
INSERT INTO `map_table` VALUES ('606', '1', '0', '0');
INSERT INTO `map_table` VALUES ('607', '1', '0', '0');
INSERT INTO `map_table` VALUES ('608', '1', '0', '0');
INSERT INTO `map_table` VALUES ('609', '1', '0', '0');
INSERT INTO `map_table` VALUES ('610', '1', '0', '0');
INSERT INTO `map_table` VALUES ('612', '1', '0', '0');
INSERT INTO `map_table` VALUES ('613', '1', '0', '0');
INSERT INTO `map_table` VALUES ('614', '1', '0', '0');
INSERT INTO `map_table` VALUES ('615', '1', '0', '0');
INSERT INTO `map_table` VALUES ('616', '1', '0', '0');
INSERT INTO `map_table` VALUES ('617', '1', '0', '0');
INSERT INTO `map_table` VALUES ('618', '1', '0', '0');
INSERT INTO `map_table` VALUES ('619', '1', '0', '0');
INSERT INTO `map_table` VALUES ('620', '1', '0', '0');
INSERT INTO `map_table` VALUES ('621', '1', '0', '0');
INSERT INTO `map_table` VALUES ('622', '1', '0', '0');
INSERT INTO `map_table` VALUES ('623', '1', '0', '0');
INSERT INTO `map_table` VALUES ('624', '1', '0', '0');
INSERT INTO `map_table` VALUES ('625', '1', '0', '0');
INSERT INTO `map_table` VALUES ('628', '1', '0', '0');
INSERT INTO `map_table` VALUES ('631', '1', '0', '0');
INSERT INTO `map_table` VALUES ('632', '1', '0', '0');
INSERT INTO `map_table` VALUES ('641', '1', '0', '0');
INSERT INTO `map_table` VALUES ('642', '1', '0', '0');
INSERT INTO `map_table` VALUES ('647', '1', '0', '0');
INSERT INTO `map_table` VALUES ('649', '1', '0', '0');
INSERT INTO `map_table` VALUES ('650', '1', '0', '0');
INSERT INTO `map_table` VALUES ('658', '1', '0', '0');
INSERT INTO `map_table` VALUES ('668', '1', '0', '0');
INSERT INTO `map_table` VALUES ('672', '1', '0', '0');
INSERT INTO `map_table` VALUES ('712', '1', '0', '0');
INSERT INTO `map_table` VALUES ('713', '1', '0', '0');
INSERT INTO `map_table` VALUES ('718', '1', '0', '0');
INSERT INTO `map_table` VALUES ('723', '1', '0', '0');
INSERT INTO `map_table` VALUES ('724', '1', '0', '0');
INSERT INTO `nodelist` VALUES ('1', 'Alpha', '127.0.0.1', '8099', '0', '0');
