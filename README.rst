Project Blinkenlights -- RGB LED Strip Controller
=================================================

**Project Blinkenlights** is a RGB LED strip controller. This repository
includes its hardware and firmware.

.. image:: prjblinkenlights/blob/master/doc/PrjBlinkenlights.360x480.jpg
   :align: right
   :target: prjblinkenlights/blob/master/doc/PrjBlinkenlights.2112x2816.jpg


License
-------

    Copyright (C) 2012 Johann Glaser <Johann.Glaser@gmx.at>

    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or  
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

The repository includes several datasheets. Their copyright belongs to their
respective owners and are only replicated here for easier access.

The name **Project Blinkenlights** is used by numerous other organizations,
products and initiatives. This RGB LED strip controller does not have to do
anything with them. No rights on the name are requested.


Language
--------

**Project Blinkenlights** is developed for German speaking users. Therefore
its user interface and its `user manual
<prjblinkenlights/blob/master/doc/Betriebsanleitung.pdf>`_ use the German
language. Nevertheless, all firmware source files and all hardware design
files are written in English.


Directory Structure
-------------------

  ``doc/``
    Documentation

  ``eagle/``
    Hardware design files

  ``datasheets/``
    Datasheets of the components used in the hardware design

  ``workspace/``
    Eclipse workspace with numerous testing projects

  ``workspace/PrjBlinkenlights/``
    The main Eclipse project with the full firmware


Hardware
--------

The hardware of **Project Blinkenlights** uses a TI LaunchPad MSP-EXP430 Rev.
1.5. A custom BoosterPack (see its `design files
<prjblinkenlights/tree/master/eagle/>`_) provides the facilities to control
the device, switch the LEDs, and hold an LCD display.

The LaunchPad holds an MSP430G2553 microcontroller.


Build
-----

The MSP430 microcontroller is programmed using ``msp430-gcc`` and
``mspdebug``. For details on the tool installation in Debian GNU/Linux see
my `blog post
<http://johann-glaser.blogspot.co.at/2012/10/msp430-launchpad-with-debian.html>`_.

An Eclipse workspace was created and used seamlessly with these tools for
compiling and debugging. More details to follow.


TODO
----

 - Firmware: update documentation in main.c
 - BoosterPack: change layer of wires to the LaunchPad connectors from bottom
   to top
 - Documentation: how to use Eclipse with MSP430
 - Documentation: design decisions, ideas, ... (see Zim)
