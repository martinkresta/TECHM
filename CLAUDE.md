# ASW_TECHM — Technical Room Controller

## Hardware
- **MCU:** STM32L431RC
- **CAN Node:** 3 (NODEID_TECHM)
- **HWID:** 0x103
- **Version:** 100 (Dec 2025)

## Role
Controls the house heating system: solid-fuel boiler, 6-coil electrical heater, thermal storage tanks, radiator circuit, wall heating circuit, and mixing valves. Also monitors water consumption and publishes 20+ temperature variables.

---

## Key Modules

| Module | Role |
|--------|------|
| HC (HEATING) | Boiler state machine — pump control, overheat protection |
| ELH (ELHEATER) | 6-coil electric heater control — SOC-based, power-balanced |
| BAC (BOILER_AIR) | Combustion air valve PID control |
| TC (TEMPCON) | Radiator & wall circuit temperature control |
| AVC_V2 | Servo-driven mixing valve position control |
| WM | Water flow meter pulse counting (cold + hot) |
| DO | Digital output drivers (pumps, heater relays, servo signals) |
| PTCTEMP | PTC analog temperature sensor linearization |
| SCOM | Serial comms to PC application |

---

## Sensors

### 1-Wire Temperature (18+ sensors)
Board, 6 storage tanks (T1–T6), boiler inlet/outlet, boiler exhaust, radiator hot/cold, wall circuit in/out, tank inlet/outlet (hot & cold), + outdoor and room temps via CAN.

### Analog
- 6 ADC channels for electric heater current measurement (one per coil)
- 1 PTC sensor via analog amplification

### Digital Inputs
| Input | Function |
|-------|---------|
| IN1 (STOVE_BTN) | Boiler button — 2/3/4-touch gesture recognition |
| IN2 (STOVE_DOOR) | Boiler door open/close |
| IN3 (AV_HOMEPOS) | Air valve home position sensor |

### Flow Meters (WM)
- Cold water consumption — interrupt-driven pulse counting → VAR_FLOW_COLD, VAR_CONS_COLD
- Hot water consumption → VAR_FLOW_HOT, VAR_CONS_HOT

---

## Actuators / Outputs

### Pumps (3)
- PUMP_BOILER — boiler circuit (on at 60°C, off at 50°C)
- PUMP_WALL — wall heating circuit
- PUMP_RAD — radiator circuit

### Servo Valves (TIM1 PWM)
- TIM1_CH1: Wall circuit mixing valve
- TIM1_CH2: Radiator mixing valve
- TIM1_CH3: Boiler air intake valve

### Electric Heater Relays (6)
Six individually switched 10A coils (max 95A total). Ramp-up control: 7s between each coil enable.

---

## Control Modules Detail

### HC — Boiler Control
State machine: Idle → HeatUp → Heating → CoolDown → Overheat → Error  
Pump on at 60°C, off at 50°C. Overheat limit 86°C. Tank max 95°C.

### ELH — Electric Heater (`ELHEATER.c`)

Controls 6 × 10A coils in the accumulation tank. Called every 1s via `ELH_Update_1s()`.  
Tank sensor used: `VAR_TEMP_TANK_6` (top sensor). Battery data from ELECON via CAN.  
Coil sequencing: 7s ramp between each step (`INCREASE_PERIOD_S`). Coils controlled via `DO_SetElHeaters(mHeaterMask)` where bit0 = bottom coil, bit5 = top coil.

#### Safety chain (all cause immediate off + return)
1. CAN data invalid (any subscribed VAR stale)
2. Emergency thermostat GPIO active (`ETS_Pin` high)
3. 48V supply missing (`PG_48V_Pin` low)
4. ELECON node not communicating (`COM_GetNodeStatus`)
5. Board temperature > 65°C
6. Tank top temperature > seasonal target (`mReqTankTemp`)

#### Seasonal tank targets
- May–Aug (`now.Month > 4 && now.Month < 9`): `SUMMER_REQ_TEMP = 70°C`
- Otherwise: `WINTER_REQ_TEMP = 90°C`

#### Operating modes — priority order (mutually exclusive `if/else if` chain)

