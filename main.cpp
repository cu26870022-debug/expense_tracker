// ============================================================================
// Lab Sheet-09 : Smart Digital Expense Tracker with Budget Prediction
// BCA I Semester (Session 2026-2027)
//
// Modules : 1 User Registration     2 Login System        3 Add Daily Expense
//           4 View Expense History  5 Category-wise       6 Monthly Report
//           7 Budget Setting        8 Budget Warning      9 Expense Prediction
//          10 Search Expense       11 Delete Expense     12 File Saving
//
// Concepts: variables, loops, arrays, structures, functions, file handling,
//           strings, searching (linear search) and sorting (insertion sort).
//
// Data files created in the working folder:
//   users.txt               -> username|password-hash
//   expenses_<user>.txt     -> id|DD-MM-YYYY|category|description|amount
//   budget_<user>.txt       -> monthly budget amount
// ============================================================================
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <functional>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <ctime>
using namespace std;

// ---------------------------- Constants & Data ------------------------------
const int MAX_EXPENSES = 500;
const int CATEGORY_COUNT = 6;
const string CATEGORY_NAMES[CATEGORY_COUNT] =
    {"Food", "Travel", "Education", "Entertainment", "Shopping", "Other"};
const string USERS_FILE = "users.txt";

// One expense record
struct Expense {
    int id;               // unique number used for deleting
    int day, month, year; // date of expense
    string category;
    string description;
    double amount;        // in rupees
};

Expense expenses[MAX_EXPENSES];  // all expenses of the logged-in user
int expenseCount = 0;            // how many are stored in the array
string currentUser;              // logged-in username
double monthlyBudget = 0;        // 0 means "not set"
bool echoInput = false;          // run with --echo to print typed input (for reports)

// ------------------------------ String helpers ------------------------------
string trim(const string &text) {
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

string toLowerCase(string text) {
    for (size_t i = 0; i < text.size(); i++)
        text[i] = (char)tolower((unsigned char)text[i]);
    return text;
}

// ------------------------------ Input helpers -------------------------------
// Reads one full line; exits cleanly if input ends (Ctrl+D / Ctrl+Z)
string readLine(const string &prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        cout << "\nInput ended. Goodbye!\n";
        exit(0);
    }
    if (echoInput) cout << line << endl;
    return line;
}

// Reads a whole number in [minValue, maxValue], repeating until valid
int readInt(const string &prompt, int minValue, int maxValue) {
    while (true) {
        string line = trim(readLine(prompt));
        try {
            size_t used = 0;
            int value = stoi(line, &used);
            if (used == line.size() && value >= minValue && value <= maxValue)
                return value;
        } catch (...) {}
        cout << "  Please enter a whole number between " << minValue
             << " and " << maxValue << ".\n";
    }
}

// Reads a positive amount of money
double readAmount(const string &prompt) {
    while (true) {
        string line = trim(readLine(prompt));
        try {
            size_t used = 0;
            double value = stod(line, &used);
            if (used == line.size() && value > 0 && value <= 10000000) return value;
        } catch (...) {}
        cout << "  Please enter a positive amount (up to 10000000).\n";
    }
}

// ------------------------------- Date helpers -------------------------------
bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysInMonth(int month, int year) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year)) return 29;
    return days[month - 1];
}

bool isValidDate(int day, int month, int year) {
    return year >= 2000 && year <= 2100 && month >= 1 && month <= 12 &&
           day >= 1 && day <= daysInMonth(month, year);
}

void getToday(int &day, int &month, int &year) {
    time_t now = time(0);
    tm *local = localtime(&now);
    day = local->tm_mday;
    month = local->tm_mon + 1;
    year = local->tm_year + 1900;
}

// Turns a date into a sortable number, e.g. 15-08-2026 -> 20260815
int dateKey(const Expense &e) { return e.year * 10000 + e.month * 100 + e.day; }

string formatDate(int day, int month, int year) {
    ostringstream out;
    out << setfill('0') << setw(2) << day << "-" << setw(2) << month << "-" << year;
    return out.str();
}

