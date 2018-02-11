-- SQlite3 init script

DROP TABLE IF EXISTS `t_stream_element`;
DROP TABLE IF EXISTS `t_stream`;
DROP TABLE IF EXISTS `t_category_info`;
DROP TABLE IF EXISTS `t_config`;
DROP TABLE IF EXISTS `t_media_history`;
DROP TABLE IF EXISTS `t_playlist_element`;
DROP TABLE IF EXISTS `t_playlist`;
DROP TABLE IF EXISTS `t_meta_data`;
DROP TABLE IF EXISTS `t_media`;
DROP TABLE IF EXISTS `t_folder`;
DROP TABLE IF EXISTS `t_image_cover`;
DROP TABLE IF EXISTS `t_data_source`;

CREATE TABLE `t_data_source` (
  `tds_id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `tds_username` TEXT,
  `tds_name` TEXT NOT NULL,
  `tds_description` TEXT,
  `tds_path` TEXT NOT NULL,
  `tds_icon` TEXT,
  `tds_last_updated` TIMESTAMP NULL DEFAULT NULL,
  `tds_refresh_status` TINYINT DEFAULT 0 -- 0: not running, 1 pending, 2: preparing, 3: runnin, 4: cleaning, 5: error, 6: stop, 7: not found
);
CREATE INDEX `i_tds_name` ON `t_data_source`(`tds_name`);

CREATE TABLE `t_image_cover` (
  `tic_id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `tds_id` INTEGER,
  `tic_cover_original` MEDIUMBLOB,
  `tic_cover_thumbnail` MEDIUMBLOB,
  `tic_fingerprint` TEXT,
  FOREIGN KEY(`tds_id`) REFERENCES `t_data_source`(`tds_id`) ON DELETE CASCADE
);
CREATE INDEX `i_tic_fingerprint` ON `t_image_cover`(`tic_fingerprint`);

CREATE TABLE `t_folder` (
  `tf_id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `tds_id` INTEGER NOT NULL,
  `tf_name` TEXT NOT NULL,
  `tf_path` TEXT NOT NULL,
  `tf_parent_id` INTEGER,
  `tf_refresh_status` INTEGER DEFAULT 0, -- 0 processed, 1 in refresh
  `tic_id` INTEGER,
  FOREIGN KEY(`tds_id`) REFERENCES `t_data_source`(`tds_id`) ON DELETE CASCADE,
  FOREIGN KEY(`tf_parent_id`) REFERENCES `t_folder`(`tf_id`) ON DELETE CASCADE,
  FOREIGN KEY(`tic_id`) REFERENCES `t_image_cover`(`tic_id`) ON DELETE SET NULL
);
CREATE INDEX `i_tf_name` ON `t_folder`(`tf_name`);

CREATE TABLE `t_media` (
  `tm_id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `tds_id` INTEGER NOT NULL,
  `tm_name` TEXT NOT NULL,
  `tm_path` TEXT NOT NULL,
  `tm_duration` INTEGER DEFAULT 0,
  `tf_id` INTEGER,
  `tm_type` TEXT,
  `tm_last_updated` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  `tm_refresh_status` INTEGER DEFAULT 0, -- 0 processed, 1 in refresh
  `tic_id` INTEGER,
  FOREIGN KEY(`tds_id`) REFERENCES `t_data_source`(`tds_id`) ON DELETE CASCADE,
  FOREIGN KEY(`tf_id`) REFERENCES `t_folder`(`tf_id`) ON DELETE CASCADE,
  FOREIGN KEY(`tic_id`) REFERENCES `t_image_cover`(`tic_id`) ON DELETE SET NULL
);
CREATE INDEX `i_tm_name` ON `t_media`(`tm_name`);
CREATE INDEX `i_tm_type` ON `t_media`(`tm_type`);

CREATE TABLE `t_meta_data` (
  `tmd_id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `tm_id` INTEGER NOT NULL,
  `tmd_key` TEXT NOT NULL,
  `tmd_value` TEXT,
  `tmd_internal` INTEGER DEFAULT 0, -- 0 tag in taliesin only, 1 tag inside file
  FOREIGN KEY(`tm_id`) REFERENCES `t_media`(`tm_id`) ON DELETE CASCADE
);
CREATE INDEX `i_tmd_key` ON `t_meta_data`(`tmd_key`);
CREATE INDEX `i_tmd_value` ON `t_meta_data`(`tmd_value`);

CREATE TABLE `t_playlist` (
  `tpl_id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `tpl_username` TEXT,
  `tpl_name` TEXT NOT NULL,
  `tpl_description` TEXT,
  `tic_id` INTEGER,
  FOREIGN KEY(`tic_id`) REFERENCES `t_image_cover`(`tic_id`) ON DELETE SET NULL
);
CREATE INDEX `i_tpl_name` ON `t_playlist`(`tpl_name`);