**Mode A — Main mode** (`mSocEnableHys == 1`)  
Active after battery was fully balanced today (SOC ≥ 99% AND charging_A == 0). Stays active while SOC > 95%. Resets at midnight (`ELH_Midnight()`).  
Uses `ControlHeaterPower(battCurr_A)` to regulate load against battery current:  
- SOC > 96% + charger off + solar voltage ≥ 100V → `mMaxHeaterLoad = 95A` (burn full excess)  
- SOC > 96% + charger off + no sun → `mMaxHeaterLoad = 0` (night, don't discharge)  
- SOC > 96% + charger on → `mMaxHeaterLoad = 15A` (discharge faster than charge rate)  
- SOC ≤ 96% → `mMaxHeaterLoad = -2A` (charger gets priority)  
Published state: `eElh_Heating` / `eElh_NoFreePower`

**Mode B — Comfort heat** (`mSocComfortEna == 1` AND Apr–Sep AND tank < 55°C)  
*Added Jun 2026. Handles cloudy days when battery never reaches 100% but hot water is needed.*  
SOC hysteresis: enable at ≥ 43% (`COMFORT_SOC_ENABLE`), disable below 40% (`COMFORT_SOC_DISABLE`). Summer only (Apr–Sep, intentionally wider than main mode's May–Aug). Winter is never entered.  
- Tank < 54°C (`COMFORT_REQ_TEMP - 1`): turn on top 3 coils only (`COMFORT_COIL_MASK = 0x38`, bits 3–5). No power regulation — full 3 coils on.  
- Tank 54–55°C: hysteresis zone — maintain current output unchanged.  
- Tank ≥ 55°C: condition false, mode exits; balance support or else branch takes over.  
`mSocComfortEna` is NOT reset at midnight (tracks real-time SOC, not daily cycle).  
Published state: `eElh_ComfortHeat`

**Mode C — Balance support** (`mOptimalBalancingCurrent != 0` AND `mBatteryBalancedToday == 0`)  
ELECON sends an optimal balancing current via `ELH_SetBalanceInfo()`. TECHM heater draws that load to help reduce charging current during cell balancing, OR to utilise excess solar power throughout the day before battery is fully charged.  
Uses `ControlHeaterPower()` with `mMaxHeaterLoad = -mOptimalBalancingCurrent`.  
Safety: only runs if `mOptimalBalancingCurrent > 0` (positive = discharge target).  
Published state: `eElh_BalanceSupport`

**Mode D — Off** (else): `eElh_LowSOC`, all coils off.

#### Coil priority logic in `IncreasePower()` / `DecreasePower()`
- Tank top < `MIN_UTIL_TEMP_C` (45°C): top coils have priority (utility hot water)
- Tank top ≥ 45°C: bottom coils have priority (space heating energy)
- `mHeaterEnaMask` (default `0x3F`) allows individual coils to be disabled

#### External interfaces
- `ELH_SetBalanceInfo(balancedToday, optBalancingCurrent)` — called from APP when CAN message received from ELECON
- `ELH_Midnight()` — called at midnight reset; clears `mTodayEnergy_Wh`, `mBatteryBalancedToday`, `mSocEnableHys`
- `VAR_EL_HEATER_STATUS` publishes `mState` (0=Heating, 1=TempReached, 2=NoFreePower, 3=LowSOC, 4=48VFail, 5=TankOvertemp, 6=BoardOvertemp, 7=InvalidInputs, 8=ELeconComError, 9=SwitchError, 10=BalanceSupport, 11=ComfortHeat)

### BAC — Boiler Air Control
PID-based air valve: Kp=1.5, Ki=0.01, Kd=13, period=5s.  
Setpoints: HeatUp 200°C, Control 200°C, CoolDown 140–160°C.  
Safety: closes on door open or overheat.

### TC — Temperature Control (Radiators & Wall)
Hysteresis ±2°C. Pump delay 40s. Regulation period 30s.  
Radiator target: 40°C. Wall target: 47°C. Minimum tank energy thresholds enforced.

---

## Published VARS (CAN stream, 1–3s intervals)

| Group | VARs |
|-------|------|
| Temperatures | VAR_TEMP_TECHM_BOARD, VAR_TEMP_BOILER, VAR_TEMP_BOILER_IN/OUT, VAR_TEMP_TANK_1–6, VAR_TEMP_RAD_H/C, VAR_TEMP_WALL_IN/OUT, VAR_TEMP_TANK_IN_H/C, VAR_TEMP_TANK_OUT_H/C, VAR_TEMP_BOILER_EXHAUST |
| Water | VAR_FLOW_COLD, VAR_FLOW_HOT, VAR_CONS_COLD, VAR_CONS_HOT |
| Heating | VAR_EL_HEATER_STATUS, VAR_EL_HEATER_POWER, VAR_EL_HEATER_CURRENT, VAR_EL_HEATER_CONS, VAR_HEAT_HEATING_WH, VAR_HEAT_TOTAL_WH |
| Boiler | VAR_BOILER_POWER, VAR_BOILER_HEAT |

---

## Read from CAN
- VAR_BAT_SOC — controls ELH enable/disable thresholds
- RTC sync from RPi

---

## Notes for Future Work
- TECHM is the most complex node — multiple interdependent control loops
- PC serial interface (SCOM, 57600 baud) for diagnostics
- Watchdog: 3-second timeout
