ALTER TABLE `characters`
  ADD COLUMN `achievementPoints` SMALLINT(5) UNSIGNED NOT NULL DEFAULT 0 AFTER `knownTitles`;