ALTER TABLE `quest_template` ADD `RewCurrencyId1` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `OfferRewardEmoteDelay4`;
ALTER TABLE `quest_template` ADD `RewCurrencyCount1` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `RewCurrencyId1`;
ALTER TABLE `quest_template` ADD `RewCurrencyId2` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `RewCurrencyCount1`;
ALTER TABLE `quest_template` ADD `RewCurrencyCount2` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `RewCurrencyId2`;
ALTER TABLE `quest_template` ADD `RewCurrencyId3` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `RewCurrencyCount2`;
ALTER TABLE `quest_template` ADD `RewCurrencyCount3` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `RewCurrencyId3`;
ALTER TABLE `quest_template` ADD `RewCurrencyId4` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `RewCurrencyCount3`;
ALTER TABLE `quest_template` ADD `RewCurrencyCount4` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `RewCurrencyId4`;
ALTER TABLE `quest_template` ADD `ReqCurrencyId1` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `RewCurrencyCount4`;
ALTER TABLE `quest_template` ADD `ReqCurrencyCount1` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `ReqCurrencyId1`;
ALTER TABLE `quest_template` ADD `ReqCurrencyId2` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `ReqCurrencyCount1`;
ALTER TABLE `quest_template` ADD `ReqCurrencyCount2` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `ReqCurrencyId2`;
ALTER TABLE `quest_template` ADD `ReqCurrencyId3` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `ReqCurrencyCount2`;
ALTER TABLE `quest_template` ADD `ReqCurrencyCount3` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `ReqCurrencyId3`;
ALTER TABLE `quest_template` ADD `ReqCurrencyId4` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `ReqCurrencyCount3`;
ALTER TABLE `quest_template` ADD `ReqCurrencyCount4` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `ReqCurrencyId4`;

ALTER TABLE `quest_template`  DROP COLUMN `RewSkillLineId`,  DROP COLUMN `RewSkillPoints`,  DROP COLUMN `RewRepMask`,  DROP COLUMN `QuestGiverPortrait`,  DROP COLUMN `QuestTurnInPortrait`,  DROP COLUMN `QuestGiverPortraitText`,  DROP COLUMN `QuestGiverPortraitUnk`,  DROP COLUMN `QuestTurnInPortraitText`,  DROP COLUMN `QuestTurnInPortraitUnk`,  DROP COLUMN `QuestTargetMark`,  DROP COLUMN `QuestStartType`,  DROP COLUMN `SoundAccept`,  DROP COLUMN `SoundTurnIn`,  DROP COLUMN `RequiredSpell`;