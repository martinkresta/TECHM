# Step 2c report — TECHM states and setpoints

Date 2026-11-16. Brief: `ASW_TECHM/tasks/techm_states_setpoints.md`. Steps 2a and 2b are merged and pushed.

**Status:** TECHM **v002.01** is built (`Deployment/v002.01/TECHM_ASW_v002.01.bin`, CRC `0xFE528C60`) and not
flashed. The new YAML and the Grafana TECHM row are deployed on the Pi. All changes are committed on branch
`feature/vars2-states-techm` in ASW_TECHM, UHA_COMMON and UHARPI — **not merged, not pushed.**
Flash +1884 B, RAM +848 B against v002.00. That covers 52 new table rows (all nodes' states since v002.00, ~16 B RAM
each) plus 12 B of new statics.

## 1. Touched control files — line by line (please review before flashing)

**`ELHEATER.c` and `HEATING.c` are not changed at all.** The heater status and the boiler state are read through
what exists: `Vars2.ElHeater.State` (written by ELHEATER as before) and the existing `HC_GetStatus()`.

### `Core/Src/DO.c` (output drivers)

| line(s) | change | why it cannot change behaviour |
|---|---|---|
| 15–20 | 5 new `static` variables `mPumps`, `mElHeaters`, `mServoRadPct`, `mServoWallPct`, `mServoAirValvePct` | Storage only. They are written by the setters and read only by the new getters; no existing code reads them. |
| 36 | `DO_SetElHeaters()`: first line `mElHeaters = heaters;` | Only copies the argument. All six `HAL_GPIO_WritePin()` calls after it are unchanged. |
| 129 / 142 / 156 | `DO_SetPumpBoiler/Wall/Rad()`: first line sets / clears the bit `PUMP_BOILER` / `PUMP_WALL` / `PUMP_RAD` in `mPumps` | Only bookkeeping of the argument; the `if (state)` GPIO write after it is unchanged. |
| 193 / 221 / 247 | `DO_SetServoRad/Wall/AirValve()`: `mServoXxxPct = pct;` after the 0..100 clamp and the duty-cycle calculation | Copies the clamped value. `sConfigOC.Pulse`, PWM stop / config / start are unchanged and come after it. |
| 260–265 | 5 new getters `DO_GetPumps()`, `DO_GetElHeaters()`, `DO_GetServoRad/Wall/AirValve()` | Each only returns one of the variables above. Called only from `APP_UpdateStates_1s()`. |

### `Core/Src/TEMPCON.c` (radiators / walls)

| line(s) | change | why it cannot change behaviour |
|---|---|---|
| 56–57 | 2 new globals `mUpReqTemp_C`, `mDownReqTemp_C` | Storage only; TEMPCON never reads them. |
| 168–169 | after the day/night selection: `mUpReqTemp_C = upReqTemp; mDownReqTemp_C = downReqTemp;` | Copies the locals that were just selected. All later decisions still use the locals `upReqTemp` / `downReqTemp`, unchanged. |

Not changed: the setpoints `mUpReqTempDay/Night`, `mDownReqTempDay/Night` stay where they are (set in `TC_Init()`,
same values 21.5 / 19.5 / 21.5 / 21.5). `TEMPCON.h` only got `extern` declarations of existing globals, so APP.c can
read them.

On the early return (invalid room / water inputs: pumps off) the two copies keep the last value; that is only what
is published, not used in control.

### `Core/Src/BOILER_AIR.c` (boiler air valve)

| line(s) | change | why it cannot change behaviour |
|---|---|---|
| 26 | new global `float mBacControlTemp_C = BAC_DEF_CONTROL_TEMP;` | Initialised from the same define (200). Nothing writes it (settable only in step 3). |
| 200 | `SetState()` case `es_AirControl`: `mPidSetpoint_C = BAC_DEF_CONTROL_TEMP;` → `mPidSetpoint_C = mBacControlTemp_C;` | `mPidSetpoint_C` is a `float`; before it got `200` converted to `200.0f`, now it gets the variable that holds `200.0f`. Same value. |

