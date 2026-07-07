# mod-secondary-class

AzerothCore WotLK 3.3.5a module: **secondary class (dual-class) system**.

A player keeps their primary class (attributes, talents, gear, resource bar) and
may additionally take a **secondary class** whose **spells** are granted, synced
by primary level, persisted, and restored on login. Stage 1 = spells only
(talents / gear / stat mixing are out of scope; see design doc).

## Design

Full spec: `docs/superpowers/specs/2026-07-07-dual-class-design.md` (in the
parent `wow` repo).

Source-map (functional core / imperative shell):

| file | role |
|------|------|
| `src/SC_spell_resolver.{h,cpp}` | **core** — pure: class+level → spell set (queries `SkillLineAbility` DBC) |
| `src/SC_class_compat.{h,cpp}` | **core** — pure: primary+secondary resource-type compatibility |
| `src/SC_store.{h,cpp}` | **shell** — DB read/write of the two tables |
| `src/SC_player_hooks.{h,cpp}` | **shell** — `PlayerScript` OnLogin / OnPlayerLevelChanged |
| `src/SC_commands.{h,cpp}` | **shell** — `.secondary set/unset/show` |
| `src/SC_entry.cpp` | registers hooks + command |
| `src/SC_loader.{h,cpp}` | `AC_ADD_SCRIPT_LOADER` entry |

## Config

`conf/mod_secondary_class.conf.dist` → copied to `server/etc/modules/mod_secondary_class.conf`.

## Commands (GM)

```
.secondary set <classname>   # warrior/paladin/hunter/rogue/priest/dk/shaman/mage/warlock/druid
.secondary unset
.secondary show
```
