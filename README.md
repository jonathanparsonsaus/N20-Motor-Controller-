This Project is still a work in progress, with the aim is to provide dual motor control for an N20 motor with built in quadrature encoder from an arduino nano
Update: Now the set PWM speed function are working
Help interface created to explain commands
Motor closed loop speed now working taking command -5000 to 50000 but only proportional control. Need to implement Integral and Derivative
Need to understand if we are actually servicing the loop at 100ms or if the interrupt service routine is taking up additional compute