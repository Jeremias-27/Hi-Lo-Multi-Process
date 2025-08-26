Hi-Lo Multi-Process Game
📌 Overview

This project implements a Hi-Lo number guessing game using UNIX processes. The game demonstrates concurrent gameplay by spawning multiple child processes that interact with a shared game state through file-based inter-process communication (IPC) and signals for synchronization.

This project reinforces core systems programming concepts, including:

Process creation and management

Signal handling

IPC via file I/O

Resource cleanup and synchronization

🛠 Features

Multi-process architecture using fork()

Signal-based communication for process synchronization

File I/O for shared state between processes

Configurable guessing range and difficulty level

⚙️ Technologies

Language: C

Concurrency: UNIX Processes

IPC: Signals, file I/O

Platform: Linux / UNIX-like systems

📂 Structure
├── hi_lo_process.c
├── Makefile
└── README.md

🚀 Getting Started
✅ Prerequisites

GCC compiler

Linux/UNIX environment

🔧 Build and Run
git clone https://github.com/your-username/hi-lo-multi-process-game.git
cd hi-lo-multi-process-game
make
./hi_lo_process

📚 Learning Outcomes

Understanding process-based concurrency

Implementing IPC with signals and files

Handling synchronization in multi-process systems
