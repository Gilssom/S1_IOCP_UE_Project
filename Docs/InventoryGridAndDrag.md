# Inventory grid / quick-slot drag UI stage — 2026-10-12

## Follow-up: quick-slot mouse routing correction — 2026-10-12

User PIE reported right-click unassignment did not work with inventory open.
Read-only MCP confirmed WBP_S1QuickSlots RootCanvas was HitTestInvisible, which
disables hit testing for its entire subtree even when the outer UserWidget uses
SelfHitTestInvisible. Changed ONLY RootCanvas.visibility to SelfHitTestInvisible;
layout, native code, server, DB and other assets unchanged. This also restores the
input path needed by drag/drop. Backup before mutation: checkpoint
InventoryGrid_20261012/QuickSlotInputFix/WBP_S1QuickSlots_before.uasset, SHA256
A71477C0337150334636454D230A697044D7D02BC78C52A682DC2D4BEC9D9E2E matched source.
Affected Blueprint compiled with warnings_as_errors=true without error; before
dirty=false, after change=true reported, selected save=true, final dirty=false,
property readback=SelfHitTestInvisible. No new C++ build required or executed.
Previous builds/tests/package predate this asset correction; package not refreshed.
User subsequently confirmed PIE right-click unassignment testing complete and
explicitly confirmed drag registration works normally (2026-10-12). These are
user-reported rendered checks, not agent-executed tests; client count and all four
destinations were not individually specified. Updated packaged-client validation
and remaining UI acceptance items are still pending. The original failed test
remains recorded above; the confirmation applies to the corrected asset.

## Approved scope and deferred work

Follow-up: user approved empty-slot relocation implementation; see
InventorySlotMoveImplementation.md for current progress. Earlier movement-deferred
statements below describe the initial UI-only stage. Swap/merge/split/discard and
real80-slot capacity remain deferred.

User approved 8 columns x10rows (80 visible cells), drag inventory potion to four
quick-slots, instant right-click unassignment. Registration/clear/use buttons are
absent in the new window; double-click uses existing RequestUseItem. Permanent
discard was chosen for a LATER stage (future ground-drop extension), not implemented
here. Server slot movement/exchange, stack merge/split, discard/count/Yes-No modal,
and real80-slot capacity are deferred; no DB/protobuf/server changes this stage.
Server and client authoritative inventory checks remain20slots. Cells21-80 are
dimmed/locked and the window explicitly announces this limit. Do not claim this is
an80-slot persistent inventory. Dropping between cells/outside/drag cancellation
changes nothing; no optimistic stock changes.

## Active assets / editing

/Game/S1/UI/WBP_S1InventoryGrid (S1InventoryWidget) is the new active window.
/Game/S1/UI/WBP_S1InventorySlot (S1InventorySlotWidget) is the cell template.
Old WBP_S1Inventory / WBP_S1InventoryStackRow retained for legacy regression,
not deleted or modified. WBP_S1QuickSlots keeps the user's original layout/binary;
native mouse event handlers use each Icon1-4's separate immediate parent geometry
as its hit region. Preserve separate per-slot containers and those12bindings.

New window: GridRoot/InventoryPanel/InventoryBackground/Contents and InventoryGrid
(UniformGridPanel). Initial420width, centered, cells42square/padding2. Runtime80
cells Row=slot/8,Column=slot%8; server slot_index determines occupancy, no dense-list
reordering. GridSlotClass points to the cell WBP. Cell must retain ItemIcon(Image),
CountText/SlotText(TextBlock); root size/background/font are designer-owned.
Main StatusLabel/QuantityLabel/SelectionLabel and CloseButton retain their roles.
Legacy main BindWidget fields are optional for compatibility, not displayed controls.
MCP GetWidgets includes optional inherited placeholders as widget=None; those are
not actual hidden buttons. Actual main tree13widgets plus12unbound placeholders.

