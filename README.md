Motor Controller - N20 Motor Encoder Interface
Welcome to the Motor Controller project! This program provides a comprehensive interface to control N20 motors using encoder feedback. Whether you need open-loop control, closed-loop speed control, or position control, this project has you covered.

Features
This program allows you to:

Set Open-Loop Motor Speed: Control the motor speed directly by setting the PWM values.
Set Closed-Loop Motor Speed (ticks/s): Maintain a specific motor speed using encoder feedback.
Set Desired Position (ticks): Control the motor to move to a specific position based on encoder counts.
Available Commands:
Below is a list of commands that you can use to interact with the motor controller:

SET SPEED A <value>: Set the speed for Motor A (value between -255 to 255).
SET SPEED B <value>: Set the speed for Motor B (value between -255 to 255).
SET TARGET SPEED A <ticks/sec>: Set the target speed for Motor A in encoder ticks per second.
SET TARGET SPEED B <ticks/sec>: Set the target speed for Motor B in encoder ticks per second.
READ ENCODERS: Read and display the current encoder values for both motors.
HELP: Display the list of available commands and their descriptions.
Usage
Setting Open-Loop Speed:

To control the motor speed directly, use the SET SPEED A <value> or SET SPEED B <value> commands. Replace <value> with a number between -255 and 255. Positive values set the motor to move forward, and negative values set it to move backward.
Setting Closed-Loop Speed:

Use the SET TARGET SPEED A <ticks/sec> or SET TARGET SPEED B <ticks/sec> commands to control the motor speed in closed-loop mode. The controller will adjust the PWM signal to maintain the desired speed in encoder ticks per second.
Reading Encoder Values:

Use the READ ENCODERS command to retrieve the current encoder counts for both motors. This is useful for monitoring motor position and verifying control accuracy.
Getting Help:

If you need a reminder of the available commands, simply type HELP to see the list of commands and their descriptions.
License
This project is open-source and available under the MIT License.

This README should give users a clear understanding of what the project does, how to use it, and the available commands for controlling the motors.