// Reads DD-MM-YYYY; pressing Enter uses today's date
void readDate(const string &prompt, int &day, int &month, int &year) {
    while (true) {
        string line = trim(readLine(prompt));
        if (line.empty()) { getToday(day, month, year); return; }
        if (sscanf(line.c_str(), "%d-%d-%d", &day, &month, &year) == 3 &&
            isValidDate(day, month, year))
            return;
        cout << "  Invalid date. Use DD-MM-YYYY (example: 15-08-2026).\n";
    }
}

void readMonthYear(int &month, int &year) {
    month = readInt("Enter month (1-12): ", 1, 12);
    year = readInt("Enter year (example 2026): ", 2000, 2100);
}

// =========================== MODULES 1 & 2: AUTH ============================
bool isValidUsername(const string &username) {
    if (username.size() < 3 || username.size() > 20) return false;
    for (size_t i = 0; i < username.size(); i++)
        if (!isalnum((unsigned char)username[i]) && username[i] != '_') return false;
    return true;
}

// Passwords are stored as a hash, never as plain text
string hashPassword(const string &password) {
    ostringstream out;
    out << hash<string>()(password);
    return out.str();
}

// Linear search for a username in users.txt
bool findUser(const string &username, string &storedHash) {
    ifstream file(USERS_FILE.c_str());
    string line;
    while (getline(file, line)) {
        size_t bar = line.find('|');
        if (bar == string::npos) continue;
        if (line.substr(0, bar) == username) {
            storedHash = trim(line.substr(bar + 1));
            return true;
        }
    }
    return false;
}

// Module 1: User Registration
void registerUser() {
    cout << "\n--- User Registration ---\n";
    string username = toLowerCase(trim(readLine("Choose username (3-20 letters/digits/_): ")));
    if (!isValidUsername(username)) { cout << "  Invalid username.\n"; return; }
    string existingHash;
    if (findUser(username, existingHash)) { cout << "  Username already exists.\n"; return; }
    string password = readLine("Choose password (min 4 characters): ");
    if (password.size() < 4) { cout << "  Password too short.\n"; return; }
    if (password != readLine("Confirm password: ")) { cout << "  Passwords do not match.\n"; return; }
    ofstream file(USERS_FILE.c_str(), ios::app);
    file << username << "|" << hashPassword(password) << "\n";
    cout << "  Registration successful! You can now log in.\n";
}

// Module 2: Login System (3 attempts)
bool loginUser() {
    cout << "\n--- Login ---\n";
    for (int attempt = 1; attempt <= 3; attempt++) {
        string username = toLowerCase(trim(readLine("Username: ")));
        string password = readLine("Password: ");
        string storedHash;
        if (findUser(username, storedHash) && storedHash == hashPassword(password)) {
            currentUser = username;
            cout << "  Login successful. Welcome, " << currentUser << "!\n";
            return true;
        }
        cout << "  Wrong username or password. Attempts left: " << (3 - attempt) << "\n";
    }
    return false;
}

// ========================= MODULE 12: FILE SAVING ===========================
string expensesFileName() { return "expenses_" + currentUser + ".txt"; }
string budgetFileName() { return "budget_" + currentUser + ".txt"; }

void saveExpenses() {
    ofstream file(expensesFileName().c_str());
    for (int i = 0; i < expenseCount; i++) {
        file << expenses[i].id << "|"
             << formatDate(expenses[i].day, expenses[i].month, expenses[i].year) << "|"
             << expenses[i].category << "|" << expenses[i].description << "|"
             << fixed << setprecision(2) << expenses[i].amount << "\n";
    }
}

void loadExpenses() {
    expenseCount = 0;
    ifstream file(expensesFileName().c_str());
    string line;
    while (expenseCount < MAX_EXPENSES && getline(file, line)) {
        string fields[5];
        stringstream parts(line);
        int fieldCount = 0;
        while (fieldCount < 5 && getline(parts, fields[fieldCount], '|')) fieldCount++;
        if (fieldCount < 5) continue;  // skip damaged lines
        Expense e;
        try {
            e.id = stoi(fields[0]);
            e.amount = stod(fields[4]);
        } catch (...) { continue; }
        if (sscanf(fields[1].c_str(), "%d-%d-%d", &e.day, &e.month, &e.year) != 3) continue;
        e.category = fields[2];
        e.description = fields[3];
        expenses[expenseCount++] = e;
    }
}