I opens -> one click selects -> double-click consumes1 through existing server
contract -> drag to quick-slot registers item DEFINITION, not a stock transfer.
Right-click quick-slot removes assignment immediately/no confirmation. Use I to
show cursor, or existing Alt cursor mode outside inventory. Ordinary gameplay keeps
HUD HitTestInvisible; cursor UI uses SelfHitTestInvisible so mouse interaction
does not make the entire viewport an opaque HUD input target. I/Esc/close return
game focus. Source slot consumes mouse input; does not send combat actions.

Unknown request recovery: when normal double-click is blocked, explicit R while
inventory focused calls existing same-request recheck (can pass0 if vanished stack;
coordinator keeps original request/item). No new request or implicit resend.
Busy/unknown blocks drag/use. Native drag payload has exact ItemId + weak source
window; coordinator checks current window, valid current server entry, supported
potion definition20001, positive count, slot0-3, common pending gates. Closed/foreign
window or deleted stack cannot authorize registration. Clearing local assignment
does not cancel an outstanding consume or change its item/request.

## Source / checkpoint

Modified UI/S1InventoryWidget.h/.cpp, S1QuickSlotWidget.h/.cpp,
Loot/S1InventorySubsystem.h/.cpp, UI/S1InventoryWidgetTests.cpp.
New UI/S1InventoryDragOperation.h, S1InventorySlotWidget.h/.cpp,
S1InventoryGridTests.cpp. Exact hard native reference makes the grid cookable and
grid CDO references slot class. ToggleInventory can load exact grid class after
in-session asset authoring if constructor reference was initially unavailable.
No changes to player/combat/HP pending edits or GameServer; all preserved.

Checkpoint: C:\Users\user\Documents\Unreal IOCP Server\checkpoints\InventoryGrid_20261012.
Original7sources/old inventory+quick-slot binaries copied/hash verified. New WBP
initial/final snapshots copied/hash verified. Native constructor lookup created
an empty grid package before file existed; MCP CreateWidgetBlueprint rejects any
FindPackage. After confirming no disk asset/no existing WidgetBlueprint, created
GridLayout at temporary path, saved only new pair and checkpointed, then used
supported asset move into final Grid path. Final compile/warnings-as-errors passed;
new pair dirty reported and selected-save -> clean. No engine modification or
broad redirector fixup/save-all. Original inventory/quick-slot stayed clean.

## Executed verification / remaining gates

S1Editor and S1 Development full build/link exit0. Actual new WBP parent/class,
GridSlotClass and selective-save verified via serial MCP. UnrealAutomation.log +
Saved/Automation/InventoryGrid_20261012/index.json:22Success/failed0/notRun0
(21plain,1existing Q GameplayCue fallback warning). New GridAndDrag error0/warning0.
Tests use real saved WBP and actual slot native double-click/drag-detected handlers:
80cells/8x10, sparse slot19 mapping, future-slot locks, no legacy buttons,
quantity/font/stable cells, typed source+ID, busy/empty/locked prevention, unknown
R recheck, four coordinator registrations, foreign/closed/missing item rejection,
inventory count invariant and immediate local unassignment. Existing legacy
inventory and quick-slot tests retained (coordinator ref changed to new grid).
NullRHI/delegate calls do NOT validate physical drop hit geometry or renderer.

Latest Windows Build/Cook/Stage/Package/Archive passed (Package.log BUILD SUCCESSFUL,
AutomationTool exit0), output Saved/InventoryGridPackage_20261012/Windows. Cooked
registry includes grid/slot/quick-slot and three maps. Actual archived S1.exe
PackageGridSmoke.log loaded grid/slot generated classes1object each, Quit/exit0.
Existing Auth InputMode:UIOnly Non-Focusable error1 persists; not a new grid error.
Package smoke is headless class load, NOT rendered drop or authenticated play.
User confirmed corrected PIE drag registration and right-click unassignment.
Other rendered acceptance items not explicitly confirmed: I/grid,
all4destinations/replacement/empty slot,
double-click use/HP/quantity/FX, no combat input, focus close/numeric use restoration,
cursor-only right click outside inventory, persistence and account separation,
busy/stale cancelled drag. Existing Auth focus error is unrelated and not fixed.
No Git commit/push. Binaries remain excluded from source-only Git.
