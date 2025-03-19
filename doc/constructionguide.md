#  LesScope
 Construction of LesScope Dual Channel Scope eurorack-modul.

**Table of Contents**

- [Preconditions](#preconditions)
- [Construction](#construction)
- [First Switchon](#firstswitchon)
- [License](#license)

## Preconditions<a name="preconditions"></a>

- The PCB-Board must be available.  
- All BOM-parts must be available for the PCB-Board (see BOM-file: [LesScope](./../hw/bom/LesScope.html)).
- The frontpanel is required for aligned soldering the encoder and all jacks and switches to the PCB-Board.

## Construction<a name="construction"></a>

### PCB-Board: LesScope 'Backplane'

>#### 'Backplane' back side MTH-parts (start soldering here) [back side parts](./pictures/LesScope_Bot.png)

> start soldering MTH-parts on back side of the Board.  

>- place all resistors to the back side and solder them on top side.
>- place all capacitors to the back side and solder them on top side.
>- Watch the rigth orientation on capacitor C3.
>- fit the two 15-position, 1 row pin-sockets to the back side and solder them at the top side.  
>- fit the 5-position, 2 rows pin-header J6 (eurorack power) to the back side. Watch the rigth orientation and solder it on top side.  
>- place one DIP14 socket at U1-position on back side. Watch the rigth orientation and solder it on top side.  

>#### 'Backplane' top side MTH-parts [top side parts](./pictures/LesScope_Top.png)

> continue soldering MTH-parts on top side of the Board.  

>- place all resistors to the top side and solder them on back side. Be aware some of them mounted vertically.  
>- place all capacitors to the top side and solder them on back side.  
>- place all diodes to the top side. Watch the rigth orientation and solder them on back side. Be aware some of them mounted vertically.  
>- fit one 7-position, 1 row pin-header (for U2 OLED) to the back side and solder it at the top side.  
>- the currently mounted 'LesScope'-backplane top side should look like: [LesScope backplane top side (rev.: 1.1)](./pictures/Backplane_top_parts_mounted.png) 

>- **All following parts have to be aligned to the frontpanel and not yet soldered **
>- fit that snapin-encoder SW1 on PCB top side.
>- fit that switches SW2 and SW3 on PCB top side. Must be ON-ON type with two positions.
>- fit all fife jacks on PCB top side.
>- place the **frontpanel** on top side parts and **align** them.
>- check alignment and distance to the frontpanel, see: [alignment](./pictures/LesScope_R_side.png)
>- use nuts for the jacks and switches to fix that frontpanel to the PCB-parts.
>- now solder all parts on back side and make sure everything is still aligned.
>- unmount that frontpanel from backplane again.
>- mount that 1.3'' OLED display with one 7-position, 1 row pin-socket to the U2 pin-header on backplane. Cut that sockets a bit to get the right distance to the frontpanel.
>- use two 5mm spacer with 3mm screws and plastic washer to fix that display on the backplane, see [mounted parts](./pictures/LesScope_L_side.png).
>- remount that frontpanel to the backplane again and use nuts for jacks and switches to fix it.


## First Switchon<a name="firstswitchon"></a>
 Preconditions:

- Push IC U1 (MCP6004) into the DIP-socket on the Backplane (watch the alignment of Pin1).
- Push IC A1 (Arduino Nano V3.x) into the pin-sockets on the Backplane (watch the alignment of Pin1).
- The frontpanel and all PCB-parts are mounted.
- connect one 10pin buscabel between eurorack-bus and the LesScope Backplane (watch the alignment of Pin1 := -12V).

 Action: Optical check

- Switch **on** the eurorack-case power.
- Watch the startup-display at the frontpanel, example see:  
  [startup-display](./pictures/LesScope_startup.png)

 Action: Measurements

- Switch **on** the eurorack-case power.
- Measure voltages at points:  

<table>
<tr>
    <th>LesScope Board</th>
    <th>measuring point</th>
    <th>measured voltages</th>
</tr>
<tr>
    <td>A1 Arduino Nano (back side)</td>
    <td>IC A1 Pin30</td>
    <td>+12 Volt</td>
</tr>
<tr>
    <td>A1 Arduino Nano (back side)</td>
    <td>IC A1 Pin27</td>
    <td>+5 Volt</td>
</tr>
<tr>
    <td>U1 MCP6004 (back side)</td>
    <td>IC U1 Pin4</td>
    <td>+5 Volt</td>
</tr>
</table>

 Result:

If any measurement is out of range check the board and repeat the required steps.

 Postconditions:

None

## License<a name="license"></a>
> Hardware:cc by-nc-sa 4.0  
> Software:licensed under GPL-3.0

