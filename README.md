# BBC Mirco USB Keyboard

A USB driver for the BBC Micro keyboard using an Arduino Pro Micro (AtMega32U4) written in Arduino using the built in USB Keyboard library. 
Mostly functional and definitely not practical. 

## Getting Started

Pull a copy, compile and copy using the Arduino IDE.

Pin mappings:

| BBC Pin       |1| 2| 3|4|5|6|7|8|9|10|11|12|13|14|15|16|
| --------------|:-------------:| :-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| Function	| Ground | Break | Clock | Row A | Row B | Row C | Col A | Col B | Col C | Col D | W | LED0 | CA2 | +5v | LED1 | LED2 |
| Arduino Pin   |GND|A3|9|6|7|8|5|4|3|2|A1|16|10|VCC|14|15|

Pin mappings for the BBC are from the top left to right, with the keyboard face up with the pins facing away.


## Contributing

Pull requests are more than welcome.


## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details