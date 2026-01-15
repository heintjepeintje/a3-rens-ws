#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <micro_ros_platformio.h>
#include <WiFi.h>


#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>
#include <rmw_microros/rmw_microros.h>

#define ACTUATOR_PIN A1
#define TIMEOUT_MS 5000
#define REQUIRED_PINGS 3
//Neopixel
#define NEOPIXEL_PIN 0
#define NEOPIXEL_COUNT 1
Adafruit_NeoPixel pixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setPixel(uint8_t r, uint8_t g, uint8_t b) {
  pixel.setPixelColor(0, pixel.Color(r, g, b));
  pixel.show();
}

// ROS objects
rcl_allocator_t allocator;
rclc_support_t support;
rcl_node_t node;
rcl_subscription_t subscriber;
rclc_executor_t executor;
std_msgs__msg__Int32 msg;

// State machine 
enum micro_ros_state_t {
  WAITING_AGENT,
  CONNECTED
};
micro_ros_state_t state = WAITING_AGENT;
micro_ros_state_t last_state = WAITING_AGENT;

// WiFi
char WIFI_SSID[] = "GL-A1300-04e";
char WIFI_PASS[] = "HH936XSKBZ";

// micro-ROS agent
IPAddress agent_ip(192, 168, 10, 168);   // <-- IP van ROS 2 PC
const uint16_t agent_port = 8888;      // standaard micro-ROS UDP poort

// Variables
unsigned long last_msg_time = 0;
uint8_t ping_ok_count = 0;

// Error loop
void error_loop() {
  while (1) {
    setPixel(255, 0, 0);
    delay(500);
    setPixel(0, 0, 0);
    delay(500);
  }
}

// Subscriber callback 
void subscription_callback(const void * msgin) {
  const std_msgs__msg__Int32 * msg =
      (const std_msgs__msg__Int32 *)msgin;

  last_msg_time = millis();

  // 0 = uitgeschoven (HIGH), anders inschuiven (LOW)
  digitalWrite(ACTUATOR_PIN, (msg->data == 0) ? LOW : HIGH);
}

// Create ROS entities
bool create_entities() {
  allocator = rcl_get_default_allocator();

  if (rclc_support_init(&support, 0, NULL, &allocator) != RCL_RET_OK)
    return false;

  if (rclc_node_init_default(
        &node,
        "actuator",
        "",
        &support) != RCL_RET_OK)
    return false;

  if (rclc_subscription_init_best_effort(
        &subscriber,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "actuator_subscriber") != RCL_RET_OK)
    return false;

  if (rclc_executor_init(
        &executor,
        &support.context,
        1,
        &allocator) != RCL_RET_OK)
    return false;

  if (rclc_executor_add_subscription(
        &executor,
        &subscriber,
        &msg,
        &subscription_callback,
        ON_NEW_DATA) != RCL_RET_OK)
    return false;

  return true;
}

// Destroy ROS entities
void destroy_entities() {
  rclc_executor_fini(&executor);
  rcl_subscription_fini(&subscriber, &node);
  rcl_node_fini(&node);
  rclc_support_fini(&support);
}

// Setup
void setup() {
  pixel.begin();
  pixel.setBrightness(50);   // niet verblindend
  setPixel(255, 0, 0);         // uit bij start

  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  pinMode(ACTUATOR_PIN, OUTPUT);
  digitalWrite(ACTUATOR_PIN, LOW); // veilige start

  delay(2000);

  // micro-ROS WiFi transport
  set_microros_wifi_transports(
    WIFI_SSID,
    WIFI_PASS,
    agent_ip,
    agent_port
  );
  delay(2000);
}

// Standard loop
void loop() {
  static bool led_state = false;
  static unsigned long last_led_toggle = 0;

  // Detecteer state change
  if (state != last_state) {
    led_state = false;
    last_led_toggle = millis();

    if (state == WAITING_AGENT) {
      setPixel(0, 0, 0);     // LED reset
      ping_ok_count = 0;
    } 
    else if (state == CONNECTED) {
      setPixel(0, 255, 0);   // Vast groen
    }

    last_state = state;
  }

  switch (state) {

    case WAITING_AGENT:
      // Blauw knipperen
      if (millis() - last_led_toggle > 500) {
        last_led_toggle = millis();
        led_state = !led_state;
        setPixel(0, 0, led_state ? 255 : 0);
      }

      // Ping agent
      if (rmw_uros_ping_agent(500, 1) == RMW_RET_OK) {
        ping_ok_count++;
      } else {
        ping_ok_count = 0;
      }

      // Pas verbinden na meerdere succesvolle pings
      if (ping_ok_count >= REQUIRED_PINGS) {
        if (create_entities()) {
          state = CONNECTED;
        } else {
          ping_ok_count = 0;
        }
      }
      break;

    case CONNECTED:
      // Verbindingsverlies detecteren
      if (rmw_uros_ping_agent(100, 1) != RMW_RET_OK) {
        destroy_entities();
        state = WAITING_AGENT;
        break;
      }

      rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));

      // Timeout → actuator veilig
      if (millis() - last_msg_time > TIMEOUT_MS) {
        digitalWrite(ACTUATOR_PIN, LOW);
      }
      break;
  }
}