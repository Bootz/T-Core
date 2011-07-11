DELETE FROM `command` WHERE `name`='flusharenapoints'
INSERT INTO `command` (`name`, `security`, `help`) VALUES ('weekreset', 3, 'Syntax: .weekreset\r\nForce Week to finish, and reset WeeGames / WeekWins.');

INSERT INTO `trillium_string` (`entry`, `content_default`, `content_loc1`, `content_loc2`, `content_loc3`, `content_loc4`, `content_loc5`, `content_loc6`, `content_loc7`, `content_loc8`) VALUES (1136, 'Week is finished. Arena Stats (Week Games/Wins) set to 0.', '', '', '', '', '', '', '', '');

