import unreal


SOURCE_MAP = "/Game/Maps/DevMap"
MAP_CONFIGS = (
    ("/Game/Maps/LobbyMap", unreal.S1LobbyGameMode),
    ("/Game/Maps/LoadingMap", unreal.S1LoadingGameMode),
    ("/Game/Maps/MainMap", unreal.S1MainGameMode),
)


def ensure_map(map_path):
    if unreal.EditorAssetLibrary.does_asset_exist(map_path):
        return unreal.EditorAssetLibrary.load_asset(map_path)

    world = unreal.EditorAssetLibrary.duplicate_asset(SOURCE_MAP, map_path)
    if not world:
        raise RuntimeError(f"Failed to create map: {map_path}")
    return world


for target_map, game_mode_class in MAP_CONFIGS:
    world = ensure_map(target_map)
    world_settings = world.get_world_settings()
    world_settings.set_editor_property("default_game_mode", game_mode_class)
    if not unreal.EditorAssetLibrary.save_loaded_asset(world, only_if_is_dirty=False):
        raise RuntimeError(f"Failed to save map: {target_map}")
    unreal.log(f"Configured {target_map} with {game_mode_class}")
