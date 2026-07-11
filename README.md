# STM32 Line-Following Robot

Academic embedded systems project implementing an STM32-based robotic car capable of following a line using infrared sensors. The robot also supports manual control through USART commands sent from a PC.

The system combines sensor reading, PWM motor control, ADC-based speed adjustment, timer interrupts, GPIO control and serial communication.

## Features

- Autonomous line-following mode using two infrared sensors
- Manual movement commands through USART
- DC motor speed control using PWM
- Potentiometer-based speed adjustment using ADC
- Buzzer feedback depending on the detected line state
- Timer interrupt-based periodic control loop
- GPIO-based motor direction control
- User button used as a start condition

## Hardware

- STM32 development board
- DC motors
- Motor driver
- Two infrared line sensors
- Potentiometer
- Buzzer
- USART-to-USB adapter
- External battery supply

## Peripherals Used

| Peripheral | Purpose |
|---|---|
| TIM3 CH1 / CH3 | PWM generation for motor speed control |
| TIM4 CH3 | Periodic interrupt for sensor reading and control logic |
| ADC1 IN5 | Potentiometer reading for speed adjustment |
| USART1 | Serial communication with the PC |
| GPIO Inputs | Line sensors and user button |
| GPIO Outputs | Motor direction control and buzzer |

## Pinout

| Signal | Pin |
|---|---|
| Motor IN1 | PC6 |
| Motor IN2 | PC7 |
| Motor IN3 | PC8 |
| Motor IN4 | PC9 |
| Line sensor 1 | PC1 |
| Line sensor 2 | PC2 |
| Potentiometer | PA5 |
| Buzzer | PB8 |
| USART RX | PB7 |
| USART TX | PB6 |
| User button | PA0 |

## Hardware Setup

<img src="image/setup.jpg" alt="Hardware setup" width="500">

## Operating Modes

The robot can work in two main modes:

### Manual mode

The movement direction is selected through USART commands received from the PC.

Supported commands include:

| Command | Action |
|---|---|
| `0` | Stop |
| `1` | Forward |
| `2` | Backwards |
| `3` | Right |
| `4` | Left |
| `5` | Automatic mode |
| `6` | Speed level 1 |
| `7` | Speed level 2 |
| `8` | Speed level 3 |
| `9` | Maximum speed |

### Automatic mode

In automatic mode, the robot reads both line sensors periodically and adjusts the wheel speeds depending on the detected line position.

The control loop is handled inside the TIM4 interrupt routine. The robot also uses the buzzer to provide feedback when a line is detected.

## Implementation Details

- TIM3 is configured in PWM mode to control the speed of both motors.
- TIM4 is configured as a periodic timer interrupt.
- ADC1 reads the potentiometer value and maps it to a maximum speed value.
- USART1 receives one-character commands from the PC.
- GPIO registers are used to read sensors and control motor direction.
- UART transmission and reception are handled using interrupt callbacks.


