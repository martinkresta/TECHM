# TECHM firmware changelog

Newest version first. Versions before v001.03 are not listed.

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
