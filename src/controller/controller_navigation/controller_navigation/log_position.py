#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
import math

import tf2_ros
from geometry_msgs.msg import TransformStamped


class PoseLogger(Node):
    def __init__(self):
        super().__init__('pose_logger')

        self.tf_buffer = tf2_ros.Buffer()
        self.tf_listener = tf2_ros.TransformListener(self.tf_buffer, self)

        self.timer = self.create_timer(0.5, self.timer_callback)

    def timer_callback(self):
        try:
            t: TransformStamped = self.tf_buffer.lookup_transform(
                'map',
                'base_link',   # of 'base_footprint'
                rclpy.time.Time()
            )

            x = t.transform.translation.x
            y = t.transform.translation.y

            q = t.transform.rotation
            yaw = math.atan2(
                2.0 * (q.w * q.z + q.x * q.y),
                1.0 - 2.0 * (q.y * q.y + q.z * q.z)
            )

            self.get_logger().info(
                f'Robot pose (SLAM) — x: {x:.3f}, y: {y:.3f}, yaw: {yaw:.3f}'
            )

        except Exception as e:
            self.get_logger().warn('Waiting for TF map → base_link...')


def main():
    rclpy.init()
    node = PoseLogger()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
