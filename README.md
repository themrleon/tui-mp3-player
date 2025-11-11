# What is this ?
This is a very simple TUI MP3 player using `mpg123` under the hood:

![demo](https://github.com/user-attachments/assets/beb6f021-f617-48c3-95d1-bb117f57e458)

# What is ncurses, mpg123 and TUI ?
> TUI (Terminal User Interface) is app-like experiences via menus inside the terminal, like the `htop` tool.  
> Ncurses is the essential library used to build TUIs, it's almost always pre-installed as it's a core dependency for many system tools.  
> mpg123 is a classic MP3 player for the terminal.

# Why ?
Existing tools are too complicated to compile for embedded systems, all I needed was a simple, no dependencies TUI mp3 player, but none of the existing tools I tried had binaries for my raspberry pi 32 bit ARMv6, and compilling them from source required lots of odd libraries with different versions from the ones my OS had, things got very time consumming and got nowhere, that is why I made this.

# How the tool works ?
This is only a terminal UI using the famous `ncurses` library, that is the only dependency to compile this tool from scratch, `ncurses` is available for anything and likely already in your OS, making this tool ideal if you just want a simple way to play mp3 files with some dignity in the console without haveing to manually call `mpg123` with the mp3 file names. The tool will show a list of all MP3 files in the current directory, and will use `mpg123` to play them once you select one.

# How to compile and run ?
1. Install the `ncurses` development library, ex: `sudo apt install libncurses-dev`
2. Install or get the `mpg123` binary from somewhere, ex: `sudo apt install mpg123`
3. Run `make` and `sudo make install`
4. Go to a folder that has mp3 files and run it `tuimp3`

> [NOTE]
> The tool supports two parameters, text and background color, numbers 0 to 7, ex: `./tuimp3 0 1`
