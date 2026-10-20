# MaruBuri multi-weight UI Font — 2026-10-16

## Cause and result

User requested a fix after read-only diagnosis: all five weight assets existed in
EngineFonts as FontFace, but the only Font assigned to four S1 WBPs had a single
Default entry referencing Light. Importing a FontFace does not itself add a style
to the Font currently used by TextBlock.

New project-owned assets under `/Game/S1/UI/Fonts/`:

| Asset | Class / purpose |
| --- | --- |
| MaruBuri_Font | Runtime Font, composite default family |
| MaruBuri_ExtraLight | FontFace |
| MaruBuri_Light | FontFace |
| MaruBuri_Regular | FontFace |
| MaruBuri_SemiBold | FontFace |
| MaruBuri_Bold | FontFace |

These are Unreal asset duplicates of the user's imported EngineFonts assets, not
UE5_RPG content or engine-source edits. No new font download. User's local
Downloads/maruburi.zip also contains all five OTF/TTF weights. Public distribution
license/provenance verification was not performed in this technical fix; preserve
applicable license notices when distributing the font. Content is excluded from
source-only Git, so the checkpoint is the binary recovery source.

Font default family has named entries ExtraLight, Light, Regular, SemiBold, Bold,
each linked to the matching **project-owned** Face. Additional Default aliases
Light to preserve current user styling. No engine FontFace dependency remains in
MaruBuri_Font. No fallback/subfamily changes or simulated bold used.

## Editing

In Widget Blueprint Designer select a Text control, then Details → Appearance →
Font: choose `/Game/S1/UI/Fonts/MaruBuri_Font`, then select Typeface/Typeface Font
Name (style) ExtraLight, Light, Regular, SemiBold or Bold. Default remains Light.
Changing point size alone does not change weight. No C++ override forces Light.

Existing Light Font references replaced in exactly 12 TextBlock fontObject fields:

- WBP_S1Dialogue: QuestTitleText, QuestDetailText, NpcNameText, DialogueText,
  AcceptLabel, DeclineLabel (6).
- WBP_S1NpcInteractPrompt: KeyText, InteractionText (2).
- WBP_S1NpcNameplate: NameText (1).
- WBP_S1InventoryDiscardDialog: MessageText, ConfirmText, CancelText (3).

Only fontObject changed. Existing Default style, size (including fractional
points), font material, outline and spacing settings preserved by round-trip
property comparison. Color/layout/control structure untouched. Other fonts (e.g.
SpinBox amount input) intentionally not replaced. No global UI redesign.

## Workflow / evidence

- Read-only MCP identified Font/FontFace classes, old dependency and four referencers.
- Backed up clean WBPs before changes in external checkpoint
  `C:/Users/user/Documents/Unreal IOCP Server/checkpoints/MaruBuriFont_20261016/ContentBefore`.
- Used serial MCP asset duplication and actual Font editor Slate controls to
  configure styles; every face/name verified against observed UI text. Font
  composite Typeface array is not exposed by the generic MCP property schema, so
  it was not modified via invented API calls or engine code.
- Reported six new dirty font packages before selective save. Then reported four
  dirty WBPs before saving only those four. NPC BP/MainMap/original engine Font
  remained clean. ContentAfter preserves all ten saved packages (about 5.3 MB fonts).
- Four affected WBPs compiled warnings-as-errors with zero warnings/errors.
- Asset Registry: new Font depends on exactly five project Faces; old engine Light
  Font has no remaining referencers returned; new Font has the four intended WBPs.
- Current run `Saved/Automation/MaruBuriFont_20261016/index.json`: 29 tests,
  28 clean success +1 success with pre-existing GameplayCueNotifyPaths warning,
  zero failures/not-run. Exit0. This verifies existing UI/data construction, not
  visual comparison of all font weights.
- No source/native changes, no C++ build needed or claimed. No server/DB/config/map
  modifications. No engine save. No new package/cook or authenticated PIE visual
  test run; Korean glyph appearance/readability at each chosen weight and final
  packaged rendering remain manual validation items. No content-rule deviation.

Documents updated: this record, NpcNameplateWidget.md and CurrentAssetRegistry.md.
