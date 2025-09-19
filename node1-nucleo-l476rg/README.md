⚙️ Board-Specific Notes 
 
- Board: ST NUCLEO-L476RG 
- Clock Source (HSE): 
  By default, this board does not have an external crystal. 
  I soldered the following bridges on the bottom side of the board: 
  - MCO → HSE: SB16 
  - Oscillator bypass: SB50 
    This routes the 8 MHz ST-LINK oscillator into the STM32 as HSE input. 
 
📸 See image below: 
![bottom_side_of_the_nucleo-l476rg](https://github.com/user-attachments/assets/16d87dc7-210a-41f4-98f4-c87de2fd7547) 

 
Effect in Code: 
Without these solder bridges, the code will not run because 
SystemClock_Config() in main.c assumes HSE (8 MHz bypass) → PLL → SYSCLK. 
If you use another Nucleo or custom board, you may need to adapt the clock settings. 

--- 
 
🛠️ Features of Node 1 Firmware 
 
- Sends LED command frame every 1 second (1 byte payload). 
 
- Sends a Remote frame every 4 seconds (requests 2-byte response). 
 
- Receives Node 2 response and prints debug logs over UART2 (115200 baud). 
 
- LED on PA5 toggles on every CAN TX. 

--- 
 
💻 Build & Flash 
 
- Open the project in STM32CubeIDE 
 
- Build → Debug / Run to flash firmware via ST-LINK 
 
- Monitor logs using UART2 (/dev/ttyACM0 or /dev/ttyACM1) 
 
  sudo minicom -D /dev/ttyACM0 -b 115200 
 
---








