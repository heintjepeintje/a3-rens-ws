# Workspace voor Project Robot Vloot (Groep A3)

## Uitleg
Dit project is bedoelt als algemene workspace voor de robots van de robot vloot. Het bevat dus niet de code voor de fleetmanager.
Het project is opgesteld zodat iedere losse component een eigen package is en dus ook apart te runnen is.

## Benodigdheden
### Software
- [ROS2 (Jazzy)](https://docs.ros.org/en/jazzy/index.html)
- [PlatformIO](https://platformio.org/)
- [Rens Hardware](https://github.com/AvansTi/rens_hardware)
- [RViz](https://wiki.ros.org/rviz)
### Hardware
- ReNS robot

## Structuur
```
src
├─ controller
├─ firmware
├─ linorobot2
│  ├─ linorobot2_bringup
│  ├─ linorobot2_description
│  ├─ linorobot2_gazebo
│  ├─ linorobot2_navigation
├─ micro_ros_setup
├─ oak_detection_ws
│  ├─ oak_camera_debug
│  ├─ oak_camera_detection
└─ uros
```

De meest 'lage' folder die je in het bovenstaande diagram ziet is een 'project' binnen de workspace, deze zijn gegroepeerd om zo soortgelijke functionaliteit bij elkaar te houden.

## Uitvoeren 
Er zijn verschillende componenten van de robot die allemaal los uitvoerbaar zijn:
### 1. Verbinding maken met de robot
Om met de robot te verbinden maken we gebruik van een SSH-server op de robot. Voor je de juiste IPv4 adressen kunt gebruiken is het eerst nodig om de volgende opzet te hebben met de robot, de fleetmanager en de routers.

Vervolgens kun je de individuele robots bereiken met behulp van de volgende command:
```bash
ssh rens@<ip_robot>
```
Hierbij vervang je ```<ip_robot>``` door het IPv4 adres van je robot. 

### 2 Opzetten van het project
Om het project te compileren voer je de volgende commando uit
```bash
colcon build
```
Om vervolgens ros2 de gebouwde nodes te laten herkennen heb je de volgende commando nodig:
```bash
source install/local_setup.sh
```

### 3 Bringup
Om de robot op te starten heb je de volgende command nodig:
```bash
ros2 launch linorobot2_bringup bringup.launch.py
```

### 4 Input
Om de robot te kunnen besturen is er een vorm van input nodig. Dit kan zowel via een controller als via een toetsenbord.

#### 4.1 Controller input 
```bash
ros2 launch teleop_twist_joy teleop-launch.py joy_config:=ps3
```

#### 4.2 Toetsenbord input
```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard
```

### 5 De robot intern aansturen
```bash
ros2 launch cmd_vel_arbiter arbiter.launch.py
```

### 5 SLAM/Navigie
Om de robot zijn eigen map te laten maken is SLAM nodig. Hiervoor voer je de volgende commando's uit:
```bash
ros2 launch linorobot2_navigation slam.launch.py
```

#### 5.1 Navigatie (Nav2) met externe map
```bash
ros2 launch linorobot2_navigation navigation.launch.py map:=<path_to_map>.yaml
```
Vervang hierbij ```<path_to_map>``` door de bestandsnaam van je lokale map.

### 6 Camera

#### 6.1 Opstarten
```bash
ros2 run depthai_ros_driver camera_node --ros-args --params-file oak_detection_ws/yolo.yml
```

#### 6.2 Camera Detection
```bash
ros2 run oak_camera_detection detection
```

#### 6.3 Camera Feed
```bash
ros2 run oak_camera_debug preview
```

### 7 Ultrasonic Sensor

#### 7.1 Ultrasonic listener
```bash
ros2 run ultrasonic_listener ultrasonic
```

#### 7.2 Ultrasonic 
```bash
ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888
```

### 7. Actuator

#### 7.1 Actuator Agent
```bash
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/micro_ros_esp32 -b 115200
```

**NOTE:** Als dit een error geeft, probeer dan de ESP32 opnieuw te flashen.
```bash
pio run --target upload -e adafruit_feather_esp32_v2
pio device monitor -p /dev/ttyACM1 -b 115200
```

#### 7.2 Node voor het omzetten van data vanuit de camera naar de actuator
```bash
ros2 run oak_to_actuator center_actuator
```

## Auteurs 

- Hein Dekkers ([@heintjepeintje](https://github.com/heintjepeintje))
- Benthe Vermeulen ([@ijsbeer05](https://github.com/ijsbeer05))
- Nicky Buurstee ([@RetroTrack](https://github.com/RetroTrack))
- Abdullah Al Kathiry ([@lateNightCoder200](https://github.com/lateNightCoder200))
- Kyara Mennens ([@gravityjournal](https://github.com/gravityjournal))
- Dimitri van der Hel ([@Dimitri1708](https://github.com/dmitri1708))

Het is belangrijk om te vermelden dat degene door wie de commit is gemaakt niet per-se degene is die de veranderingen heeft gemaakt. Dit komt doordat dan een ander persoon ingelogd was op github in de robot. 
