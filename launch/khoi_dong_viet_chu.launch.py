import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node

def generate_launch_description():
    ur_gazebo_pkg = get_package_share_directory('ur_simulation_gazebo')
    khoi_dong_gazebo_moveit = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(ur_gazebo_pkg, 'launch', 'ur_sim_moveit.launch.py')
        ),
        launch_arguments={'ur_type': 'ur3e'}.items()
    )

    node_sinh_vien = TimerAction(
        period=20.0,
        actions=[
            Node(
                package='ur_viet_chu',
                executable='node_dieu_khien',
                output='screen',
                parameters=[{'use_sim_time': True}]
            )
        ]
    )

    return LaunchDescription([
        khoi_dong_gazebo_moveit,
        node_sinh_vien
    ])