void saveBudget() {
    ofstream file(budgetFileName().c_str());
    file << monthlyBudget << "\n";
}

void loadBudget() {
    monthlyBudget = 0;
    ifstream file(budgetFileName().c_str());
    if (!(file >> monthlyBudget)) monthlyBudget = 0;
}

// ============================ Common calculations ===========================
int nextExpenseId() {
    int highest = 0;
    for (int i = 0; i < expenseCount; i++)
        if (expenses[i].id > highest) highest = expenses[i].id;
    return highest + 1;
}

int categoryIndex(const string &name) {
    for (int i = 0; i < CATEGORY_COUNT; i++)
        if (CATEGORY_NAMES[i] == name) return i;
    return CATEGORY_COUNT - 1;  // unknown names count as "Other"
}

bool inMonth(const Expense &e, int month, int year) {
    return e.month == month && e.year == year;
}

double monthTotal(int month, int year) {
    double total = 0;
    for (int i = 0; i < expenseCount; i++)
        if (inMonth(expenses[i], month, year)) total += expenses[i].amount;
    return total;
}

// Fills totals[] per category. month == 0 means "all time". Returns grand total.
double computeCategoryTotals(double totals[], int month, int year) {
    double grandTotal = 0;
    for (int i = 0; i < CATEGORY_COUNT; i++) totals[i] = 0;
    for (int i = 0; i < expenseCount; i++) {
        if (month == 0 || inMonth(expenses[i], month, year)) {
            totals[categoryIndex(expenses[i].category)] += expenses[i].amount;
            grandTotal += expenses[i].amount;
        }
    }
    return grandTotal;
}

void printCategoryTable(const double totals[], double grandTotal) {
    cout << left << setw(16) << "Category" << right << setw(14) << "Amount (Rs.)"
         << setw(10) << "Share" << "\n";
    cout << string(40, '-') << "\n";
    for (int i = 0; i < CATEGORY_COUNT; i++) {
        double share = grandTotal > 0 ? totals[i] / grandTotal * 100 : 0;
        cout << left << setw(16) << CATEGORY_NAMES[i] << right << setw(14) << totals[i]
             << setw(9) << share << "%\n";
    }
    cout << string(40, '-') << "\n";
    cout << left << setw(16) << "TOTAL" << right << setw(14) << grandTotal << "\n";
}

// --------------------------------- Sorting ----------------------------------
// mode 1 = date (oldest first), 2 = amount (highest first), 3 = order entered
bool shouldComeAfter(const Expense &a, const Expense &b, int mode) {
    if (mode == 1) return dateKey(a) > dateKey(b);
    if (mode == 2) return a.amount < b.amount;
    return a.id > b.id;
}

// Insertion sort on a COPY, so the stored data keeps its order
void sortExpenses(Expense list[], int size, int mode) {
    for (int i = 1; i < size; i++) {
        Expense key = list[i];
        int j = i - 1;
        while (j >= 0 && shouldComeAfter(list[j], key, mode)) {
            list[j + 1] = list[j];
            j--;
        }
        list[j + 1] = key;
    }
}

void printTable(const Expense list[], int size) {
    if (size == 0) { cout << "  No expenses to show.\n"; return; }
    cout << left << setw(5) << "ID" << setw(12) << "Date" << setw(15) << "Category"
         << setw(26) << "Description" << right << setw(12) << "Amount" << "\n";
    cout << string(70, '-') << "\n";
    double total = 0;
    for (int i = 0; i < size; i++) {
        string shortText = list[i].description.substr(0, 24);
        cout << left << setw(5) << list[i].id
             << setw(12) << formatDate(list[i].day, list[i].month, list[i].year)
             << setw(15) << list[i].category << setw(26) << shortText
             << right << setw(12) << list[i].amount << "\n";
        total += list[i].amount;
    }
    cout << string(70, '-') << "\n";
    cout << left << setw(58) << (to_string(size) + " record(s)  |  Total") << right
         << setw(12) << total << "\n";
}

