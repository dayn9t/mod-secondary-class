-- mod-secondary-class: spells granted by the secondary class, per player.
-- Used by .secondary unset (precise removal — never touches primary/quest/item spells)
-- and OnLogin (verify presence, re-learn any lost on worldserver restart).

CREATE TABLE IF NOT EXISTS `character_secondary_class_spells` (
  `guid`      INT UNSIGNED NOT NULL,
  `spell_id`  MEDIUMINT UNSIGNED NOT NULL,
  PRIMARY KEY (`guid`, `spell_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
