ALTER TABLE `logs`
  CHANGE `time` `time` int(11) unsigned NOT NULL,
  CHANGE `realm` `realm` int(11) unsigned NOT NULL,
  CHANGE `type` `type` varchar(32) NOT NULL,
  ADD `level` tinyint(3) NOT NULL DEFAULT 0 AFTER `type`;