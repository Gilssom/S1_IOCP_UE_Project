# Source-only Git policy — 2026-10-08

User approved S1 Git registration/Private origin, then excluded actual assets.
Root E:\Git Fork\S1_UE5.8; origin https://github.com/Gilssom/S1_IOCP_UE_Project.
main continues remote initial commit 6cbc4c86a69637a184b05c9c9963174da6df1d64.
Author reused locally only: GilSsom / gil5299@naver.com. Global config unchanged.
GameServer HEAD a6aadb7210dab526a564850155ec97b22f524ff7 has local changes;
HEAD alone is not its complete current state. GameServer is untouched.

Include Source/Config/docs/scripts/uproject/AGENTS/project plugin source and
supplied Source/ProtobufCore/Lib/Win64/libprotobuf.lib. Ignore Content and all
Unreal packages, Saved, cache, generated binaries/solutions, .codex/.aws, private
keys and DB dumps. Ignore is not deletion. Only S1NavigationTools project plugin
was found; source exists and binaries rebuildable. No binary-only plugin found.
LFS installed locally with reserved package rules; baseline intended to have ZERO
LFS objects. Never force-add Content without a new scope/license/quota review.
Local Content: 5,282 files / ~7.92 GiB, excluded from remote.

Check status/diff before work; preserve user's existing changes. Commit by feature
with actual validation evidence. No automatic reset --hard, clean or force push.
Review staged names/content for secrets even in Private. Update asset README and
registry as dependencies change. Source/config Git is NOT a backup of edited
Blueprints/maps/montages/widgets/inputs, SaveGames, server or DB. Preserve local
assets and existing partial checkpoints; full backup/restore remains deferred.
No broad Save All. No UnrealEditor process observed during registration inspection;
live dirty packages could not be queried. AWS/DB/Agent changes remain unapproved.

Staging review: asset/build/cache/secret filename exclusions checked, no credential
literal detected in text scans. Three password-pattern matches were UI GetText
variable assignments without literals. This is a scoped review, not a guarantee
against every possible secret. Existing whitespace warnings retained, not reformatted.
Git/LFS installed and Private/push permissions independently checked through API.
No LFS objects are staged. Source includes the supplied protobuf testing headers
otherwise excluded by the inherited generic Testing rule. Commit/push verification
completed for baseline 4be9a045bc261e189529daa17713f8033b194654: push succeeded,
ls-remote main exactly matched local HEAD, status clean, 711 tracked files,
Content files 0 and LFS objects 0. Required lib blob size 17,131,848 bytes verified.
Existing initial commit preserved; no force push. No full clone/restore or new
build/PIE was performed for this configuration-only task. Completion documentation
is a separate subsequent commit; see git log for its exact ID.
