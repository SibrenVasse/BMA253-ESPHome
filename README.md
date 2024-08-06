# BMA253 acceleration sensor ESPHome
```
sensor:
  - platform: bma253
    update_interval: 60s
    acceleration_x:
      name: "BMA253 Acceleration X"
    acceleration_y:
      name: "BMA253 Acceleration Y"
    acceleration_z:
      name: "BMA253 Acceleration Z"
    orientation:
      name: "BMA253 Orientation"
```

```
Orientation
00 portrait up    | 01 portrait down
10 landscape left | 11 landscape right
```