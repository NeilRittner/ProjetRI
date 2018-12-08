--
-- Table structure for table `page`
--

CREATE TABLE `page` (
  `id_page` bigint(20) NOT NULL auto_increment,
  `url` text NOT NULL,
  `pr` float NOT NULL default '0',
  `resume` text NOT NULL,
  PRIMARY KEY  (`id_page`)
) TYPE=MyISAM;

--
-- Table structure for table `word`
--

CREATE TABLE `word` (
  `id_word` bigint(20) NOT NULL auto_increment,
  `word` varchar(30) NOT NULL default '',
  PRIMARY KEY  (`id_word`)
) TYPE=MyISAM;

--
-- Table structure for table `word_page`
--

CREATE TABLE `word_page` (
  `id_word` bigint(20) NOT NULL default '0',
  `id_page` bigint(20) NOT NULL default '0',
  PRIMARY KEY  (`id_page`,`id_word`)
) TYPE=MyISAM;
