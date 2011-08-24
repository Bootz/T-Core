ALTER TABLE characters
  add column `talentSpec` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT 0 AFTER `achievementPoints`;