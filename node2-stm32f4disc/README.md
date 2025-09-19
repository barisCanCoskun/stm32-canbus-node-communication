Node 2 – STM32F4DISCOVERY(CAN Slave) 
 
This project configures the STM32F407VG Discovery board as the CAN Slave node. 
It receives LED commands from Node 1 and blinks the corresponding LEDs, while also responding to remote frame requests with 2-byte data packets. 

--- 

⚙️ Project Configuration 

When creating the project in STM32CubeIDE, all default peripheral pinouts were cleared to simplify the setup. Only the necessary features were added: 

- Serial Wire (SWD) → for programming/debugging 
 
- HSE Clock (Crystal) → external oscillator 
 
- CAN1 → using pins PB8 (CAN_RX) and PB9 (CAN_TX) 
 
- USART2 → for UART debug messages 
 
- Pushbutton → configured as external interrupt 
 
- Clock Config → APB1 set to 42 MHz (required for CAN bitrate) 

--- 

🔌 UART Debug Connection (VCP) 
 
The STM32F4DISCOVERY board’s ST-LINK/V2-A provides a Virtual COM Port (VCP). 
To connect it to USART2 on the MCU, flying wires were used: 
 
Signal	ST-LINK/V2-A Pin (U2)	STM32F407 Pin	Header Pin 
VCP_TX	Pin 12	PA3 (USART2_RX)	P1.13 
VCP_RX	Pin 13	PA2 (USART2_TX)	P1.14 
 
💡 Reference: UM1472 – Discovery kit with STM32F407VG MCU 
A screenshot from the manual describing this process is included below. 
 
<img width="784" height="799" alt="ST-LINK_VCP_connection_to_USART2" src="https://github.com/user-attachments/assets/3a2c9783-6519-4b59-a3a0-815fefb15238" /> 
 
--- 
 
🖼️ Hardware Setup 
 
- Connect CANH ↔ CANH and CANL ↔ CANL between Node 1 and Node 2 
 
- Common GND between both boards 
 
- UART connected to PC over VCP 
 
--- 
 
🛠️ Features 
 
- Receives LED number via CAN data frame and toggles corresponding LED 
 
- Responds to CAN remote frames with 2-byte data 
 
- Debug messages via USART2 (VCP → PC terminal) 
 
- Interrupt-driven CAN communication (no polling)
 
--- 
 
💻 Build & Flash 
 
- Open the project in STM32CubeIDE 
 
- Build → Debug / Run to flash firmware via ST-LINK 
 
- Monitor logs using UART2 (/dev/ttyACM1) 
 
    sudo minicom -D /dev/ttyACM1 -b 115200 
 
--- 



