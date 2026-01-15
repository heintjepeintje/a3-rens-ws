from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='ultrasonic_listener',
            executable='ultrasonic',
            name='ultrasonic_reader'
        )
    ])
