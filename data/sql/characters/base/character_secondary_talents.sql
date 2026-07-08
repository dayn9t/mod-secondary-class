-- mod-secondary-class (Phase 3): per-character secondary-class talents.
-- One row per learned talent (talent_id from Talent.dbc col0, talent_rank 1..5).
-- NOTE: column is `talent_rank`, not `rank` -- "rank" is a MySQL 8 reserved word
-- (window functions) and AC aborts the core on the resulting SQL syntax error.
CREATE TABLE IF NOT EXISTS `character_secondary_talents` (
  `guid` int unsigned NOT NULL,
  `talent_id` int unsigned NOT NULL,
  `talent_rank` tinyint unsigned NOT NULL,
  PRIMARY KEY (`guid`,`talent_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