// ====================== MODULE 8: BUDGET WARNING ============================
void showBudgetStatus(int month, int year) {
    double spent = monthTotal(month, year);
    cout << "\n--- Budget Status for " << setfill('0') << setw(2) << month << setfill(' ')
         << "/" << year << " ---\n";
    cout << "Spent so far   : Rs. " << spent << "\n";
    if (monthlyBudget <= 0) {
        cout << "No monthly budget is set. Use 'Set Monthly Budget' first.\n";
        return;
    }
    double percentUsed = spent / monthlyBudget * 100;
    cout << "Monthly budget : Rs. " << monthlyBudget << "\n";
    cout << "Budget used    : " << percentUsed << "%\n";
    if (spent > monthlyBudget)
        cout << "!! ALERT: Budget EXCEEDED by Rs. " << (spent - monthlyBudget) << " !!\n";
    else if (percentUsed >= 80)
        cout << "!! WARNING: You have used over 80% of your budget. Only Rs. "
             << (monthlyBudget - spent) << " left.\n";
    else
        cout << "OK: You are within budget. Rs. " << (monthlyBudget - spent) << " left.\n";
}

// ========================= MODULE 3: ADD EXPENSE ============================
void addExpense() {
    cout << "\n--- Add Daily Expense ---\n";
    if (expenseCount >= MAX_EXPENSES) { cout << "  Expense list is full.\n"; return; }
    Expense e;
    e.id = nextExpenseId();
    readDate("Date DD-MM-YYYY (press Enter for today): ", e.day, e.month, e.year);

    cout << "Categories:\n";
    for (int i = 0; i < CATEGORY_COUNT; i++) cout << "  " << (i + 1) << ". " << CATEGORY_NAMES[i] << "\n";
    e.category = CATEGORY_NAMES[readInt("Choose category (1-6): ", 1, CATEGORY_COUNT) - 1];

    while (true) {
        e.description = trim(readLine("Description: "));
        if (!e.description.empty()) break;
        cout << "  Description cannot be empty.\n";
    }
    for (size_t i = 0; i < e.description.size(); i++)   // '|' is our file separator
        if (e.description[i] == '|') e.description[i] = '-';

    e.amount = readAmount("Amount (Rs.): ");
    expenses[expenseCount++] = e;
    saveExpenses();
    cout << "  Expense saved (ID " << e.id << ").\n";
    if (monthlyBudget > 0) showBudgetStatus(e.month, e.year);   // instant warning
}

// ======================= MODULE 4: VIEW HISTORY =============================
void viewHistory() {
    cout << "\n--- Expense History ---\n";
    if (expenseCount == 0) { cout << "  No expenses recorded yet.\n"; return; }
    cout << "Sort by: 1. Date (oldest first)  2. Amount (highest first)  3. Order entered\n";
    int mode = readInt("Choose (1-3): ", 1, 3);
    Expense sortedList[MAX_EXPENSES];
    for (int i = 0; i < expenseCount; i++) sortedList[i] = expenses[i];
    sortExpenses(sortedList, expenseCount, mode);
    printTable(sortedList, expenseCount);
}

// ====================== MODULE 5: CATEGORY-WISE =============================
void categoryWiseExpense() {
    cout << "\n--- Category-wise Expense (All Time) ---\n";
    double totals[CATEGORY_COUNT];
    double grandTotal = computeCategoryTotals(totals, 0, 0);
    if (grandTotal == 0) { cout << "  No expenses recorded yet.\n"; return; }
    printCategoryTable(totals, grandTotal);
}

