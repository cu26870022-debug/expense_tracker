# Smart Digital Expense Tracker with Budget Prediction

A menu-driven C++ console application that helps students record daily expenses, track spending by category, monitor a monthly budget and predict whether the budget will be exceeded.

**Course:** BCA I Semester (Session 2026-2027), COER University, Roorkee
**Lab Sheet:** 09
**Author:** *Your Name* | **Roll No:** *Your Roll Number*

## Features

| # | Module | What it does |
|---|--------|--------------|
| 1 | User Registration | Create an account (password stored as a hash, not plain text) |
| 2 | Login System | Log in with up to 3 attempts |
| 3 | Add Daily Expense | Date, category, description and amount, with input validation |
| 4 | View Expense History | Sort by date, amount, or order entered |
| 5 | Category-wise Expense | Totals and percentage share per category |
| 6 | Monthly Expense Report | Full report for any month, including the highest expense |
| 7 | Budget Setting | Set a monthly budget |
| 8 | Budget Warning | Warning at 80% and alert above 100% of the budget |
| 9 | Expense Prediction | Predicts month-end spending from the daily average |
| 10 | Search Expense | By keyword, exact date or amount range |
| 11 | Delete Expense | Delete by ID, with confirmation |
| 12 | File Saving | Data is saved to files and reloaded on login |

**Concepts used:** variables, loops, arrays, structures, functions, file handling, strings, searching (linear search) and sorting (insertion sort).

## How to Compile and Run

Requires any C++11 compiler (g++, MinGW, Visual Studio, Code::Blocks, Dev-C++).

```bash
g++ -o tracker main.cpp
./tracker          # on Windows: tracker.exe
```

## Usage

1. Choose **Register** and create a username and password.
2. Choose **Login**.
3. Use **Set Monthly Budget** first (option 5), then add expenses (option 1). A budget warning appears automatically after each expense.
4. Use options 2 to 9 for history, reports, prediction, search and delete.
5. Choose **0** to save and exit.

Dates use the format `DD-MM-YYYY`. Press Enter at the date prompt to use today's date.

## Sample Output

```
--- Budget Status for 11/2026 ---
Spent so far   : Rs. 5050.00
Monthly budget : Rs. 5000.00
Budget used    : 101.00%
!! ALERT: Budget EXCEEDED by Rs. 50.00 !!
```

```
--- Expense Prediction ---
Spent so far        : Rs. 5050.00
Days passed         : 3 of 30
Average per day     : Rs. 1683.33
PREDICTED month end : Rs. 50500.00
```

## How Prediction Works

```
daily average   = amount spent so far / days passed in the month
predicted total = daily average x total days in the month
```

If the predicted total is above the budget, the program warns the user and suggests a safe daily spending limit for the remaining days.

## Data Files

These are created automatically in the program folder (they are listed in `.gitignore` and are not uploaded):

| File | Content |
|------|---------|
| `users.txt` | `username|password-hash` |
| `expenses_<user>.txt` | `id|DD-MM-YYYY|category|description|amount` |
| `budget_<user>.txt` | monthly budget amount |

## Limitations and Future Enhancements

- The prediction is a simple average, so early in the month it can be inaccurate.
- Planned: AI-based spending prediction, mobile app, QR bill scanner, cloud backup.

## License

For educational use as part of the BCA curriculum.
