# BW - Isolation layer for C++ GUI Applications

This is a (now) obsolete reverse dependency layer for cross platform graphical
applications in C++.

It was use to make a few small applications and some web servers.


It has some still useful code that can be extracted into single file libraries:
- An XML parser
- An .ini file read/write with localization string support
- A SQL layer built on top of ODBC

It has it's own assertions and error hierarchy, that was used as the basis for
https://github.com/BrassHead/selftest

Enjoy, it's now available under the [MIT License](LICENSE-MIT.txt)


## MacOS prerequisites

- `brew install libx11`
- `brew install unixodbc`
- `brew install libxext`
- Install XQuartz from https://www.xquartz.org
