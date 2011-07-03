REVOKE ALL PRIVILEGES ON * . * FROM 'trillium'@'localhost';

REVOKE ALL PRIVILEGES ON `world` . * FROM 'trillium'@'localhost';

REVOKE GRANT OPTION ON `world` . * FROM 'trillium'@'localhost';

REVOKE ALL PRIVILEGES ON `characters` . * FROM 'trillium'@'localhost';

REVOKE GRANT OPTION ON `characters` . * FROM 'trillium'@'localhost';

REVOKE ALL PRIVILEGES ON `auth` . * FROM 'trillium'@'localhost';

REVOKE GRANT OPTION ON `auth` . * FROM 'trillium'@'localhost';

DROP USER 'trillium'@'localhost';

DROP DATABASE IF EXISTS `world`;

DROP DATABASE IF EXISTS `characters`;

DROP DATABASE IF EXISTS `auth`;
