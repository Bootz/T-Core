ALTER TABLE `realmlist` CHANGE `gamebuild` `gamebuild` IN( 11 ) UNSIGNED NOT NULL DEFAULT '14333';
UPDATE `realmlist` SET `gamebuild`=14333;