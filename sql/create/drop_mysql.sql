REVOKE ALL PRIVILEGES ON * . * FROM 'singularity'@'localhost';

REVOKE ALL PRIVILEGES ON `world` . * FROM 'singularity'@'localhost';

REVOKE GRANT OPTION ON `world` . * FROM 'singularity'@'localhost';

REVOKE ALL PRIVILEGES ON `characters` . * FROM 'singularity'@'localhost';

REVOKE GRANT OPTION ON `characters` . * FROM 'singularity'@'localhost';

REVOKE ALL PRIVILEGES ON `auth` . * FROM 'singularity'@'localhost';

REVOKE GRANT OPTION ON `auth` . * FROM 'singularity'@'localhost';

DROP USER 'singularity'@'localhost';

DROP DATABASE IF EXISTS `world`;

DROP DATABASE IF EXISTS `characters`;

DROP DATABASE IF EXISTS `auth`;
