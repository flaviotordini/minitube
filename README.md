# Minitube
Minitube is a YouTube desktop application. It is written in C++ using the Qt framework. Contributing is welcome, especially in the Linux desktop integration area.

## Traslating Minitube to your language
Minitube translations are done at https://www.transifex.com/projects/p/minitube/
Just register and apply for a language team. Please don't request translation merges on GitHub.

## Building
To compile Minitube you need at least Qt 4.8. The following Qt modules are needed:
core, gui, network, sql (using the Sqlite plugin), script, dbus, phonon

On a Debian or Ubuntu system, type:

    $ sudo apt-get install build-essential qt4-dev-tools libphonon-dev

### Google API Key

Google is now requiring an API key in order to access YouTube Data web services.
Create a "Browser Key" at https://console.developers.google.com

The key must be specified at compile time as shown below.
Alternatively Minitube can read an API key from the GOOGLE_API_KEY environment variable.

### Compiling
Run:

    $ qmake -DAPP_GOOGLE_API_KEY="YouAPIKeyHere"

and then:

    $ make

Beware of the Qt3 or Qt5 version of qmake! If things go wrong try running qmake-qt4 instead.

### Running

	$ ./build/target/minitube
	
## Installing on Linux

    $ sudo make install

This is for packagers. End users should not install applications in this way.

## A word about Phonon on Linux
To be able to actually watch videos you need a working Phonon setup.
Please don't contact me about this, ask for help on your distribution support channels.

These days Minitube is tested with the VLC backend only.
Please don't report bugs with other backends as they're not supported.

## Legal Stuff
Copyright (C) Flavio Tordini

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
