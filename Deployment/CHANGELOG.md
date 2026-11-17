# TECHM firmware changelog

Newest version first. Versions before v001.03 are not listed.

## v002.02 (2026-11-17)

- Task: commands step 3b (`UHA_COMMON/tasks/cmd_lights_blinds_setpoints.md`, spec `UHA_COMMON/VarDefs/CMD_spec.md`),
  branch `feature/cmd-3b` (UHA_COMMON + ASW_IOBOARD_D + ASW_IOBOARD_U + ASW_ELECON_D + ASW_TECHM)
- CRC: `0x0D78F4C9`

- Commands on CAN `0x203` (`APP_OnCommand()`): `CMD_UP_REQ_DAY` / `CMD_UP_REQ_NIGHT` / `CMD_DOWN_REQ_DAY` /
  `CMD_DOWN_REQ_NIGHT` (301-304, 5..25 °C) and `CMD_EXHAUST_SET` (305, 150..250 °C).
- Setters `TC_SetUpReqTempDay()` ... `TC_SetDownReqTempNight()` (TEMPCON) and `BAC_SetControlTemp()`
  (BOILER_AIR): store the value clamped to `CMD_*_MIN/_MAX`, nothing else. Room setpoints are used from the next
  `TC_Update_1s()`; the exhaust setpoint when `es_AirControl` is entered next time (a running air control keeps
  its setpoint). RAM only, defaults after reset. No change of any control decision.
- Flash +276 B, RAM +0 B (since v002.01).

## v002.01 (2026-11-16)

- Task: VARS2 states step 2c (`ASW_TECHM/tasks/techm_states_setpoints.md`), branch `feature/vars2-states-techm`
  (UHA_COMMON + ASW_TECHM); review list `ASW_TECHM/tasks/techm_states_report.md`
- CRC: `0xFE528C60`

- New VARS2 group `States.Techm` (ids 440–464): setpoints, room requests in effect, circuits, pumps, heater coils,
  boiler state (`send: on_change`) and valve positions / required water temperatures (periodic 5 s, logged).
- `EL_HEATER_STATUS` (id 80) is sent on change + every 20 s (was every 3 s); id, value and meaning unchanged.
- **No control decision changed.** `DO.c` remembers the last value given to each setter (read-only getters);
  `TEMPCON.c` copies the day/night request in effect into two globals; `BOILER_AIR.c` uses `mBacControlTemp_C`
  (= `BAC_DEF_CONTROL_TEMP`, 200) instead of the define; `APP_UpdateStates_1s()` (called after all 1 s control
  tasks) only writes `Vars2`.
- Shared code: VARS2 `send: on_change` (step 2a) and the new YAML rows of all nodes.
- Flash +1.9 KB, RAM +0.8 KB (52 new rows since v002.00: all nodes' states + VARS2 state).

## v001.07 (2026-11-14)

- Task: VARS2 direct access part B (`UHA_COMMON/tasks/vars2_direct_access.md`), branch `feature/vars2_direct_access`
- CRC: `0x370114A8`

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
