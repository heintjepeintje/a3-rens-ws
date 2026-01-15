#!/usr/bin/env python3

import rclpy
import math
from geometry_msgs.msg import PoseStamped
from nav2_simple_commander.robot_navigator import BasicNavigator, TaskResult


def main():
    rclpy.init()

    navigator = BasicNavigator()

    print("⏳ Wachten tot Nav2 actief is...")
    navigator.waitUntilNav2Active(localizer=False)

    # --- USER INPUT ---
    x = float(input("Goal X (meters): "))
    y = float(input("Goal Y (meters): "))
    yaw_deg = float(input("Yaw (graden): "))

    yaw = math.radians(yaw_deg)

    # yaw → quaternion
    qz = math.sin(yaw / 2.0)
    qw = math.cos(yaw / 2.0)

    goal_pose = PoseStamped()
    goal_pose.header.frame_id = "map"
    goal_pose.header.stamp = navigator.get_clock().now().to_msg()

    goal_pose.pose.position.x = x
    goal_pose.pose.position.y = y
    goal_pose.pose.orientation.z = qz
    goal_pose.pose.orientation.w = qw

    print(f"🚀 Navigating to x={x}, y={y}, yaw={yaw_deg}")
    navigator.goToPose(goal_pose)

    while not navigator.isTaskComplete():
        rclpy.spin_once(navigator, timeout_sec=0.1)

    result = navigator.getResult()

    if result == TaskResult.SUCCEEDED:
        print("✅ Goal bereikt!")
    elif result == TaskResult.CANCELED:
        print("⚠️ Navigatie geannuleerd")
    elif result == TaskResult.FAILED:
        print("❌ Navigatie mislukt")

    rclpy.shutdown()


if __name__ == '__main__':
    main()