`BAC_DEF_CONTROL_TEMP` had exactly this one use. The heat-up, cool-down and stop thresholds (`BAC_DEF_HEATUP_TEMP`,
`BAC_DEF_COOLDOWN_*`, `BAC_DEF_STOP_TEMP`) are untouched.

One small deviation from the brief: the brief says "static variable". I made it a plain global with an `extern` in
`BOILER_AIR.h`, so that APP.c can read it the same way as the TEMPCON setpoints. If you prefer `static` + a getter,
that's a 3-line change.

### `Core/Src/scheduler.c`

| line | change | why it cannot change behaviour |
|---|---|---|
| 279 | `APP_UpdateStates_1s();` after `BAC_Update_1s()` in `Timer_Task_1s()` | It runs after all control tasks of the second and only writes `Vars2.States.Techm`; it calls no setter and changes no module variable. Order of ELH / HC / WDG / APP / TC / BAC is unchanged. |

### `Core/Src/APP.c` (not a control file, for completeness)

- `APP_Init()`: `VARS2_SetGroupValid(&Vars2.States.Techm, 1);` after `UHAMON_Init()`.
- New `APP_UpdateStates_1s()`: reads the values below and writes them into `Vars2.States.Techm`. The only
  computation is counting the set bits of the heater mask (0..6).

## 2. Variables (`States.Techm`, ids 440–464)

On change (`send: on_change`, 20 s refresh, `agg: event`):

| id | name | type | source |
|----|------|------|--------|
| 440 | ST_UP_REQ_DAY_C | float °C | TEMPCON `mUpReqTempDay` (21.5) |
| 441 | ST_UP_REQ_NIGHT_C | float °C | `mUpReqTempNight` (19.5) |
| 442 | ST_DOWN_REQ_DAY_C | float °C | `mDownReqTempDay` (21.5) |
| 443 | ST_DOWN_REQ_NIGHT_C | float °C | `mDownReqTempNight` (21.5) |
| 444 | ST_EXHAUST_SET_C | float °C | BOILER_AIR `mBacControlTemp_C` (200) |
| 445 | ST_UP_REQ_C | float °C | `upReqTemp` in effect (copy `mUpReqTemp_C`) |
| 446 | ST_DOWN_REQ_C | float °C | `downReqTemp` in effect (copy `mDownReqTemp_C`) |
| 447 | ST_RAD_ON | bool | TEMPCON `mRadOn` |
| 448 | ST_WALL_ON | bool | TEMPCON `mWallOn` |
| 449 | ST_PUMP_BOILER | bool | last `DO_SetPumpBoiler()` (HEATING) |
| 450 | ST_PUMP_WALL | bool | last `DO_SetPumpWall()` (TEMPCON) |
| 451 | ST_PUMP_RAD | bool | last `DO_SetPumpRad()` (TEMPCON) |
| 452 | ST_EL_HEATER_COILS | uint32 0–6 | set bits of the last `DO_SetElHeaters()` mask |
| 453 | ST_BOILER_STATE | enum BoilerStatus | `HC_GetStatus()` = `mBoilerState` |

Periodic (`send: periodic` on the variable inside `States.Techm`, `period_ms: 5000`, `log_s: 60`, avg). That was
the simpler option: the validator already allows `send` on a variable, so no separate subgroup was needed.

| id | name | type | source |
|----|------|------|--------|
| 460 | TECHM_VALVE_WALL_PCT | float % | last `DO_SetServoWall()` |
| 461 | TECHM_VALVE_RAD_PCT | float % | last `DO_SetServoRad()` |
| 462 | TECHM_AIR_VALVE_SIGNAL_PCT | float % | last `DO_SetServoAirValve()` — see note |
| 463 | TECHM_REQ_WATER_RAD_C | float °C | TEMPCON `mReqWaterRad_C` |
| 464 | TECHM_REQ_WATER_WALL_C | float °C | TEMPCON `mReqWaterWall_C` |

