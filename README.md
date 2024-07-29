# TTDMonitor
![](https://github.com/deanOcoin/ttdmonitor/blob/main/ttdmonitor.png)
### What is TTD Client?
* TTD Client is a program that uses [VanitySearch](https://github.com/JeanLucPons/VanitySearch) to scan ranges of bitcoin private keys for the Bitcoin Puzzle Challenge and report the scanned ranges to the [TTD collective pool](http://www.ttdsales.com/66bit/index.php).
* TTD Client creates an instance of VanitySearch as a child process for each range that it needs to check.

### What is the purpose of TTDMonitor?
* Although VanitySearch will output its key rate to the console, I wanted to be able to visually monitor the key rate in order to better determine how overclock settings, or other system interruptions, will affect my graphic card's key rate.

### How does it work?
* TTDMonitor is written using both C++ and Python. The backend component is written in C++ and the frontend (GUI) component is written in Python. TTDMonitor is **ONLY** for 64-bit Windows machines!
* The C++ component tracks the parent process, TTD Client, and its child processes. It iterates through child processes and finds the VanitySearch process. Once VanitySearch is found, we find its main thread and save the thread context in order to read the data we need.
* The C++ component communicates to the Python GUI using a local socket. This means they are two separate processes! Think of the C++ component as the server and the Python component as the client.
* The GUI is written using Tkinter and Matplotlib in Python 3.10, which displays a key rate vs time graph along with other information about the current range being scanned.

## TTDMonitor is VERY buggy! Please report any bugs in the issues section of its repository so the software can be improved.
