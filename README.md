# backman
A simple utility that changes your background, that also likes to segfault.
You can install it by `git clone`ing this repo to some location, then run `chmod +x install.sh && install.sh` to install it, assuming you have GCC, X11 and Imlib2 installed.

## Why does it like to segfault?
Because I was the one writing the code.

## How do I use it?
For now, the only image mode is `stretch`, but I may add more options in the future, using the power of pull requests.
For usage, you can simply input `backman` into the terminal.

## How do I remove it?
Remove the `backman` symlink in `/bin`, then remove the source folder (the folder you cloned the repository into).