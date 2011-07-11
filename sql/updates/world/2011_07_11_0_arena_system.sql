DELETE FROM command WHERE name='flusharenapoints';
DELETE FROM command WHERE name='weekreset';
INSERT INTO command (name, security, help) VALUES 
('weekreset', 3, 'Syntax: .weekreset\r\nForce Week to finish, and reset WeeGames / WeekWins.');

DELETE FROM trillium_string WHERE entry=1136;
INSERT INTO trillium_string (entry, content_default) VALUES 
(1136, 'Week is finished. Arena Stats (Week Games/Wins) set to 0.');