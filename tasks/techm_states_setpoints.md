# TECHM: states and setpoints (step 2c)

Precondition: 2a and 2b done (`UHA_COMMON/tasks/vars2_states_on_change.md`, `UHA_COMMON/tasks/states_nodes.md`).
Same rules: lightweight mode, existing tests pass, **Martin flashes**, `ASW_VERSION` bump + CHANGELOG, feature
branch, subgroup `States.Techm`, ids 440–479.

## Why this node is special

TECHM drives the electric water heater coils directly from the battery with no hardware protection — protection is
firmware only (see `decisions.md` / `ASW_TECHM/CLAUDE.md`). Therefore:
- **No change of any control decision.** Only publish values that exist, and turn hard-coded setpoints into
  variables with exactly the same initial values.
- In the report, list **every changed line that touches ELHEATER, HEATING, TEMPCON, BOILER_AIR, DO** (file, what,
  why it cannot change behaviour). Martin reads this before flashing.

## Setpoints (become variables, published, settable in step 3)

| Today | Becomes | Publish as (`float` °C, on change) |
|---|---|---|
| `mUpReqTempDay` / `mUpReqTempNight` (TEMPCON, already variables, set in `TC_Init()`) | unchanged | `UpReqTempDay_C`, `UpReqTempNight_C` |
| `mDownReqTempDay` / `mDownReqTempNight` | unchanged | `DownReqTempDay_C`, `DownReqTempNight_C` |
| `BAC_DEF_CONTROL_TEMP` (`#define 200` in `BOILER_AIR.h`) | static variable `mBacControlTemp_C`, initialised from the define, used where the define is used for the control setpoint | `ExhaustSetpoint_C` |

Day/night hours stay as they are and are not published. No persistence (flash/backup) now — later UhaGatePy will
re-send setpoints after a node restart (step 3).

## States (on change, `agg: event`)

- Currently valid room requests: `upReqTemp`, `downReqTemp` from TEMPCON (day or night value in effect) — `float`.
- Circuits on: `mRadOn`, `mWallOn` — `bool`.
- Pumps: boiler, wall, radiator — `bool`, the state really set via `DO_SetPumpBoiler/Wall/Rad()` (keep the last
  value in DO.c if it is not stored yet).
- Electric heater: number of coils switched on (popcount of the mask actually given to `DO_SetElHeaters()`) —
  `uint32` 0–6.
- `EL_HEATER_STATUS` (existing variable, id unchanged, group `ElHeater`): switch to `send: on_change`,
  `period_ms: 20000`, `agg: event`. Check every receiver of this id still works with the 60 s timeout.
- Boiler state: `eBoilerState` (`HEATING.h`) as enum `BoilerState`
  {Idle, HeatUp, Heating, CoolDown, Overheat, Error, InvalidInputs} (values from the code).

## Periodic values (not on change; `period_ms: 5000`, `log_s: 60`, `agg: avg`)

Put them in the `States.Techm` subgroup but with `send` overridden to periodic (or a separate subgroup `Heating` —
choose what the validator allows more simply and say which):
- Mixing valve positions wall and radiators, boiler air valve position — % as really commanded
  (`DO_SetServoWall/Rad/AirValve()`).
- Required water temperatures `mReqWaterRad_C`, `mReqWaterWall_C` (TEMPCON).

## UhaGatePy / Grafana

New YAML is enough (deploy). Grafana States dashboard: TECHM row — pumps, circuits, boiler state, heater coils,
`EL_HEATER_STATUS` timeline; setpoints vs. actual room temperatures (time series with `TEMP_KIDROOM_C`,
`TEMP_DOWNSTAIRS_C`); valve positions.

## Report and stop

Version, variable list (id, name, source), the line-by-line list of touched control files, RAM/flash delta.
Stop — Martin reviews the list, flashes and checks.
