# Time-Keeper-System-Utility-Program

Time Keeper is a System Utility program which allows end-users to obtain the execution statistics of a program or sequence of programs connected by pipes.

For example, to obtain the execution statistics of the program *firefox*, you enter the following command under the command prompt: 

```bash
./timekeeper firefox
```

After the firefox program terminated, timekeeper will display the firefox process's running statistics: 

```bash
The command "firefox" terminated with returned status code = 0
real: 37.21s, user: 4.13s, system: 0.44s, context switch: 20292
```