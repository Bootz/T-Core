-- item_set_names
DROP TABLE IF EXISTS `item_set_names`;

-- locales_item_set_names
DROP TABLE IF EXISTS `locales_item_set_names`;

-- command
DELETE FROM `command` WHERE `name` = 'reload item_set_names';
DELETE FROM `command` WHERE `name` = 'reload locales_item_set_name';