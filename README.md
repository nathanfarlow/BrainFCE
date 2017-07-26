# BrainfuckCE
A Brainfuck interpreter and native compiler written in C for the TI 84 PCE calculators. The bytecode compiler and interpreter work on any system. The native code only compiles to ez80. Gui/editor coming soon!


## How to build
* Download and install https://github.com/CE-Programming/toolchain/releases
* Navigate to BrainfuckCE directory in a command prompt/terminal
* Type 'make' without quotes.
* You will need to download [these libraries](https://github.com/CE-Programming/libraries/releases/latest) and transfer them to your calculator.

Huge thanks to MateoConLechuga for the toolchain and all the work on the CE calculator. Thanks also to Iambian over on the cemetech irc. He, Mateo, and many others answered tons of questions and helped me to make this :)


### Running fibonacci with unoptimized bytecode interpreter
![unoptimized bytecode](https://github.com/nathanfarlow/BrainfuckCE/blob/master/img/unoptimized_bytecode.gif)

### Running fibonacci with optimized bytecode interpreter
![optimized bytecode](https://github.com/nathanfarlow/BrainfuckCE/blob/master/img/optimized_bytecode.gif)

### Running fibonacci with optimized native code
![native code](https://github.com/nathanfarlow/BrainfuckCE/blob/master/img/native_ver_2.gif)

### Running the xmas tree program with 10 as input
![xmas](https://github.com/nathanfarlow/BrainfuckCE/blob/master/img/xmas.png)
