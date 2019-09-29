<p align="center">
<img src="https://flavio.tordini.org/files/products/minitube.png">
</p>

# Minitube
Minitube is a YouTube desktop application. It is written in C++ using the Qt framework. Contributing is welcome, especially in the Linux desktop integration area.

## Translating to your language
Translations are done at https://www.transifex.com/flaviotordini/minitube/
Just register and apply for a language team. Please don't request translation merges on GitHub.

## Google API Key
Google is now requiring an API key in order to access YouTube Data web services.
Create a "Browser Key" at https://console.developers.google.com and enable the Youtube Data API.

The key must be specified at compile time as shown below.
Alternatively Minitube can read an API key from the GOOGLE_API_KEY environment variable.

## Build instructions
Clone from Github:

    git clone --recursive https://github.com/flaviotordini/minitube.git

You need Qt >= 5.10 and MPV >= 0.29.0. The following Qt modules are needed: core, gui, widgets, network, sql (using the Sqlite plugin), declarative, dbus, x11extras.

To be able to build on a Debian (or derivative) system:

    sudo apt install build-essential qt5-default qttools5-dev-tools qt5-qmake qtdeclarative5-dev libqt5sql5-sqlite libqt5x11extras5-dev libmpv-dev

Compiling:

    qmake "DEFINES += APP_GOOGLE_API_KEY=YourAPIKeyHere"
    make

Running:

    build/target/minitube

Installing on Linux:

This is for packagers. End users should not install applications in this way.

    sudo make install

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
