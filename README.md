# 🚗 stm32-canbus-node-communication 
CAN Bus Communication Between Two STM32 Boards (Node-to-Node) 
 
This project demonstrates **CAN communication between two STM32 MCUs** using the HAL library.  
Node 1 periodically sends and requests data, while Node 2 responds and blinks LEDs based on received commands.  
The setup closely follows the concept shown below, adapted to different hardware and extended with additional debugging tools. 

<img width="949" height="521" alt="project-overview" src="https://github.com/user-attachments/assets/899997a8-b823-4d92-810d-3d7bd9b19379" /> 
 
--- 
 
## 🖼️ Project Overview 
 
| Node   |  Board            |   Role                                  | 
|--------|-------------------|-----------------------------------------| 
| Node 1 | NUCLEO-L476RG     | CAN **Master** – sends and request data | 
| Node 2 | STM32F4-Discovery | CAN **Slave**  – responds, blinks LEDs  | 
 
 
--- 
 
## 🛠️ Features 
- CAN initialization with HAL library 
- **Node 1** sends LED number every 1 s via Data Frame 
- **Node 2** toggles the corresponding LED on reception 
- **Node 1** sends Remote Frame every 4 s requesting 2 bytes of data 
- **Node 2** responds to Remote Frame with Data Frame 
- Fully **interrupt-driven** code (TX/RX callbacks) 
- UART debug output for monitoring CAN activity 
- Optional logic analyzer capture to verify timing 
 
--- 
 
## 📡 CAN Frame Format 
 
| Frame Type         | CAN ID  | DLC     | Direction     | Payload Example | Purpose                               | 
| ------------------ | ------- | ------- | ------------- | --------------- | ------------------------------------- | 
| **LED Command**    | `0x65D` | 1       | Node1 ➜ Node2 | `02`            | Turns on LED #2 on Node2              | 
| **Remote Request** | `0x651` | 2 (RTR) | Node1 ➜ Node2 | –               | Asks for 2 bytes of data              | 
| **Remote Reply**   | `0x651` | 2       | Node2 ➜ Node1 | `01 2C`         | Replies with 16-bit value (MSB first) | 
 
---  
 
## 🔧 Hardware Connections 
 
| **Signal** | **Node 1 Pin** | **Node 2 Pin** | 
|----------- |----------------|----------------| 
| CAN_RX     | PA11           | PB8            | 
| CAN_TX     | PA12           | PB9            | 
| GND        | GND            | GND            | 
 
--- 
 
## 🔧 Hardware Setup 
 
Below is the actual setup used for this project: 
 
![can-hw-setup](https://github.com/user-attachments/assets/a69f4582-5cf1-4068-8f07-1b2a2d465575) 
 
 
--- 



 