// ====================== MODULE 6: MONTHLY REPORT ============================
void monthlyReport() {
    cout << "\n--- Monthly Expense Report ---\n";
    int month, year;
    readMonthYear(month, year);

    Expense monthList[MAX_EXPENSES];
    int monthCount = 0;
    for (int i = 0; i < expenseCount; i++)
        if (inMonth(expenses[i], month, year)) monthList[monthCount++] = expenses[i];

    cout << "\n===== REPORT: " << setfill('0') << setw(2) << month << setfill(' ') << "/" << year
         << "  (User: " << currentUser << ") =====\n";
    if (monthCount == 0) { cout << "  No expenses in this month.\n"; return; }

    sortExpenses(monthList, monthCount, 1);
    printTable(monthList, monthCount);

    int highest = 0;                                    // find the largest expense
    for (int i = 1; i < monthCount; i++)
        if (monthList[i].amount > monthList[highest].amount) highest = i;
    cout << "\nHighest expense : Rs. " << monthList[highest].amount << " ("
         << monthList[highest].description << ")\n\n";

    double totals[CATEGORY_COUNT];
    double grandTotal = computeCategoryTotals(totals, month, year);
    printCategoryTable(totals, grandTotal);
    showBudgetStatus(month, year);
}

// ======================= MODULE 7: BUDGET SETTING ===========================
void setBudget() {
    cout << "\n--- Set Monthly Budget ---\n";
    if (monthlyBudget > 0) cout << "Current budget: Rs. " << monthlyBudget << "\n";
    monthlyBudget = readAmount("Enter new monthly budget (Rs.): ");
    saveBudget();
    cout << "  Budget saved: Rs. " << monthlyBudget << "\n";
}

// ===================== MODULE 9: EXPENSE PREDICTION =========================
void predictExpense() {
    cout << "\n--- Expense Prediction ---\n";
    int month, year;
    readMonthYear(month, year);

    int todayDay, todayMonth, todayYear;
    getToday(todayDay, todayMonth, todayYear);
    int selected = year * 12 + month, current = todayYear * 12 + todayMonth;

    if (selected > current) { cout << "  That month is in the future - no data to predict from.\n"; return; }

    double spent = monthTotal(month, year);
    int totalDays = daysInMonth(month, year);
    cout << "Spent so far        : Rs. " << spent << "\n";

    if (selected < current) {   // month already finished
        cout << "This month is over, so the final total is Rs. " << spent << ".\n";
        if (monthlyBudget > 0)
            cout << (spent > monthlyBudget ? "The budget WAS exceeded.\n" : "The budget was NOT exceeded.\n");
        return;
    }

    // Current month: predicted total = average per day so far x days in month
    double dailyAverage = spent / todayDay;
    double predictedTotal = dailyAverage * totalDays;
    int daysLeft = totalDays - todayDay;
    cout << "Days passed         : " << todayDay << " of " << totalDays << "\n";
    cout << "Average per day     : Rs. " << dailyAverage << "\n";
    cout << "PREDICTED month end : Rs. " << predictedTotal << "\n";

    if (monthlyBudget <= 0) { cout << "Set a monthly budget to compare the prediction.\n"; return; }
    cout << "Monthly budget      : Rs. " << monthlyBudget << "\n";
    if (predictedTotal > monthlyBudget) {
        cout << "!! PREDICTION: You are likely to EXCEED your budget by about Rs. "
             << (predictedTotal - monthlyBudget) << ".\n";
        if (daysLeft > 0 && spent < monthlyBudget)
            cout << "   Tip: spend at most Rs. " << (monthlyBudget - spent) / daysLeft
                 << " per day for the remaining " << daysLeft << " day(s).\n";
        else if (spent >= monthlyBudget)
            cout << "   Budget is already used up. Avoid extra spending.\n";
    } else {
        cout << "GOOD: At this rate you should stay within budget (about Rs. "
             << (monthlyBudget - predictedTotal) << " to spare).\n";
    }
}

