DROP TABLE IF EXISTS `creature_onkill_currency`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `creature_onkill_currency` (
  `creature_id` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'Creature Identifier',
  `type` mediumint(8) NOT NULL DEFAULT '0',
  `amount` mediumint(8) NOT NULL DEFAULT '0',
  PRIMARY KEY (`creature_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC COMMENT='Creature OnKill Currency gain';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `creature_onkill_currency`
--

LOCK TABLES `creature_onkill_currency` WRITE;
/*!40000 ALTER TABLE `creature_onkill_currency` DISABLE KEYS */;
/*!40000 ALTER TABLE `creature_onkill_currency` ENABLE KEYS */;
UNLOCK TABLES;