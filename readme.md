### 1, Product picture

<img width="600" height="600" alt="image" src="https://github.com/user-attachments/assets/9fc1b1d5-ed19-42ce-98d4-28864a6b0dad" />


### 2, Product version number

|      | Hardware | Software | Remark |
| ---- | -------- | -------- | ------ |
| 1    | V1.0     | V1.0     | latest |

### 3, product information

#### Display Module Specifications
| Main Chip: ESP32-S3R8  |                                                              |
| ---------------------- | ------------------------------------------------------------ |
| Processor              | Equipped with high-performance Xtensa 32-bit LX7 dual-core processor, with a main frequency of up to 240MHz |
| System memory          | 512KB SRAM、8M PSRAM                                         |
| Storage                | 16M Flash                                                    |
| Screen                 |                                                              |
| Size                   | 1.46 inch                                                    |
| Screen Type            | IPS                                                          |
| Touch Type             | Capacitive Touch                                             |
| Resolution             | 360*360                                                      |
| Wireless Communication |                                                              |
| Bluetooth              | Bluetooth Low Energy and Bluetooth 5.0                       |
| WiFi                   | Support 802.11a/b/g/n，2.4GH                                 |
| Hardware               |                                                              |
| UART Interface         | 2x UART, ZX-MX 1.25-4P                                       |
| I2C interface          | ZX-MX 1.25-4P                                                |
| FPC connector          | 12P, Power supply burning port, Pitch: 0.5 mm.               |
| Button                 | RESET button, BOOT button, confirmation button (knob press switch) |
| LED Light              | Power indicator, LED ambient light                           |
| Other                  |                                                              |
| Power Input            | 5V/1A                                                        |
| Operating temperature  | -20~65℃                                                      |
| Storage temperature    | -40~80℃                                                      |
| Operation Power        | Module：DC5V  Main Chip：3.3V                                |
| Size                   | 60\*60*27.5mm                                                |
| Shell                  | Plastic + acrylic                                            |
| Net Weight             | 60g                                                          |



### 4, Use the driver module

| Name | dependency library |
| ---- | ------------------ |
| LVGL | lvgl/lvgl@8.3.11   |

### 5,Quick Start

##### Arduino IDE starts

1.Download the library files used by this product to the 'libraries' folder.

C:\Users\Documents\Arduino\libraries\

![2](https://github.com/user-attachments/assets/86c568bb-3921-4a07-ae91-62d7ce752e50)



2.Open the Arduino IDE

<img width="534" height="569" alt="image" src="https://github.com/user-attachments/assets/f7b3a739-1f1e-4a25-a3cc-a5a2cdc892a4" />


3.Open the code configuration environment and burn it.

<img width="751" height="798" alt="image" src="https://github.com/user-attachments/assets/bad622bc-8a6d-4c3c-9ed1-343b9d9623fa" />


### 6,Folder structure.

|--3D file： Contains 3D model files (.stp) for the hardware. These files can be used for visualization, enclosure design, or integration into CAD software.

|--Datasheet: Includes datasheets for components used in the project, providing detailed specifications, electrical characteristics, and pin configurations.

|--Eagle_SCH&PCB: Contains **Eagle CAD** schematic (`.sch`) and PCB layout (`.brd`) files. These are used for circuit design and PCB manufacturing.

|--example: Provides example code and projects to demonstrate how to use the hardware and libraries. These examples help users get started quickly.

|--factory_firmware: Stores pre-compiled factory firmware that can be directly flashed onto the device. This ensures the device runs the default functionality.

|--factory_sourcecode: Contains the source code for the factory firmware, allowing users to modify and rebuild the firmware as needed.

### 7,Pin definition

| Function       | GPIO   |
| -------------- | ------ |
| LCD SCLK       | GPIO10 |
| LCD MOSI       | GPIO11 |
| LCD CS         | GPIO9  |
| LCD DC         | GPIO3  |
| LCD RST        | GPIO14 |
| Touch SDA      | GPIO6  |
| Touch SCL      | GPIO7  |
| Touch INT      | GPIO5  |
| Touch RST      | GPIO13 |
| Backlight      | GPIO46 |
| Encoder A      | GPIO45 |
| Encoder B      | GPIO42 |
| Encoder Switch | GPIO41 |
| Power LED      | GPIO40 |
| RGB LED        | GPIO48 |
| I2C_SDA        | GPIO38 |
| I2C_SCL        | GPIO39 |
