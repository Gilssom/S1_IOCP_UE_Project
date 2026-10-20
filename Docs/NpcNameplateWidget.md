# NPC overhead name Widget Blueprint — 2026-10-16

## Scope and editing

User-approved passive overhead NPC name, separate from the `[G] 대화` prompt and
the modal dialogue UI. No server, DB, packets, quest acceptance, player/monster
HP policy, NPC mesh, or animation changes. Names are local NPC presentation data,
not a new server entity identity.

- `/Game/S1/UI/WBP_S1NpcNameplate`: parent `US1NpcNameplateWidget`.
- `/Game/S1/Quest/BP_S1QuestNpc`: inherited `NpcNameplate` component points to the WBP.
- Name content: NPC `DisplayName` (same source as dialogue header). Set per BP default
  or placed actor. For runtime changes, call `RefreshNameplate` after setting it.
- Styling: WBP Designer `NameplateSize → NameplateCanvas → NameText`.
  Keep the bound TextBlock name **NameText**; it is required. The Designer's sample
  text is replaced with the NPC name at runtime. No Blueprint Tick/binding needed.
- Height: BP Components → NpcNameplate → Relative Location Z **250 cm**, relative to
  NPC root/feet, not the animated head. Tune for replacement models manually.
- Screen draw size **300×36**, pivot (0.5,1), Screen space, WindowFocusable=false.
  If resizing, match WBP NameplateSize and Canvas text dimensions.
- Initial style: centered gold 18 pt text with dark shadow; no text wrapping.
  Canvas allows free positioning; no generated art was imported.
- Distance policy (2026-10-16 follow-up): show within **1500 cm**, hide beyond
  **1800 cm**, retain previous state in between. Adjust component `ShowDistance` /
  `HideDistance` in BP (cm). Invalid HideDistance below ShowDistance is clamped to
  ShowDistance at runtime. No combat/party/occlusion/dialogue-only policy added.
  Screen-space widgets do not provide world-wall occlusion.
- Passive HitTestInvisible/nonfocusable UI; no interaction events or authority.

## Implementation

`S1QuestNpc.h/.cpp` creates an independent component and supplies DisplayName in
OnConstruction/BeginPlay and explicit RefreshNameplate. It does not replace the
existing DialoguePrompt class override. `S1NpcNameplateWidget.h/.cpp` caches name
data in the component, refreshes on InitWidget/recreation, and passes it to the
native-base WBP. Widget code only sets text, never restyles Designer controls.
NPC actor Tick every 0.1 s supplies its world's local first controller pawn;
3D actor-root distance, not camera distance or animated head position. No valid
local pawn hides the name and clears hysteresis. Independent client worlds have
independent visibility; split-screen multiple local views are not implemented.
Actor Tick remains active when the widget is hidden, allowing reapproach to show
it again; the visibility logic does not depend on hidden UWidgetComponent ticks.

`S1NpcNameplateTests.cpp` checks actual NPC/WBP load, data delivery, explicit update,
recreation, screen space and no focus/collision/input capture.

## Checkpoint and verification

External checkpoint:
`C:/Users/user/Documents/Unreal IOCP Server/checkpoints/NpcNameplate_20261016`.
Pre-change NPC BP SHA256:
`C14C8CC656070A76F8F459219F4CC32A2131995D8D7225EDBDA69B531BC734C9`.
SourceBefore and ContentBefore preserve current files; Git excludes binary Content.

- S1Editor and S1 Win64 Development builds succeeded (exit 0); logs in checkpoint.
- New WBP and NPC BP compiled with warnings-as-errors; no new warnings/errors.
  Parent /Script/S1.S1NpcNameplateWidget and generated WBP_S1NpcNameplate_C verified.
- Reported both dirty packages before selectively saving only those two.
  MainMap, Dialogue WBP and G prompt WBP were clean and not saved. All five clean
  again after verification (assets reloaded after PIE garbage collection).
