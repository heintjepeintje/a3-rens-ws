from setuptools import setup

package_name = 'controller_navigation'

setup(
    name=package_name,
    version='0.0.0',
    packages=[package_name],

    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],

    install_requires=['setuptools'],
    zip_safe=True,

    maintainer='rens',
    maintainer_email='rens@example.com',
    description='Custom navigation nodes for Linorobot2',
    license='Apache License 2.0',

    entry_points={
        'console_scripts': [
            'log_position = controller_navigation.log_position:main',
            'navigate_to_goal = controller_navigation.navigate_to_goal:main',
        ],
    },
)
