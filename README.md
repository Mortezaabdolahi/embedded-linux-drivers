# 🐧 Embedded Linux & POSIX C Practice Suite (Raspberry Pi 4)

A collection of user-space drivers, POSIX multithreading (pthreads), network socket programming, and hardware interfacing developed for Raspberry Pi 4 running custom Embedded Linux (Buildroot).

---

## 📂 Repository Structure

```text
embedded-linux-drivers-practice/
├── 01-i2c-drivers/         # I2C peripherals: AT24C32 EEPROM & DS1307 RTC user-space drivers
├── 02-spi-drivers/         # SPI hardware bring-up & ILI9341 display tests using libgpiod
├── 03-uart-thread-drivers/ # Multi-threaded POSIX UART communication (termios, pthreads)
├── 04-pwm-drivers/         # Linux sysfs PWM hardware control interface
└── 05-networking/          # Network socket programming (TCP client/server & raw ioctl setup)
```

---

## 🛠️ Modules Overview

1. **I2C Peripherals (`01-i2c-drivers/`)**
   - Modular user-space driver for AT24C32 with 16-bit addressing and the `i2c-dev` ioctl interface.
   - DS1307 real-time clock interface with BCD conversion and continuous time monitoring.

2. **SPI Display (`02-spi-drivers/`)**
   - SPI bus configuration through `/dev/spidev0.0`.
   - GPIO management using `libgpiod`.
   - Basic ILI9341 TFT display bring-up and communication tests.

3. **Concurrency & UART (`03-uart-thread-drivers/`)**
   - Asynchronous serial communication through `/dev/ttyAMA0`.
   - POSIX threads (`pthreads`) and `termios` configuration.
   - 8N1 serial format with a baud rate of 115200.

4. **PWM Control (`04-pwm-drivers/`)**
   - User-space PWM control through the Linux `sysfs` interface.
   - Configuration of PWM frequency and duty cycle using `pwmchip0`.

5. **Networking (`05-networking/`)**
   - TCP client and server implementations using POSIX sockets.
   - Low-level network interface configuration using Linux `ioctl`, including `SIOCSIFHWADDR`.

---

## 🧰 Technologies

- Embedded Linux (Buildroot, Raspberry Pi 4)
- POSIX C, Linux System Programming
- pthreads, termios, Sockets, ioctl, sysfs, libgpiod, i2c-dev, spidev

---

## 📫 Contact

- GitHub: https://github.com/Mortezaabdolahi
- Email: mortezaabdolahi007@gmail.com
