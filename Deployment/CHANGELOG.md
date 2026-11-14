# TECHM firmware changelog

Newest version first. Versions before v001.03 are not listed.

## v001.07 (2026-11-14)

- Task: VARS2 direct access part B (`UHA_COMMON/tasks/vars2_direct_access.md`), branch `feature/vars2_direct_access`
- CRC: `0x7BFCAFD1`

- Variables live in `Vars2` (value in physical units + valid flag); the old `Vars[]` / `VAR_*` API is no longer used here.
- ELHEATER, HEATING, TEMPCON and BOILER_AIR read their inputs directly and compute in degC / A / %. The
  thresholds switch at the same values as before (the old code used truncated whole units); same
  invalid-input handling (electric heater off when any of its 6 inputs is invalid).
- WM: the water meter interrupt only counts pulses; consumption and flow are published from the main loop.
- The start-up write of the battery voltage (52.0 V) is gone: until ELECON is received the value is invalid.
- Obsolete serial PC link (`SCOM`) removed.
- VARS2 sends this node's variables by the YAML `period_ms` (same 31 variables and periods as the removed
  `COM_AddStreamedVariable()` list).
- Flash -1.3 KB, RAM -2.3 KB.

## v001.06 (2026-11-13)

- Task: VARS2 step 6b part B (`UHA_COMMON/tasks/vars2_stop_legacy_tx.md`), branch `feature/vars2_stop_legacy_tx`
- CRC: `0x9C4988D8`

- Old variable frame `0x110+node` is no longer sent; only VARS2 `0x180+node` (shared `COM.c`, part A).
- `CMD_VAR_VALUE` RX case removed from `APP.c`. All remote variables read here (ELHEATER battery/MPPT values, TEMPCON room/outdoor temperatures) have a VARS2 row and are read through the read layer since v001.05.
- First TECHM build with the MCAN CAN TX supervision (dead-TX self-reset, ABOM, TEC/REC monitor) and the `NETWORK_STATUS` row in `VarMeta`.

## v001.05 (2026-11-08)

- Task: VARS2 read layer, part B (`UHA_COMMON/tasks/vars2_read_layer.md`), branch `feature/vars2_read_techm`
- CRC: `0x7F4C9243`

- `VAR_EnableVars2Read()` after `COM_Init()`: remote variables are read from VARS2: `BAT_VOLTAGE_V10`, `BAT_SOC`, `CHARGING_A10`, `BAT_CURRENT_A10`, `MPPT_SOLAR_VOLTAGE_V100` (ELHEATER, from ELECON) and `TEMP_DOWNSTAIRS` (IOBOARD_D), `TEMP_KIDROOM`, `TEMP_OUTSIDE` (ELECON) in TEMPCON.
- Behaviour after a producer timeout (3 s for battery/MPPT values, 9 s for temperatures): the value is flagged invalid, ELHEATER goes to `eElh_InvalidInputs` and switches the heater off immediately, TEMPCON switches the heating pumps off. Before, the last value was used indefinitely.
- Own variables (tank, boiler, board temperatures) are still read from `Vars[]`; the local initial `BAT_VOLTAGE_V10 = 520` (APP.c) is kept until the first VARS2 frame arrives.
- `SCOM.c` is not used on this node and was not changed.

## v001.04 (2026-11-08)

- Task: VARS2 step 2 (`UHA_COMMON/tasks/elecon_vars2_tunel.md`), branch `feature/elecon_vars2_tunel`
- CRC: `0x89853F65`

- No functional change of the node, shared VARS2 code updated (frame hook for ELECON, unused here).
- VARS2 fix: a node's own value no longer reports a timeout in `VARS2_IsValid()` (own state refreshed when the frame is sent).
- VARS2 fix: `0x18x` frames with DLC != 8 or RTR are dropped in `COM_GetRxMessage()`.

## v001.03 (2026-11-01)

- Task: VARS2 step 1 (`UHA_COMMON/tasks/vars2_dual_transmission.md`), branch `vars2-step1`
- CRC: `0x1C59800E`

- Sends all variables in the old format (`0x110+node`) and the new VARS2 format (`0x180+node`).
- Received VARS2 frames are stored in `Vars2`; control logic still uses the old variables.