Changed existing variable: **`EL_HEATER_STATUS` (id 80, group `ElHeater`)** is now `send: on_change`,
`period_ms: 20000` (was 3000), `agg: event`. Same id, value and meaning. Receivers: no firmware node reads it
(checked in all ASW projects). On the RPi, `uha.json` and `uha-heat.html` take its last value of any age, and
`uha2.json` treats it as fresh for 60 s (3 × 20 s) — the 20 s refresh keeps it valid. It now also gets rows in
`events`, so it shows as a timeline in Grafana.

## 3. Notes / decisions

1. **Enum name `BoilerStatus` instead of `BoilerState`** — the same clash as `ACState` in 2b: the generated
   `eBoilerState` would collide with `eBoilerState` in `HEATING.h`. Same values:
   0 Idle, 1 HeatUp, 2 Heating, 3 CoolDown, 4 Overheat, 5 Error, 6 InvalidInputs (checked against the code). The
   variable is still called `ST_BOILER_STATE` / `BoilerState`.
2. **The air valve value is the servo signal, not the flap opening.** `DO_SetServoAirValve()` gets `mSignalDC` from
   `AVC_V2.c` = servo voltage × 10, i.e. 20..100 % for 2..10 V (0..90° servo). As the brief says, I published what
   is given to `DO_SetServoAirValve()`, hence the name `..._SIGNAL_PCT`. If you'd rather see the requested opening
   (`reqSectionPct` in `LinearizeAndMove()`), that would be one line in AVC_V2.c instead of DO.c.
3. **Pumps / coils at power-up** are 0 until the first setter call — the same as the GPIOs, which start low.
4. **Write timing:** all states are written once per second after the control tasks, so what is published is the
   state after this second's decisions. On-change frames then go out within 10 ms (min 1 s apart).
5. Day/night hours are not published (as the brief says). No persistence of setpoints.

## 4. Tests and build

- VarDefs tests pass (validator 69/69, compile check, vars2, com, mcan, tunnel). UhaGatePy 72 tests pass on the PC and
  on the Pi. `test_vars_def.py` now compares only the ids of the old flat YAML and allows the intended id-80 change.
- TECHM builds. No new warnings in the changed code (the ones in the log were there before, e.g. `htim` unused in DO.c).
- Other nodes are not rebuilt: their tables simply don't know ids 440–464, which they ignore.

## 5. RPi (deployed 08:12)

- Gateway: new YAML, restarted, clean journal. 310 variables, 48 event variables, 181 logged values (+5 periodic
  TECHM values).
- Grafana **UHA States**, TECHM row: Relays (circuits + pumps), `ElHeaterState` timeline, `BoilerStatus` timeline,
  room setpoints day / night (stepped), heater coils, "Room request in effect vs room temperature" (with
  `TEMP_KIDROOM_C` / `TEMP_DOWNSTAIRS_C`), "Exhaust setpoint vs exhaust", "Valves (commanded %)", "Water required
  vs actual" (with `TEMP_RAD_H_C` / `TEMP_WALL_IN_C`).
- Fix in the generator: the label of a state is now the name without `ST_` only if it really starts with `ST_`
  (`EL_HEATER_STATUS` would otherwise have shown as `HEATER_STATUS`).
- **Found and fixed a web-dashboard side effect (UhaGatePy 0.4.1, deployed 08:16):** `WS/js/uha-common.js` marks every
  value older than 10 s as "old" (grey). On-change states are only refreshed every 20 s, so `EL_HEATER_STATUS` on
  `uha-heat.html` (and any state shown later) would have looked stale most of the time. Now `uha2.json` has
  `"on_change": true` on these 48 variables (an extra key, nothing removed), and `ageClass()` skips the age check
  for them — their `valid` flag (60 s timeout) still shows a missing node. Only `uha-common.js` was copied to
  `/home/pi/Web/js/` (backup `uha-common.js.bak-0.4.0` next to it); your uncommitted `WS/uha-climate.html` change
  was not touched or deployed.

## What you need to do

1. Read section 1 (the touched control lines).
2. Flash TECHM v002.01.
3. Check: Grafana States → TECHM row fills in; `uha2.json` → `ST_*` of TECHM valid; heating behaves as before.
4. Then "merge 2c". Step 3 (commands / setpoints) only after your go.
