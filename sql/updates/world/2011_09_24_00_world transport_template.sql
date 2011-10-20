DROP TABLE IF EXISTS `creature_transport`;
DROP TABLE IF EXISTS `transports`;

--
-- Table structure for table `transport_template`
--
DROP TABLE IF EXISTS `transport_template`;
CREATE TABLE `transport_template` (
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `name` text NOT NULL,
  `autoCreate` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `ScriptName` char(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`entry`),
  UNIQUE KEY `idx_entry` (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Transports';

--
-- Dumping data for table `transport_template`
--

INSERT INTO `transport_template` VALUES
(20808,'Ship (The Maiden''s Fancy)',1,''),
(164871,'Zeppelin (The Thundercaller)',1,''),
(175080,'Zeppelin (The Iron Eagle)',1,''),
(176231,'Ship (The Lady Mehley)',1,''),
(176244,'Ship, Night Elf (Moonspray)',1,''),
(176310,'Ship (The Bravery)',1,''),
(176495,'Zeppelin (The Purple Princess)',1,''),
(177233,'Ship, Night Elf (Feathermoon Ferry)',1,''),
(181056,'Naxxramas',1,''),
(181646,'Ship, Night Elf (Elune''s Blessing)',1,''),
(181688,'Ship, Icebreaker (Northspear)',1,''),
(181689,'Zeppelin, Horde (Cloudkisser)',1,''),
(186238,'Zeppelin, Horde (The Mighty Wind)',1,''),
(187568,'Turtle (Walker of Waves)',1,''),
(190536,'Ship, Icebreaker (Stormwind''s Pride)',1,''),
(188511,'Turtle (Green Island)',1,''),
(186371,'Zeppelin',1,''),
(187038,'Sister Mercy',1,''),
(192241,'Orgrim''s Hammer',1,''),
(192242,'The Skybreaker',1,''),
(195121,'Alliance Gunship',1,''),
(195276,'Horde Gunship',1,''),
(190549,'The Zephyr',1,''),
(201599,'Orgrim''s Hammer',1,''),
(201811,'The Skybreaker',1,''),
(201812,'Orgrim''s Hammer',1,''),
(201598,'The Skybreaker',1,''),
(201580,'The Skybreaker',1,''),
(201581,'Orgrim''s Hammer',1,''),
(201834,'Zeppelin, Horde (The Mighty Wind) (Icecrown Raid)',1,'');
