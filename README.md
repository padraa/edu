
# EDU System CLI - Academic Management & LMS in Standard C

[![C Standard](https://img.shields.io/badge/C-C99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)](#compilation--execution)

A full-featured, zero-dependency Command-Line Interface (CLI) Academic Portal and Learning Management System (LMS) implemented in **Standard C (C99)**. Designed with role-based access control (Admin, Faculty, Student), static file persistence (JSON format), academic calendar constraints, PhD thesis advisor verification rules, and interactive LMS homeworks/exams.

---

## Table of Contents
1. [Key Features](#key-features)
2. [System Architecture & Data Models](#system-architecture--data-models)
3. [File Layout & Data Persistence](#file-layout--data-persistence)
4. [Codebase Explanation](#codebase-explanation)
5. [Compilation & Execution](#compilation--execution)
6. [Detailed Verification Test Cases](#detailed-verification-test-cases)
7. [License](#license)

---

## Key Features

### 🔐 1. Authentication & Recovery
* **Role-Based Access Control:** Separate workflows for Admin (Educational Deputy), Faculty Members, and Students.
* **Security Questions Recovery:** Interactive 3-step security question verification for self-service password reset.
* **Input Overflow Protection & Safe EOF Exit:** Protection against buffer overflows using bounded buffer reading and graceful termination on EOF (`Ctrl+D` / piped input).

### 🏛️ 2. Admin (Educational Deputy) Controls
* **Academic Calendar Management:** Toggles four term phases (`offering`, `unit selection`, `class & exams`, `grade recording`).
* **Request Approval Pipeline:** Reviews and approves/rejects faculty course offering requests, capacity increases, or course removals. Approving an offering automatically instantiates an active offering.
* **Student & Faculty Registry:** Interactive search, single-user registration, and removal capabilities.
* **Course Syllabus Management:** Define master syllabus courses (IDs, names, units, prerequisites, academic level).

### 👨‍🏫 3. Faculty Dashboard
* **Offer a Course:** Submit course offering requests to Admin (gated by the `offering` calendar phase).
* **My Offerings & Grade Recording:** View active courses across terms, adjust capacity, and record numeric grades ($0.00 - 20.00$) for enrolled students (gated by `grade_recording`).
* **LMS Publishing:** Publish 4-option multiple-choice homework assignments and exams directly to class sections.

### 🎓 4. Student Dashboard
* **Unit Selection:** Enroll in or withdraw from course offerings (gated by `unit_selection` phase and section capacity).
* **Report Card & Semester GPA:** View term-by-term course history, grade breakdown, pass/fail status, and semester GPA calculation.
* **PhD Doctorate Thesis:** Dedicated panel for PhD candidates to record thesis titles, abstracts, citation counts, and references.
* **Strict Advisor Restriction:** PhD students are strictly restricted from enrolling in "Doctorate Thesis" sections taught by anyone other than their assigned faculty mentor.
* **LMS Access:** Solve published 4-choice homeworks, submit option selections, and review exam questions.

---

## System Architecture & Data Models

The system is built entirely on standard C structures with static arrays bounded by `MAX_ITEMS` (500 entries) to guarantee deterministic memory safety without dynamic allocation leaks.

```
                  +--------------------------+
                  |       main_menu()        |
                  +------------+-------------+
                               |
        +----------------------+----------------------+
        |                      |                      |
+-------v-------+      +-------v-------+      +-------v-------+
| Student Panel |      | Faculty Panel |      |  Admin Panel  |
+-------+-------+      +-------+-------+      +-------+-------+
        |                      |                      |
        +----------------------+----------------------+
                               |
                  +------------v-------------+
                  | JSON Persistence Engine  |
                  |  (load / save _all_data) |
                  +--------------------------+
```

### Core Structs (`main.c`)
* `Student`: Personal info, security Q&A, PhD thesis metadata, and `StudentGrade` array.
* `Faculty`: Personal info, degree, department, and security Q&A.
* `Course`: Master syllabus details (units, prerequisites, department).
* `Offering`: Instantiated course section for a specific semester (capacity, enrollments, place, faculty ID).
* `AdminRequest`: Pending requests submitted by faculty awaiting admin approval.
* `AcademicCalendar`: Boolean toggles representing current term phases.
* `Homework` & `Exam`: LMS assignment specifications and student submission records (`HomeworkSubmission`).

---

## File Layout & Data Persistence

All data is stored statically in human-readable JSON files. If a file is missing at startup, the system automatically creates and seeds it with default data.

| File | Content Description |
| :--- | :--- |
| `students.json` | Student profiles, security answers, enrolled courses, and PhD thesis info |
| `faculty.json` | Faculty member profiles and security answers |
| `courses.json` | Approved master course list (syllabi) |
| `offerings.json` | Active course sections offered per semester |
| `requests.json` | Pending faculty requests requiring admin action |
| `calendar.json` | Academic calendar phase states |
| `lms.json` | Published LMS homeworks, student submissions, and exams |

---

## Codebase Explanation

### Key Utility Functions

#### 1. Screen Refresh (`clear_screen`)
Uses portable ANSI escape sequences on POSIX terminals (`\033[2J\033[H`) and `cls` on Windows to clear the terminal buffer on every menu loop transition, creating a clean GUI-like experience in the terminal.

#### 2. Robust Input Handling (`get_input`)
```c
bool get_input(char *buffer, int size) {
    if (fgets(buffer, size, stdin) == NULL) {
        printf("\nInput ended. Exiting.\n");
        exit(0);
    }
    trim_newline(buffer);
    return true;
}
```
Replaces unsafe `scanf` or `gets` with `fgets`. Detects `EOF` (end-of-file) gracefully to prevent infinite loops when input is piped from test scripts.

#### 3. Portable String Matching (`my_strcasecmp` & `my_strcasestr_contains`)
Custom C99 implementations of case-insensitive string equality and substring search using `tolower()`, replacing non-standard functions like POSIX `strcasecmp` or GNU `strcasestr` for cross-platform MSVC/GCC compatibility.

#### 4. Lightweight JSON Parsing (`parse_json_value`)
```c
/*
 * NOTE ON JSON PARSER:
 * This parser is intentionally simple and lightweight for instructional purposes.
 * It expects key-value pairs formatted on individual lines within JSON objects.
 */
void parse_json_value(const char* line, const char* key, char* out_val);
```
Reads structured key-value pairs without requiring external third-party libraries (e.g., `cJSON`), maintaining zero external dependencies.

---

## Compilation & Execution

### Prerequisites
* A standard C compiler supporting **C99** or later (`gcc`, `clang`, or `MSVC`).

### Compile Command
```bash
gcc -std=c99 -Wall main.c -o edu
```

### Run Command
```bash
./edu
```

---

## Detailed Verification Test Cases

Below are test scenarios to verify all system features end-to-end.

### Test Case 1: Password Recovery via Security Questions
1. At the main menu, select `4. Forgot password`.
2. Enter username: `404123456`.
3. Answer security questions:
   * Where were you born? `Karaj`
   * First book read? `Anne Shirley`
   * First bicycle color? `White`
4. Enter new password and confirm it.
5. **Expected Result:** `Password changed successfully.`. Log in using the new password.

---

### Test Case 2: Academic Calendar Enforcements
1. Log in as Faculty `FCS101` (`pass`).
2. Select `4. Offer a course`.
3. If `offering` phase is disabled in `calendar.json`, the system prints:  
   `Offering period is currently disabled.`
4. Log in as Admin (`admin` / `admin`) -> `1. Calendar` -> Toggle option `1` (`offering: enabled`).
5. Return to Faculty `FCS101` -> `4. Offer a course` -> Enter ID `CS101`, capacity `40`.
6. **Expected Result:** `Sent request to admin.` request created successfully.

---

### Test Case 3: Request Approval Pipeline
1. Log in as Admin (`admin` / `admin`).
2. Select `4. Requests`.
3. View pending request (e.g. Request #1: `course offering` for course `CS101` by `Dr. Hossein Asadi`).
4. Select `1. Go to request number`, enter `1`, select `1. Approve`.
5. Navigate to `5. Offerings` -> enter semester `14042`.
6. **Expected Result:** The newly approved course section appears in the semester offerings table with `enrollments: 0` assigned to `Dr. Hossein Asadi`.

---

### Test Case 4: PhD Thesis Advisor Restriction Rule
1. Log in as PhD student `402456789` (Rozhan Azizi, Mentor: `Hossein Asadi`, Password: `pass`).
2. Select `1. Offerings`, enter semester `14042`.
3. Attempt to enroll in a "Doctorate Thesis" offering taught by a different faculty member (e.g. Dr. Hasan Rezaei).
4. **Expected Result:** The system blocks enrollment and prints:  
   `Error: PhD students can only take thesis with their assigned mentor (Hossein Asadi).`
5. Attempt to enroll in the section taught by `Dr. Hossein Asadi`.
6. **Expected Result:** `Enrolled successfully.`.

---

### Test Case 5: LMS Homework & Exam Lifecycle
1. Log in as Faculty `FCS101` (`pass`). Select `1. My offerings` -> Offering `1`.
2. Select `4. Publish a homework`:
   * Title: `Data Structures HW1`
   * Question: `What is the time complexity of searching in a Balanced BST?`
   * Options: `1) O(1)`, `2) O(log N)`, `3) O(N)`, `4) O(N^2)`
   * Correct Option: `2`
3. Select `5. Publish an exam`: Title: `Midterm Exam`, Max Score: `20`.
4. Log out and log in as enrolled student `404123456` (`pass`).
5. Select `4. LMS (Homeworks & Exams)` -> `1. View & Submit Homeworks`.
6. Select Homework #1, submit choice `2`.
7. **Expected Result:** System records submission and displays `Status: Submitted (Choice: 2)`.

---

## License

This project is open-source and available under the [MIT License](LICENSE).
```