// ========================= MODULE 10: SEARCH ================================
void searchExpense() {
    cout << "\n--- Search Expense ---\n";
    cout << "1. By keyword (description/category)  2. By exact date  3. By amount range\n";
    int choice = readInt("Choose (1-3): ", 1, 3);
    Expense found[MAX_EXPENSES];
    int foundCount = 0;

    if (choice == 1) {
        string keyword = toLowerCase(trim(readLine("Keyword: ")));
        for (int i = 0; i < expenseCount; i++) {      // linear search
            string text = toLowerCase(expenses[i].description + " " + expenses[i].category);
            if (!keyword.empty() && text.find(keyword) != string::npos) found[foundCount++] = expenses[i];
        }
    } else if (choice == 2) {
        int day, month, year;
        readDate("Date DD-MM-YYYY (Enter for today): ", day, month, year);
        for (int i = 0; i < expenseCount; i++)
            if (expenses[i].day == day && inMonth(expenses[i], month, year)) found[foundCount++] = expenses[i];
    } else {
        double low = readAmount("Minimum amount: ");
        double high = readAmount("Maximum amount: ");
        for (int i = 0; i < expenseCount; i++)
            if (expenses[i].amount >= low && expenses[i].amount <= high) found[foundCount++] = expenses[i];
    }
    cout << "\nSearch results:\n";
    printTable(found, foundCount);
}

// ======================== MODULE 11: DELETE =================================
void deleteExpense() {
    cout << "\n--- Delete Expense ---\n";
    if (expenseCount == 0) { cout << "  No expenses to delete.\n"; return; }
    printTable(expenses, expenseCount);
    int idToDelete = readInt("Enter ID to delete (0 to cancel): ", 0, 1000000);
    if (idToDelete == 0) { cout << "  Cancelled.\n"; return; }

    int position = -1;
    for (int i = 0; i < expenseCount; i++)
        if (expenses[i].id == idToDelete) { position = i; break; }
    if (position == -1) { cout << "  No expense with that ID.\n"; return; }

    string answer = toLowerCase(trim(readLine("Delete '" + expenses[position].description + "'? (y/n): ")));
    if (answer != "y") { cout << "  Not deleted.\n"; return; }
    for (int i = position; i < expenseCount - 1; i++) expenses[i] = expenses[i + 1];  // shift left
    expenseCount--;
    saveExpenses();
    cout << "  Expense deleted.\n";
}

// =============================== MENUS ======================================
void showMainMenu() {
    cout << "\n==================== EXPENSE TRACKER ====================\n";
    cout << " User: " << currentUser << "\n";
    cout << "  1. Add Daily Expense\n  2. View Expense History\n  3. Category-wise Expense\n";
    cout << "  4. Monthly Expense Report\n  5. Set Monthly Budget\n  6. Budget Warning Check\n";
    cout << "  7. Expense Prediction\n  8. Search Expense\n  9. Delete Expense\n";
    cout << " 10. Save Data to File\n  0. Logout & Exit\n";
    cout << "=========================================================\n";
}

// Start menu: register / login. Returns true once a user is logged in.
bool startMenu() {
    while (true) {
        cout << "\n====== SMART DIGITAL EXPENSE TRACKER ======\n";
        cout << "  1. Register\n  2. Login\n  0. Exit\n";
        int choice = readInt("Choose (0-2): ", 0, 2);
        if (choice == 1) registerUser();
        else if (choice == 2) { if (loginUser()) return true; cout << "  Too many failed attempts.\n"; }
        else return false;
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1 && string(argv[1]) == "--echo") echoInput = true;
    cout << fixed << setprecision(2);

    if (!startMenu()) { cout << "Goodbye!\n"; return 0; }
    loadExpenses();
    loadBudget();

    int choice;
    do {
        showMainMenu();
        choice = readInt("Enter your choice (0-10): ", 0, 10);
        switch (choice) {
            case 1: addExpense(); break;
            case 2: viewHistory(); break;
            case 3: categoryWiseExpense(); break;
            case 4: monthlyReport(); break;
            case 5: setBudget(); break;
            case 6: { int m, y; cout << "\n--- Budget Warning Check ---\n"; readMonthYear(m, y); showBudgetStatus(m, y); break; }
            case 7: predictExpense(); break;
            case 8: searchExpense(); break;
            case 9: deleteExpense(); break;
            case 10: saveExpenses(); saveBudget(); cout << "  Data saved to " << expensesFileName() << ".\n"; break;
        }
    } while (choice != 0);

    saveExpenses();
    saveBudget();
    cout << "Data saved. Goodbye, " << currentUser << "!\n";
    return 0;
}