- NPC BP dependency includes new WBP, providing MainMap → NPC BP → name WBP cook
  reachability. A fresh cook/package has **not** been executed.
- Saved MainMap reload and short Simulate PIE verified placed NPC component uses
  new WBP, Screen/visible/nonfocusable. Session stopped; name WBP Designer opened.
  This was not authenticated multiplayer or manual visual acceptance.
- Automation report Saved/Automation/NpcNameplate_20261016/index.json: 29 tests,
  28 clean success + 1 success with pre-existing GameplayCueNotifyPaths warning,
  zero failures/not-run. NpcNameplate and DialogueStageA clean success. Name tests
  instantiate actual saved BP/WBP, assert displayed text before/after explicit
  refresh and widget recreation, and verify passive input settings.
- Current editor log scan found no Error/Fatal/Ensure/Blueprint Runtime Error.
- ContentAfter preserves both saved packages: BP SHA256
  F86DBF2D14F616D73217939A4B241C5E6F06ECBC464000A3288559F15C2845C3;
  name WBP SHA256 BEFDCFC60291E3E5E647B177FBB043189C7CDF672405A95B9D81C172F643D37E.

Current-version manual 1-/2-client nameplate readability/height/G prompt overlap,
dialogue framing, and packaged runtime are **not yet verified**. No content-rule
deviation or server-authority change. Distance was added in the follow-up below;
occlusion remains separate. Docs updated: this record and CurrentAssetRegistry.md.

## Distance follow-up / font investigation

The preceding build/29-test/SIE evidence describes the initial nameplate revision,
not automatically this follow-up. New evidence will be recorded separately.
Distance change touches S1QuestNpc.h/.cpp, S1NpcNameplateWidget.h/.cpp and
S1NpcNameplateTests.cpp only, plus documentation; no BP/Font/map saves intended.
Source checkpoint: checkpoints/NpcNameDistance_20261016/SourceBefore.

Read-only MCP verified the user's current NameText font points to
`/Engine/EngineFonts/MARUBURI-LIGHT_Font`, style name `Default`. All five imported
weights (ExtraLight/Light/Regular/SemiBold/Bold) exist in `/Engine/EngineFonts`,
but only MARUBURI-LIGHT_Font is a **Font**; the five weight assets are **FontFace**.
The Font's dependency is only MARUBURI-LIGHT. Thus other faces exist but are not
connected as selectable styles in the currently assigned Font. This is not a
missing weight in the downloaded archive: local Downloads/maruburi.zip contains
five OTF and five TTF weights. No engine/font asset was changed.

Recommended separate fix: import faces into a project-owned /Game UI/Fonts folder,
create a runtime Font with a DefaultTypeface entry for each weight and matching
FontFace reference, then choose its style in each relevant TextBlock. Do not edit
engine content or globally replace user-designed widgets without approval.

Later user-approved fix implemented: NameText now uses
`/Game/S1/UI/Fonts/MaruBuri_Font`, with five real weight styles and Default=Light.
Engine original untouched; existing nameplate style/geometry preserved. See
MaruBuriFontIntegration.md for the four affected WBPs and separate validation.

Distance follow-up verification: S1Editor/S1 Win64 Development builds exit0.
Saved/Automation/NpcNameDistance_20261016/index.json: 29 tests (28clean +1 existing
GameplayCueNotifyPaths warning),0failures/not-run. NpcNameplate and DialogueStageA
passed without warnings. Tests cover 15m entry, 18m exit, hysteresis both directions,
reapproach after hiding, null viewer reset and name preservation/recreation.
Restarted editor MCP query confirmed BP component showDistance1500/hideDistance1800
and existing name WBP class link. BP and WBP dirty=false; no binary saves made.
Current editor startup scan found no Error/Fatal/Ensure/Blueprint Runtime Error.
Manual authenticated 1-/2-client distance/visual acceptance and new packaging are
pending; no claim of current follow-up PIE visual success. Server/DB untouched.
