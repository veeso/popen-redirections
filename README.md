# Popen Stdin/Stdout Redirections

Developed by Christian Visintin <cvisintin@solari.it>

## Introduction

This repository is a simple C application which starts a child through fork and set the stdout and the stdin of the child to two named pipes.
The program takes the user input and writes it to the child stdin, adding at the end "echo '//EOF';". Then it also waits for stdout of the child process and prints the output.

### Okay, but why

I made this project while trying to find a solution to implement that other project of mine <https://github.com/ChristianVisintin/Pyc>. I got stuck understanding how to make my program to read the user input and convert it to latin and to convert the shell output to cyrillic.

I just think this project may be useful to devs looking for redirections with pipes in C.

## License

```txt
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
```
