# nether

nether is a Digital Logic simulator that was started as an educational project geared torwards learning how computational devices such as CPUs and GPUs work.

Currently, it features a primitive Digital Logic simulator, which works with a set of wires, `NAND` gates and `TriState` buffers. From this, other logics gates such as `AND`, `NOT`, and `XOR` can built, opening up the path for more complicated components such as adders, latches and flip flops.

This project is currently in its infancy, so there isn't a specific goal at the moment. However, the general direction is to work torwards a more realistic simulation of semiconductor devices (modeling the voltage and current relationship of MOSFETs or FinFETs) along with building more complex devices that are close to how modern CPUs and GPUs work.

**Please note that there is only Windows support at the moment. However, it is entirely possible to write a simple set of implementations for platform functions (`Print`, `Println`, ...) for other operating systems**

# Build instructions

Prerequisites:
+ Windows
+ Visual Studio 2019 version 16.8 or higher

Steps:
+ Open up `Developer Command Prompt for VS2019` or higher.
+ Navigate to the repository directory where `build.bat` is contained.
+ Run the script `build.bat`, and expect the following output:
```
> build
win32_nether.c

>
```

Upon build completion, you can use `run.bat` to run the project if you
don't want to run the executable directly. Running the project as is will
run a series of tests for a variety of logic components.

**Please note that executing from a debugger is highly recomennded as it allows you to place a debug break and watch the internal wire states.**

```
> run
[TriState]:         [SUCCESS]

[NAND]:             [SUCCESS]
[AND]:              [SUCCESS]
[OR]:               [SUCCESS]
[NOR]:              [SUCCESS]
[XOR]:              [SUCCESS]
[NOT]:              [SUCCESS]

[HalfAdder1]:       [SUCCESS]
[FullAdder1]:       [SUCCESS]
[HalfAdder]:        [SUCCESS]
[FullAdder]:        [SUCCESS]

[DLatch]:           [SUCCESS]
[DFlipFlop]:        [SUCCESS]
[Register]:         [SUCCESS]

>
```

# Code layout

The bread and butter of this project are contained within three files:
+ `nether_logic.h` - The interface of the digital logic simulator.
+ `nether_logic.c` - The implementation of the digital logic simulator.
+ `nether.c` - Contains platform-agnostic code such as the `Main` function and is also responsible for calling the digital logic simulator.

Platform code:
+ `nether_platform.h`- Declarations for platform functions such as `Print` and `Println`.

+ **Windows**:
    + `win32_platform.c` - Contains Win32 implementations of the platform functions present in `nether_platform.h`.
    + `win32_nether.c` - The file that gets compiled for a Windows executable, contains the entry point `WinMainCRTStartup` and is also responsible for calling `Main`.

# Miscellaneous

Ideas are contained within `sketch.txt`. Currently, it contains ideas for the architecture of a simple 8-bit CPU with `LOAD`, `STORE`, `ADD`, and `SUB` operation.
