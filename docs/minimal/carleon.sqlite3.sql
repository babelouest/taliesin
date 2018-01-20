
DROP TABLE IF EXISTS `c_element`;
DROP TABLE IF EXISTS `c_service`;
DROP TABLE IF EXISTS `c_mpd_websocket`;
DROP TABLE IF EXISTS `c_service_mpd`;

CREATE TABLE `c_service` (
  `cs_id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `cs_name` TEXT,
  `cs_enabled` INTEGER DEFAULT 0,
  `cs_description` TEXT
);

CREATE TABLE `c_element` (
  `ce_id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `cs_name` TEXT,
  `ce_name` TEXT,
  `ce_tag` TEXT
);

CREATE TABLE `c_service_mpd` (
  `cmpd_name` TEXT PRIMARY KEY NOT NULL,
  `cmpd_description` TEXT,
  `cmpd_host` TEXT NOT NULL,
  `cmpd_port` INTEGER,
  `cmpd_password` TEXT
);

INSERT INTO `c_service` (`cs_name`, `cs_description`, `cs_enabled`) VALUES ('service-mpd', 'Control a distant MPD', 1);
INSERT INTO `c_service_mpd` (`cmpd_name`, `cmpd_description`, `cmpd_host`, `cmpd_port`, `cmpd_password`) VALUES ('Local MPD', 'MPD Instance on localhost', 'localhost', 0, '');
