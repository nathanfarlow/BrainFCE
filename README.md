# BrainfuckCE
A Brainfuck interpreter and native compiler written in C for the TI 84 PCE calculators. The bytecode compiler and interpreter work on any system. The native code only compiles to ez80.

Huge thanks to MateoConLechuga for the toolchain and all the work on the CE calculator. Thanks also to Iambian over on the cemetech irc.

#### A screenshot of the gui
![gui](https://github.com/nathanfarlow/BrainfuckCE/blob/master/img/screenshot.png)

#### Running and viewing fractal program natively
(The flashing is not as bad when run on an actual calculator)

![native code](https://github.com/nathanfarlow/BrainfuckCE/blob/master/img/fractal_native.gif)

#### Running the xmas tree program with bytecode
(The flashing is not as bad when run on an actual calculator)

![bytecode](https://github.com/nathanfarlow/BrainfuckCE/blob/master/img/xmas_bytecode.gif)


## How to build from source
* Download and install https://github.com/CE-Programming/toolchain/releases
* Download or clone this repository
* Navigate to the BrainfuckCE directory in a command prompt/terminal
* Type 'make' without quotes.
* BRAINF.8xp will be in the bin directory.

## How to run
* Download the latest release at https://github.com/nathanfarlow/BrainfuckCE/releases
* Send BRAINF.8xp to your calculator.
* Download libraries from https://github.com/CE-Programming/libraries/releases and send them to your calculator.
* Run BrainfuckCE with Asm(prgmBRAINF
* Use arrow keys and select your program and either run with bytecode or native.

## How to create brainfuck programs
I've already packed the fractal, hello world, and xmas programs as .8xp files in the releases. You can just send these to your calculator and run with BrainfuckCE.

#### Option 1 - Create your own on the calculator
* Create a TI BASIC program on your calculator like usual.
* Fill it with brainfuck instead.
* Open BrainfuckCE and select the program and run.

#### Option 2 - Copy and paste from the internet
* Download and install TI-Connect CE.
* Click Program Editor
* Click New Program
* Paste brainfuck code
* Change VAR NAME
* Ctrl+s and save as a file
* Send file to calculator
* Open BrainfuckCE
* Select and run program

## "Documentation"
Bytecode with the 'mode to stop bytecode' option enabled is recommended until your program is perfected. This prevents infinite loops in case your brainfuck code has a bug and loops infinitely. Simply press the mode button to stop the bytecode execution as long as the checkbox is selected.

Once your program is perfected you can run it natively. There will be no interrupts, so if you want to stop the execution you will have to clear your ram... (I take no responsibility) Native execution runs exponentially faster than bytecode on brainfuck programs with lots of calculations such as the fractal program. The fractal program takes ~60 minutes to complete on a normal calculator when using optimized native.

The optimize checkbox is recommended and will increase speeds with both the bytecode and native as well as shrink the produced code without changing any of the code's original functionality.

You can input up to 13 characters in the input box; press alpha to access letters. Press delete to delete one
character or clear to clear the whole textbox.

Press the mode button to quit the application. Press the clear button to exit the console when the brainfuck program has finished.

BrainfuckCE can run both archived and unarhived programs without an issue.

## Technical docs
The max stack depth for leading opening brackets is 100.

The max program size is 12000 characters.

The max native instrucitons is 36000.

The max bytecode instructions is 9000.

You will get an error printed to the console if any of these are exceeded.

The console window has dimensions of 50x150
