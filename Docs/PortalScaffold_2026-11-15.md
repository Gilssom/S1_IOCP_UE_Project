# Portal authoring scaffold — 2026-11-15

User-selected Niagara `/Game/RPGEnvironmentVFX/VFX/Niagara/NS_TeleportZone` is a
NiagaraSystem; MCP dependency inspection found its MI_Mist/MI_Particles and engine
Niagara assets. Its original saved package was clean and SHA256 remained
5E40D18F94398A5D4EF4F3B0F4AC1F9F303A756CB0EF98B48D2F2E830F21FF16.
Portfolio-use provenance/license has not yet been provided.

Source: `Source/S1/Public/Game/S1Portal.h`, `Source/S1/Game/S1Portal.cpp`.
`AS1Portal` is an editor-placeable actor with root at floor, automatically active
Niagara component and query-only Pawn-overlap box. Box half extents start at
150/150/100cm and its center is 100cm above root. These are presentation/authoring
defaults, not a server-authoritative portal radius. Actor instance PortalId defaults
to zero (unassigned); no region or network contract is implied. No actor tick or
Unreal replication is added. No overlap event handler, fade, teleport, LoadingMap,
protocol or server state change is implemented in this stage.

Blueprint `/Game/S1/World/BP_S1Portal` is based on `/Script/S1.S1Portal`; its generated
class is `BP_S1Portal_C`. MCP compiled it with warnings-as-errors, inspected its CDO
components and Niagara system, query-only collision/Pawn overlap, box extent, and
saved only that newly created dirty Blueprint. No level, original Niagara or other
assets were saved. VillageMap was dirty before the user closed the editor; after
restart it was clean. Blueprint SHA256
943390F93521B50DC686E97793AF287A549F9F870FB9297E82EFFA4D8D6A6FC7, verified copy at
`C:/Users/user/Documents/Unreal IOCP Server/checkpoints/S1Portal_20261115/BP_S1Portal.uasset`.

S1 Game Win64 Development build succeeded (132.16 seconds), and S1Editor Win64
Development build succeeded (55.42 seconds), both using DisableAdaptiveUnity.
PIE, Niagara rendering/scale, actual overlap execution and packaging are not yet
verified. Map placement and its selective save are user-controlled. Future transfer
requires a separate approved server/packet lifecycle contract and testing; this
scaffold cannot teleport or be treated as server-validated movement.
