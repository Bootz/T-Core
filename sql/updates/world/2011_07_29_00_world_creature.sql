-- Drop DeathState column
ALTER TABLE `creature` DROP COLUMN `DeathState`;

DELETE FROM `command` WHERE `name`= 'npc set deathstate';

