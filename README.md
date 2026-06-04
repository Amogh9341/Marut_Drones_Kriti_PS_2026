# Drone-Based Seedball Dispersal Payload
**Aeromodelling Marut Drones | Kriti '26 | IIT Guwahati | Hostel ID: 2651**

---

## Problem Statement

Manual reforestation is precise but slow. Traditional aerial broadcasting is fast but wasteful — seeds cluster, land on unsuitable surfaces, and compete for resources. There is no reliable, autonomous mechanism that drops seedballs one at a time at controlled intervals from a drone, especially when seedball sizes are non-uniform.

---

## Idea

Design a standalone, drone-mounted payload system that:
- Releases seedballs **one at a time** at adjustable intervals (1–5 seconds)
- Handles **non-uniform spherical seedballs** (15–35 mm diameter)
- Operates **autonomously** — detects and resolves jams without human intervention
- Fits within **20 kg** total mass and **300 × 350 × 350 mm** volume

Design approach: *working backwards from failure* — identify what goes wrong in seed dispensing systems first, then build around those failure modes.

---

## Implementation

### Architecture: Three-Stage Modular Pipeline

**Stage 1 — Hopper Assembly**
- Gravity-fed tapered storage container
- Integrated rotational agitator (DC geared motor) prevents bridging and seed stagnation
- Capacity: up to 20 kg total (mechanism + seedballs)

**Stage 2 — Horizontal Screw Conveyor (Auger)**
- Meters seedballs from hopper to dispensing stage
- Acts as a buffer — decouples hopper pressure from dispensing rate
- Driven by a stepper motor; monitored via TMC2209 driver for stall detection
- On jam: auto-reverses motor up to 3 times, then halts safely

**Stage 3 — Disc Dispensing Mechanism**
- Continuously rotating disc with 3 conical radial grooves at varying angles
- Accommodates all seedball sizes (15–35 mm) without manual adjustment
- Releases exactly one seedball per cycle through a single drop point

### Control System
- **Microcontroller:** ESP32
- **Motor driver:** TMC2209 (built-in stall detection — no external sensors needed for jam detection)
- **Interface:** Bluetooth / Wi-Fi input for start/stop and drop interval setting
- **Fault logic:** Jam detected → reverse motor → retry up to 3 times → halt if unresolved

### Materials
- **Carbon fiber** — structural frame (lightweight, high tensile strength)
- **Acrylic sheet** — housing panels (easy to fabricate, optically clear for inspection)

### Key Design Tolerances
- Internal flow paths sized at **1.15× the largest seedball diameter** to allow for size variation without multi-ball release
- Seed drop interval: **1–5 seconds** (adjustable)

---

## Prototype

Built at **1:2 scale** (14 × 17 × 22 cm footprint) using carbon fiber and acrylic.  
Test seedballs: ~13 mm diameter (50% scale).  
Validated: gravity-fed dynamics, auger flow consistency, stall detection, and automated anti-jam routine.

---

## Limitations

- Risk of multi-seedball release cannot be fully eliminated
- Sensitive to mechanical tolerances in screw and disc assemblies
- Moisture and debris can affect flow behaviour
- Maintenance automation (dust/earth accumulation) is unsolved

---

## Future Scope

- GPS-based location-aware seedball deployment
- Active gating for improved single-ball precision
- Multi-drone swarm synchronization for large-scale restoration
- Automated rinsing/cleaning mechanism