CREATE TABLE `t_playlist_element` (
  `tpe_id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `tpl_id` INTEGER NOT NULL,
  `tm_id` INTEGER NOT NULL,
  FOREIGN KEY(`tpl_id`) REFERENCES `t_playlist`(`tpl_id`) ON DELETE CASCADE,
  FOREIGN KEY(`tm_id`) REFERENCES `t_media`(`tm_id`) ON DELETE CASCADE
);

CREATE TABLE `t_media_history` (
  `tmh_id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `tmh_datestamp` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  `tmh_stream_name` TEXT,
  `tm_id` INTEGER NOT NULL,
  `tpl_id` INTEGER,
  FOREIGN KEY(`tpl_id`) REFERENCES `t_playlist`(`tpl_id`) ON DELETE SET NULL,
  FOREIGN KEY(`tm_id`) REFERENCES `t_media`(`tm_id`) ON DELETE CASCADE
);

CREATE TABLE `t_config` (
  `tc_id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `tc_type` TEXT,
  `tc_value` TEXT,
  `tc_order` INTEGER DEFAULT 0
);

CREATE TABLE `t_category_info` (
  `tci_id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `tds_id` INTEGER NOT NULL,
  `tci_level` TEXT NOT NULL, -- supported at this time: artist, album, year, genre
  `tci_category` TEXT,
  `tci_content` MEDIUMBLOB,
  `tic_id` INTEGER,
  FOREIGN KEY(`tds_id`) REFERENCES `t_data_source`(`tds_id`) ON DELETE CASCADE,
  FOREIGN KEY(`tic_id`) REFERENCES `t_image_cover`(`tic_id`) ON DELETE SET NULL
);

CREATE TABLE `t_stream` (
  `ts_id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `ts_username` TEXT,
  `ts_name` TEXT NOT NULL,
  `ts_display_name` TEXT,
  `tpl_id` INTEGER,
  `ts_index` INTEGER DEFAULT 0,
  `ts_webradio` INTEGER,
  `ts_random` INTEGER DEFAULT 0,
  `ts_format` TEXT,
  `ts_channels` INTEGER,
  `ts_sample_rate` INTEGER,
  `ts_bitrate` INTEGER,
  FOREIGN KEY(`tpl_id`) REFERENCES `t_playlist`(`tpl_id`) ON DELETE SET NULL
);
CREATE INDEX `i_ts_name` ON `t_stream`(`ts_name`);

CREATE TABLE `t_stream_element` (
  `tse_id` INTEGER PRIMARY KEY AUTOINCREMENT,
  `ts_id` INTEGER NOT NULL,
  `tm_id` INTEGER NOT NULL,
  FOREIGN KEY(`ts_id`) REFERENCES `t_stream`(`ts_id`) ON DELETE CASCADE,
  FOREIGN KEY(`tm_id`) REFERENCES `t_media`(`tm_id`) ON DELETE CASCADE
);

INSERT INTO `t_config` (`tc_type`, `tc_value`) VALUES ('video_file_extension', '.avi'), ('video_file_extension', '.mpg'), ('video_file_extension', '.mpeg'), ('video_file_extension', '.mp4'), ('video_file_extension', '.m4v'), ('video_file_extension', '.mov'), ('video_file_extension', '.wmv'), ('video_file_extension', '.ogv'), ('video_file_extension', '.divx'), ('video_file_extension', '.m2ts'), ('video_file_extension', '.mkv');
INSERT INTO `t_config` (`tc_type`, `tc_value`) VALUES ('subtitle_file_extension', '.srt'), ('subtitle_file_extension', '.ssa'), ('subtitle_file_extension', '.vtt'), ('subtitle_file_extension', '.sub'), ('subtitle_file_extension', '.sbv');
INSERT INTO `t_config` (`tc_type`, `tc_value`) VALUES ('audio_file_extension', '.mp3'), ('audio_file_extension', '.m4a'), ('audio_file_extension', '.aac'), ('audio_file_extension', '.ogg'), ('audio_file_extension', '.oga'), ('audio_file_extension', '.flac'), ('audio_file_extension', '.wav'), ('audio_file_extension', '.wma'), ('audio_file_extension', '.aif'), ('audio_file_extension', '.aiff'), ('audio_file_extension', '.ape'), ('audio_file_extension', '.mpc'), ('audio_file_extension', '.shn'), ('audio_file_extension', '.au');
INSERT INTO `t_config` (`tc_type`, `tc_value`) VALUES ('image_file_extension', '.jpg'), ('image_file_extension', '.jpeg'), ('image_file_extension', '.png'), ('image_file_extension', '.gif'), ('image_file_extension', '.bmp');
INSERT INTO `t_config` (`tc_type`, `tc_value`, `tc_order`) VALUES ('cover_file_pattern', 'folder.jpg','0'), ('cover_file_pattern', 'folder.jpeg','1'), ('cover_file_pattern', 'cover.jpg','2'), ('cover_file_pattern', 'cover.jpeg','3'), ('cover_file_pattern', 'front.jpg','4'), ('cover_file_pattern', 'front.jpeg','5'), ('cover_file_pattern', '*.jpg','6'), ('cover_file_pattern', '*.jpeg','7'), ('cover_file_pattern', '*.gif','8'), ('cover_file_pattern', '*.png','9');
