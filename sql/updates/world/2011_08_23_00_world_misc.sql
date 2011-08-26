-- Table gameobject_template
-- Right sort
ALTER TABLE `gameobject_template`
DROP COLUMN `data24`,
DROP COLUMN `data25`,
DROP COLUMN `data26`,
DROP COLUMN `data27`,
DROP COLUMN `data28`,
DROP COLUMN `data29`,
DROP COLUMN `data30`,
DROP COLUMN `data31`,
DROP COLUMN `unk2`,
ADD COLUMN `data24` INT(10) UNSIGNED DEFAULT '0' NOT NULL AFTER `data23`,
ADD COLUMN `data25` INT(10) UNSIGNED DEFAULT '0' NOT NULL AFTER `data24`,
ADD COLUMN `data26` INT(10) UNSIGNED DEFAULT '0' NOT NULL AFTER `data25`,
ADD COLUMN `data27` INT(10) UNSIGNED DEFAULT '0' NOT NULL AFTER `data26`,
ADD COLUMN `data28` INT(10) UNSIGNED DEFAULT '0' NOT NULL AFTER `data27`,
ADD COLUMN `data29` INT(10) UNSIGNED DEFAULT '0' NOT NULL AFTER `data28`,
ADD COLUMN `data30` INT(10) UNSIGNED DEFAULT '0' NOT NULL AFTER `data29`,
ADD COLUMN `data31` INT(10) UNSIGNED DEFAULT '0' NOT NULL AFTER `data30`,
ADD COLUMN `unk2` INT(3) UNSIGNED DEFAULT '0' NOT NULL AFTER `data31`;

-- Table quest_template
-- Cache
UPDATE `quest_template` SET `ReqCreatureOrGOCount1`='1' WHERE (`entry`='14293');
UPDATE `quest_template` SET `ReqCreatureOrGOCount1`='1' WHERE (`entry`='14274');
UPDATE `quest_template` SET `ReqCreatureOrGOCount1`='2' WHERE (`entry`='14279');
UPDATE `quest_template` SET `ReqCreatureOrGOCount1`='1' WHERE (`entry`='14266');
UPDATE `quest_template` SET `ReqCreatureOrGOCount1`='1' WHERE (`entry`='14272');
UPDATE `quest_template` SET `ReqCreatureOrGOCount1`='2' WHERE (`entry`='14276');

-- Table outdoorpvp_template
DELETE FROM `outdoorpvp_template` WHERE `TypeId` = '7';

-- Table creature_template
-- Cache
UPDATE `creature_template` SET `modelid1`='14973', `modelid2`='14974', `modelid3`='14975', `modelid4`='14976' WHERE (`entry`='24529');

-- Table creature_ai_texts
DELETE FROM `creature_ai_texts` WHERE `entry` = '0';

-- Table item_template
-- Item-sparse.db2
UPDATE `item_template` SET `spellid_1`='0', `spellid_2`='0' WHERE `entry` IN (27689);

-- Table spell_dbc
DELETE FROM `spell_dbc` WHERE `Id` IN (23468, 23469, 27964);
INSERT INTO `spell_dbc` (`Id`) VALUES
(23468),
(23469),
(27964);

-- Table spell_linked_spell
DELETE FROM `spell_linked_spell` WHERE `spell_trigger` IN (51209);

-- Table npc_trainer
DELETE FROM `npc_trainer` WHERE `spell` IN (165);

-- Table spell_bonus_data
DELETE FROM `spell_bonus_data` WHERE `entry` IN (50294);