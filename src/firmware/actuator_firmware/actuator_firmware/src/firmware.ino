#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>
#include <rcl/error_handling.h>
#include <stdio.h>

#include <rmw_microros/rmw_microros.h>

#if !defined(MICRO_ROS_TRANSPORT_ARDUINO_SERIAL)
#error This example is only available for Arduino framework with serial transport.
#endif

#define ACTUATOR_PIN A1

// --- ROS2 entities ---
rcl_node_t node = rcl_get_zero_initialized_node();
rcl_subscription_t subscriber = rcl_get_zero_initialized_subscription();
std_msgs__msg__Int32 msg;

rclc_executor_t executor = rclc_executor_get_zero_initialized_executor();
rclc_support_t support;
rcl_allocator_t allocator;
rcl_timer_t timer;

// --- RCCHECK macros ---
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();} }
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){} }

// --- States for agent connection ---
enum micro_ros_state_t {
  WAITING_AGENT,
  AGENT_AVAILABLE,
  AGENT_CONNECTED
};
micro_ros_state_t state = WAITING_AGENT;

// --- Error loop ---
void error_loop() {
  while (1) {
    digitalWrite(ACTUATOR_PIN, HIGH); 
    delay(500);
    digitalWrite(ACTUATOR_PIN, LOW);
    delay(500);
  }
}

void wait_for_connection() {
  while (rmw_uros_ping_agent(1000, 5) != RMW_RET_OK) {
    // slow blink while waiting
    digitalWrite(ACTUATOR_PIN, HIGH);
    delay(2500);
    digitalWrite(ACTUATOR_PIN, LOW);
    delay(250);
  }
}

// --- Subscriber callback ---
void subscription_callback(const void * msgin) {
  const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
  digitalWrite(ACTUATOR_PIN, (msg->data == 0) ? HIGH : LOW);  
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  pinMode(ACTUATOR_PIN, OUTPUT);
  digitalWrite(ACTUATOR_PIN, LOW);

  delay(2000); // wait for Serial + agent connection

  // Initialize transport
  set_microros_serial_transports(Serial);
  
  wait_for_connection();

  allocator = rcl_get_default_allocator();
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // Node
  RCCHECK(rclc_node_init_default(&node, "micro_ros_arduino_node", "", &support));

  // Subscriber
  RCCHECK(rclc_subscription_init_default(
      &subscriber,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
      "micro_ros_arduino_subscriber"));

  // Executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA));
}

// --- Loop ---
void loop() {
  // Wait for agent before spinning
  switch (state) {
    case WAITING_AGENT:
      if (RMW_RET_OK == rmw_uros_ping_agent(100, 1)) {
        state = AGENT_CONNECTED;
      }
      delay(100);
      break;

    case AGENT_CONNECTED:
      RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
      break;
  }
}
