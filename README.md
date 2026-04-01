# ৳ Monthly Hisab App
> **A Professional-Grade Personal Finance Tracker built with C**

The **Monthly Hisab App** is a lightweight, high-performance terminal tool designed to manage personal income and expenses. It provides a crisp, ANSI-colored interface that makes financial tracking both efficient and visually satisfying.

---

## 🚀 Features
- **Modern Terminal UI**: High-fidelity box-drawing characters and ANSI escape sequences for a premium experience.
- **Dynamic Memory Management**: Utilizes `Linked Lists` for efficient, real-time data handling.
- **Binary Data Persistence**: Automatically saves and loads records from `.bin` files to ensure data is never lost.
- **Windows UTF-8 Optimized**: Fully compatible with modern Windows terminals for perfect rendering of the Taka (`৳`) symbol.
- **Snapshot Summary**: Instant view of total income, expenses, and net balance on each launch.

---

## 🧠 Application Logic
The application is built on a modular architecture to ensure scalability and reliability:

### 1. Data Structure
The core tracking logic uses a `Node` structure:
```c
typedef struct node {
    char   date[M];      // Transaction date
    double amount;       // Currency amount
    char   category[N];  // e.g. Salary, Rent, Food
    struct node *next;   // Pointer for Linked List
} Node;
```

### 2. Synchronization Loop
- **Load Phase**: On startup, the app scans `Record.bin`, `myincome.bin`, and `myexpense.bin` to reconstruct the financial state in memory using linked lists.
- **User Action**: When a user inputs data, the app dynamically appends a new node to the corresponding list and increments the global balance.
- **Persistence Phase**: Every transaction is instantly synchronized to the binary storage files to prevent data loss in case of crashes or power failures.

---

## 🎯 Expected Outcome
Using this application allows users to:
1. **Gain Financial Clarity**: Visualize exactly where money is going with categorised records.
2. **Automated Calculations**: Eliminate manual math for income vs. expense balancing.
3. **Data Portability**: Keep long-term records in compact binary files that can be easily backed up.

---

## 🛠️ How to Build & Run

### Prerequisites
- **GCC Compiler** (MinGW for Windows)
- **Terminal Emulator** (cmd, PowerShell, or Windows Terminal)

### Compilation
Open your terminal in the project directory and run:
```bash
gcc main.c -lncurses -o hisab_tracker
```
*(On Windows, just `gcc main.c -o hisab_tracker` is sufficient as it uses standard ANSI codes).*

### Usage
```bash
./hisab_tracker
```

---

## 📅 Future Roadmap
- [ ] Visual trend analysis (Simple ASCII charts)
- [ ] PDF report generation
- [ ] Password-protected financial records

---
**Developed with ♥ for Financial Freedom.**