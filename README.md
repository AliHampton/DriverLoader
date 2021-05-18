# DriverLoader
 Loads a driver from disk without signing required by using an exploited intel diagnostic driver. A Driver is manually mapped into memory with relocations and imports processed, and then the entry point is called. Driver must NOT use FxDriverEntry, must be changed to "DriverEntry" in Linker -> Advanced -> Entry Point. 

The Intel diagnostic driver "iqvw64e.sys" is well-known within the reverse engineering community as it provides completely unchecked access to physical memory writing. This memory writing privilege can be exploited to also allow for kernel function calls since you can "hook" another kernel function by inserting a jump instruction that points to another function. Therefore giving us all we need to manually map any arbitrary code into the kernel level and execute it. 

## Usage:
In its current state, the executable program can be supplied a driver by dragging a sys file on top of the executable. Although the code can easily be modified to hard code a particular path to be mapped. 