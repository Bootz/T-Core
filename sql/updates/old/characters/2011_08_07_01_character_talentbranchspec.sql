CREATE TABLE `character_talentbranchspec` (
	`guid` int(11) UNSIGNED NOT NULL DEFAULT '0',
	`talentSpec` int(11) UNSIGNED NOT NULL DEFAULT '0',
	`talentBranchSpec` int(11) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`guid`, `talentSpec`)
);