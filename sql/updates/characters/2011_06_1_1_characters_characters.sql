ALTER TABLE `characters` DROP `guildid`;
ALTER TABLE `characters` ADD `guildId` INT( 10 ) UNSIGNED NOT NULL DEFAULT '0' AFTER `actionBars`
