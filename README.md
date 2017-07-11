# Code for EIC workshop attestation
### General description 
This code is used to attest the Main Board and all the IO Boards of the TL 500 RTU ``one  at a time``. This work differs from our SmartGrid Comm submission where we attest all the boards simulataneously. The point of interest in this code is the simulator. This is the first work where we entirely simulate the checksum operation  on the client PC. The code of the respective boards is very slightly different from the SmartGrid Comm submission and hence will not be discussed much. On the other hand we shall discuss the RTU simulator.

![Attestation Console](images/attest_console.PNG?raw=true "Attestation Console")

### Simulator Project 

We have implemented the simulator as a Netbeans project and it written entirely in ``C``. It can be loaded into the Netbeans IDE or any ``C`` supported IDE.

``main.c`` 
The main file which displays the attestation console from where the respective boards to be attested can be selected. The program communicates to a RTU running the attestation service on port 2406. The IP address of the RTU is hardcoded and may need to be changed accordingly. 

``Utils.c``
This file contains some common utility functions. Some important utility functions simulate the after effects of performing a ARM ADD or SUB operation. These ARM instructions result in updation various kinds of flags (Zero, Negative, Carry, etc). These flag registers in turn control the ARM ``CPSR (Current Program Status Register) `` and hence must be updated correctly.
	
``Simulate_MainBoard.c``
This files simulates the main board for the TL 500 RTU. The main board is similar to the TL 100 board. The following values are hardcoded in the code and must be changed to accurately reflect the environment. 
* Program Counter - register r15 
* Random Seed to begin attestation - register r0 
* Status register - register cpsr 
* Base-address of the RAM/ROM for determining the mask address

All of the above are denoted by ``IMP`` in the code. The way to obtain the correct starting values for the ``seed`` and ``status register`` is to use the KEIL IDE in the debug mode and put a breakpoint just before you begin the attestation loop (for example in file checksum.s place a breakpoint at identifier ``Loop`` or the first MUL instruction). 

![Determine PC & CPSR ](images/breakpoint.PNG?raw=true "Determine PC & CPSR")

Once the breakpoint is hit note down the values in the program counter (register r15) , current program status register (register cpsr) and use these values at the simulator side as well. The same process needs to be repeated for the IO Boards as well. This needs to be done only once unless the code in the IO Board or main board is re-flashed/re-programmed in which case these register values will change (particularly the PC register).

The base address to be used for the RAM/ROM is available in the processors memory sheet/chip manual. Another way to get the same information is to use KEIL and note down the RAM and ROM addresses show in the ``Target`` tab.

![Memory Map of IO Board](images/memory_io.PNG?raw=true "Memory Map of IO Board")

![Memory Map of Main Board  ](images/memory_main.PNG?raw=true "Memory Map of Main Board ")

``Simulate_A.c, Simulate_B.c, Simulate_C.c, Simulate_D.c``
These files simulate the various IO Boards connected to the TL 500 board. For accurately simulating these boards we need the same information as that for the main board. Follow the procedures discussed above for each of the IO boards to determine the PC, cpsr, memory masks, etc and update the same in the simulator.

In order to accurately simulate the various boards we also need the memory dump on these devives so that we can do a memory walk. Before we discuss more about the memory dump files we need to understand the memory dump format.

### Memory dump format
In the EIC workshop codes we perform the memory walk over the entire RAM and no other region of the memory. We use the KEIL IDE to dump the memory into a file and then use the file at the simulator to do the memory walk. We use the [ SAVE command ](http://www.keil.com/support/man/docs/uv3/uv3_cm_save.htm)  to take the memory dump in the KEIL IDE console. Before we begin the attestation process we randomize the unused regions of the memory such as the stack and heap. In other words out of the approximately 2KB that our attestation code occupies inside the RAM the remaining memory (16-2=14KB) is filled with a random value sent by the simulator. Hence the memory dump should be taken after this operation has been performed otherwise the memory map at the simulator will not match. To take the dump we put a breakpoint at the begining of the attestation loop (same as when we determined the PC, cpsr, etc) and use the SAVE command to create the dump.

![Memory dump from KEIL IDE console](images/dump_memory.PNG?raw=true "Memory dump from KEIL IDE console")

For Main board the addresses execute the following commnad from the KIEL console ``save dump 0x40000000,0x40003FFF``

The memory dump is created in the [Intex Hex format] (http://www.keil.com/support/docs/1584.htm) where a Data record as ``:10246200464C5549442050524F46494C4500464C33`` is interpeted as 

* 10 is the number of data bytes in the record.
* 2462 is the address where the data are to be located in memory.
* 00 is the record type 00 (a data record).
* 464C...464C is the data.
* 33 is the checksum of the record.

We strip of everything except the ``data`` field from the record. We do this manually (and should ideally be automated in the future) using Notepad++ and feed the resulting file into the simulator (See sample ram_log files on github). An important thing to note is that the middle part of the log file should be filled with the same random value used to initialize the memory regions as the first step of attestation. These modified log files can now be fed to the simulator. 

``Note`` : The drawback of the above approach is that if the random value used to initialize the memory changes then the memory dump will need to be re-taken. This may not be very convenient and is not advisable. However it should be trivial to take the memory dump once at the factory and then change the file to randomize its data regions. This has not been implemented but should be easy to do. 

### What happens if the output from the Simulator and the RTU do not match? 
If the results dont match re-check if the values of the PC, cpsr & random values used to start the attestation process match. If all the above are correct then it his highly likely that the memory dump is incorrect. Take the dump again. The simulation of the ARM instructions have been tested vigoruosly many times and hence there is unlikely to have a bug in the instruction simulation. Most of the times the memory dump is incorrect. A good way to debug would be to start the attestation loop on the simulator and RTU side by side in the debug mode. Observe the change in the register values on both sides at each step since these should match. 

