-- mod-secondary-class: per-player secondary class state (one row per player).
-- `secondary_class` stores AC CLASS_* constants (1=Warrior, 6=DeathKnight, 11=Druid).

CREATE TABLE IF NOT EXISTS `character_secondary_class` (
  `guid`            INT UNSIGNED NOT NULL,
  `secondary_class` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
