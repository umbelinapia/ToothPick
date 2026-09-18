# Gantt chart

Task breakdown and ownership

## Preliminary

| Task | Responsible |
|---|---|
| Success criteria definition | All |
| Workspace dimensions definition | All |
| Disc specifications definition | All |
| Requirements identification | Umbelina |
| Gantt chart | Umbelina |
| Task repartition | Umbelina |
| Camera selection | Umbelina |
| Bill of materials | All |

## Mechanical

| Task | Responsible |
|---|---|
| Layout definition vs. workspace | Umbelina |
| Arm architecture selection | Umbelina |
| Link length calculations | Umbelina |
| Reachability analysis | Umbelina |
| Flip mechanism concept design | Max |
| Gripper type selection | Max |
| Gripper CAD design | Max |
| Mounting CAD design | Max |
| CAD → print | Max |
| Order parts | Umbelina |
| Assembly | Umbelina |

## Electrical

| Task | Responsible |
|---|---|
| Power requirement calculation | Nikitha |
| Power supply selection | Nikitha |
| ESP32 configuration | Nikitha |
| End-effector actuator driver | Nikitha |
| Sensor selection | Nikitha |
| Wiring diagram | Max |
| Wiring | Umbelina |
| Cable management | Umbelina |
| Safety switch integration | Max |

## Software

| Task | Responsible |
|---|---|
| ESP32 environment setup | Nikitha |
| Motor driver interface code | Nikitha |
| Encoder reading | Nikitha |
| Joint limit implementation | Umbelina |
| Motion profiling / speed tuning | Umbelina |
| Joint control (fkine) | Umbelina |
| Kinematics (ikine) | Umbelina |
| Trajectory planning | Umbelina |
| Pick sequence routine | Umbelina |
| Flip sequence routine | Umbelina |
| Place sequence routine | Umbelina |
| Flip/drop safety interlocks (home-position gating) | Umbelina |
| Camera calibration | Umbelina |
| Workspace calibration (pixel → mm transform) | Umbelina |
| Disc detection algorithm | Umbelina |
| Coordinate transformation to robot frame | Umbelina |
| Edge-case pick routine (disc near workspace boundary) | Umbelina |

## Testing

| Task | Responsible |
|---|---|
| Single-joint test | All |
| Multi-joint coordinated motion test | All |
| Dry run without disc | All |
| Pick test | All |
| Flip test | All |
| Full pipeline test | All |

## Validation

| Task | Responsible |
|---|---|
| Accuracy measurement | Daniel |
| Flip success rate measurement | Daniel |
| Cycle time measurement | Daniel |
| System limitations analysis | All |
| Presentation preparation | All |
| Code documentation (this repo) | Umbelina |
| CAD documentation (this repo) | Umbelina |
| Oral presentation (Viva Voce) | All |
