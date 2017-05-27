# Minitube
Minitube is a YouTube desktop application. It is written in C++ using the Qt framework. Contributing is welcome, especially in the Linux desktop integration area.

## Translating Minitube to your language
Translations are done at https://www.transifex.com/projects/p/minitube/
Just register and apply for a language team. Please don't request translation merges on GitHub.

## Google API Key
Google is now requiring an API key in order to access YouTube Data web services.
Create a "Browser Key" at https://console.developers.google.com

The key must be specified at compile time as shown below.
Alternatively Minitube can read an API key from the GOOGLE_API_KEY environment variable.

Not specifying an API key will result in Minitube showing an Google API error 'Error 403: Forbidden'

### API Key on Linux
To add or change the Google API key for an existing install of Minitube, add your API key to the file: /etc/profile.d/minitube.sh.
This file may or may not already exist, depending on the package you downloaded.
If it does not exist, create the file, make it executable (using chmod +x minitube.sh) and add the line:

    export GOOGLE_API_KEY="[YOUR KEY HERE]"
    
You will need to log out, and log back in again before Minitube will see the new API Key.

## Build instructions
To compile Minitube you need at least Qt 5.0. The following Qt modules are needed: core, gui, widgets, network, sql (using the Sqlite plugin), declarative, dbus.

To be able to build on a Debian (or derivative) system:

    $ sudo apt-get install build-essential qttools5-dev-tools qt5-qmake  qtdeclarative5-dev libphonon4qt5-dev libqt5sql5-sqlite qt5-default

Compiling:

    $ qmake "DEFINES += APP_GOOGLE_API_KEY=YourAPIKeyHere"
    $ make

Beware of the Qt 4 version of qmake!

Running:

	$ build/target/minitube
	
Installing on Linux:

    $ sudo make install

This is for packagers. End users should not install applications in this way.

## A word about Phonon on Linux
To be able to actually watch videos you need a working Phonon setup.
Please don't report bugs about this, ask for help on your distribution support channels.

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
