# S1_IOCP_UE_Project

Unreal Engine 5.8 C++ client for an external Windows IOCP game server.
Portfolio slice: server-authoritative Terra A-D/Q combat, Dusk minions,
potion loot/use, inventory and four quick-slots.

## Source-only repository

By user decision (2026-10-08), **Content is NOT uploaded**, including imported
assets AND project-created Blueprints, AnimBPs, montages, maps, inputs and widgets.
Source, Config, project plugin source, scripts, docs and S1.uproject are included.
Local assets are not deleted. Binaries/cache/logs/SaveGames/local credentials/DB
backups are excluded. GameServer and DB are separate, not submodules.

Cloning alone does NOT restore a playable project. Restore matching local Content
separately, preserving /Game paths. Importing packs alone does not recreate edited
S1 Blueprints/maps/widgets. This Git repository is not a complete asset backup.

## Content used locally (not redistributed)

| Content / local path | Role |
|---|---|
| Paragon Terra — /Game/ParagonTerra | Warrior, locomotion, A-D/Q, impact |
| Paragon Minions — /Game/ParagonMinions | Dusk melee minion |
| SwordTrailVFX — /Game/SwordTrailVFX/VFX/NS_Trail_14 | A-D/Q Niagara trail; user confirmed portfolio use |
| LootSet — /Game/LootSet/Meshes/Potions/potion_large02 | Potion drop mesh |
| DrapEffet — /Game/DrapEffet/VFX/NE_drop_effects01 | Drop effect |
| VFX_StatusEffect01 — /Game/VFX_StatusEffect01/FXS/FXS_healing000 | Healing effect |
| /Game/_Resources/Rair_Potion | Inventory/quick-slot icon |
| ARPG_Halberd — /Game/ARPG_Halberd | Imported candidate, not active equipped weapon |
| Elderboom Village — /Game/ElderboomVillage | Local village environment; source map retained, VillageMap is the gameplay copy |
| /Game/Blueprints, /Game/S1, /Game/Maps | S1 gameplay/animation/input/UI and Lobby/Loading/Main/Village |

Other imported folders include MedievalClassicUI, MaterialFunctionCollection,
Characters and LevelPrototyping; presence is not evidence of active usage.
See [asset registry](Docs/CurrentAssetRegistry.md) for exact dependencies,
active/candidate distinctions and remaining provenance/license notes. Listing
assets grants no redistribution rights. Acquire them through original sources
and licenses. UE5_RPG remains a read-only reference, not copied into this repo.

## Build / verification

UE5.8 Win64/C++20/BuildSettingsVersion.V7. Generate project files from S1.uproject,
then build S1Editor Win64 Development. Restore Content before gameplay/cooking.
Supplied Protobuf headers and required Win64 libprotobuf.lib (~16.3 MiB) are
included; the library is ordinary Git binary, not an LFS content asset.
S1NavigationTools source is included; rebuild its DLL/PDB. Engine plugins listed
in S1.uproject must exist in the matching engine installation.
Server canonical protocol, DB and navigation exports remain separate. Do not
manually edit generated protobuf outputs.

Previous inventory revision passed game/editor builds, 20 automated tests and
Windows packaging. User accepted PIE screen/controls; styling is deferred.
Git registration does not repeat testing or prove full restore, DB recovery or
packaged two-account acceptance.

Latest village revision (2026-10-21): initial/reconnected logins follow Lobby ->
Loading -> Village, using server region contract1. Deploy the matching external
server/protocol and trusted village exports separately; old client/server pairs
are rejected. MainMap combat remains preserved, but region-transfer portals and
last-location reconnect persistence are not implemented.

This revision passed game/editor builds, 40 automation tests (one existing
GameplayCue-path warning), and Windows Development packaging. User accepted the
two-client PIE village checklist, then packaged gameplay with a Main HUD omission;
the HUD-fix package explicitly cooks MainHUD/Minimap/Currency/QuestTracker/Journal,
and the user separately accepted its HUD/quest/dialogue hide-restore checks.
See [village implementation and evidence](Docs/VillageEntryContract_2026-10-21.md).
See [Git guide](Docs/GitManagement.md), [inventory](Docs/InventoryWidgetBlueprint.md)
and [recovery roadmap](Docs/RecoveryAndOperationsRoadmap_2026-10-07.md).
