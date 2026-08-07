#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_BUFFER 2048
#define MAX_ITEMS 500

/* System Data Structures */

typedef struct {
    char semester[16];
    char course_id[32];
    double grade; // -1.0 indicates enrolled but grade not recorded yet
} StudentGrade;

typedef struct {
    char id[32];
    char password[64];
    char first_name[64];
    char last_name[64];
    char national_code[32];
    char field[64];
    char entrance_year[16];
    char section[32];
    char mentor[64];
    char department[64];
    char ans1[64];
    char ans2[64];
    char ans3[64];
    StudentGrade grades[100];
    int grade_count;
    // PhD Thesis fields
    char thesis_title[128];
    char thesis_abstract[256];
    int thesis_citations;
    int thesis_refs;
} Student;

typedef struct {
    char id[32];
    char password[64];
    char first_name[64];
    char last_name[64];
    char national_code[32];
    char field[64];
    char entrance_year[16];
    char degree[32];
    char department[64];
    char ans1[64];
    char ans2[64];
    char ans3[64];
} Faculty;

typedef struct {
    char course_id[32];
    char course_name[128];
    int units;
    char prerequisites[128];
    char section[32];
    char field[64];
    char department[64];
} Course;

typedef struct {
    int number;
    char course_id[32];
    char course_name[128];
    char faculty_id[32];
    char faculty_name[128];
    char semester[16];
    int capacity;
    int enrollments;
    char department[64];
    char place[128];
} Offering;

typedef struct {
    int id;
    char type[32];
    char course_name[128];
    char faculty_id[32];
    char faculty_name[128];
    char department[64];
    int capacity;
    int enrollments;
} AdminRequest;

typedef struct {
    bool offering;
    bool unit_selection;
    bool class_exams;
    bool grade_recording;
} AcademicCalendar;

/* LMS Data Structures */

typedef struct {
    int id;
    int offering_number;
    char title[128];
    char question[256];
    char opt1[64];
    char opt2[64];
    char opt3[64];
    char opt4[64];
    int correct_option;
} Homework;

typedef struct {
    char student_id[32];
    int homework_id;
    int selected_option;
} HomeworkSubmission;

typedef struct {
    int id;
    int offering_number;
    char title[128];
    char question[256];
    int max_score;
} Exam;

/* Global Storage Arrays */
Student g_students[MAX_ITEMS];
int g_student_count = 0;

Faculty g_faculty[MAX_ITEMS];
int g_faculty_count = 0;

Course g_courses[MAX_ITEMS];
int g_course_count = 0;

Offering g_offerings[MAX_ITEMS];
int g_offering_count = 0;

AdminRequest g_requests[MAX_ITEMS];
int g_request_count = 0;

AcademicCalendar g_calendar = {false, false, false, false};

Homework g_homeworks[MAX_ITEMS];
int g_homework_count = 0;

HomeworkSubmission g_submissions[MAX_ITEMS];
int g_submission_count = 0;

Exam g_exams[MAX_ITEMS];
int g_exam_count = 0;

char g_current_user_id[32] = "";
int g_current_role = 0; // 1: Student, 2: Faculty, 3: Admin

/* Function Declarations */
void clear_screen(void);
void press_enter_to_continue(void);
void trim_newline(char *str);
void safe_strcpy(char *dest, const char *src, size_t max_size);
int my_strcasecmp(const char *s1, const char *s2);
int my_strcasestr_contains(const char *haystack, const char *needle);
bool get_input(char *buffer, int size);

void initialize_data_files(void);
void load_all_data(void);
void save_all_data(void);

void main_menu(void);
void login_flow(int role);
void forgot_password_flow(void);

void admin_dashboard(void);
void admin_calendar_menu(void);
void admin_students_menu(void);
void admin_faculty_menu(void);
void admin_requests_menu(void);
void offerings_menu_flow(int role);
void courses_menu_flow(int role);

void faculty_dashboard(void);
void faculty_my_offerings(void);
void faculty_offering_detail_menu(int idx);
void faculty_offer_course(void);

void student_dashboard(void);
void student_report_card(void);
void student_phd_thesis_menu(void);
void student_lms_menu(void);

/* Helper Functions */

void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
}

void trim_newline(char *str) {
    if (!str) return;
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r' || str[len - 1] == ' ')) {
        str[len - 1] = '\0';
        len--;
    }
}

void safe_strcpy(char *dest, const char *src, size_t max_size) {
    if (!dest || max_size == 0) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    strncpy(dest, src, max_size - 1);
    dest[max_size - 1] = '\0';
}

int my_strcasecmp(const char *s1, const char *s2) {
    if (!s1 || !s2) return s1 == s2 ? 0 : (s1 ? 1 : -1);
    while (*s1 && *s2) {
        unsigned char c1 = (unsigned char)tolower((unsigned char)*s1);
        unsigned char c2 = (unsigned char)tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return (unsigned char)tolower((unsigned char)*s1) - (unsigned char)tolower((unsigned char)*s2);
}

int my_strcasestr_contains(const char *haystack, const char *needle) {
    if (!haystack || !needle) return 0;
    if (*needle == '\0') return 1;
    size_t needle_len = strlen(needle);
    size_t haystack_len = strlen(haystack);
    if (needle_len > haystack_len) return 0;

    for (size_t i = 0; i <= haystack_len - needle_len; i++) {
        bool match = true;
        for (size_t j = 0; j < needle_len; j++) {
            if (tolower((unsigned char)haystack[i + j]) != tolower((unsigned char)needle[j])) {
                match = false;
                break;
            }
        }
        if (match) return 1;
    }
    return 0;
}

bool get_input(char *buffer, int size) {
    if (fgets(buffer, size, stdin) == NULL) {
        printf("\nInput ended. Exiting.\n");
        exit(0);
    }
    trim_newline(buffer);
    return true;
}

void press_enter_to_continue(void) {
    printf("\nPress Enter to continue...");
    char temp[16];
    get_input(temp, sizeof(temp));
}

/* File Storage & Persistence */

void initialize_data_files(void) {
    FILE *f;
    f = fopen("students.json", "r");
    if (!f) {
        f = fopen("students.json", "w");
        if (f) {
            fputs("[\n"
                  "  {\n"
                  "    \"id\": \"404123456\", \"password\": \"pass\", \"first_name\": \"Ali\", \"last_name\": \"Ahmadi\",\n"
                  "    \"national_code\": \"0123456789\", \"field\": \"Computer Engineering\", \"entrance_year\": \"404\",\n"
                  "    \"section\": \"BSc\", \"mentor\": \"Hossein Asadi\", \"department\": \"Computer Engineering\",\n"
                  "    \"ans1\": \"Karaj\", \"ans2\": \"Anne Shirley\", \"ans3\": \"White\",\n"
                  "    \"grades\": [\n"
                  "      {\"semester\": \"14042\", \"course_id\": \"CS101\", \"grade\": 18.25},\n"
                  "      {\"semester\": \"14042\", \"course_id\": \"PHY102\", \"grade\": 14.00},\n"
                  "      {\"semester\": \"14042\", \"course_id\": \"CS103\", \"grade\": 16.75},\n"
                  "      {\"semester\": \"14042\", \"course_id\": \"MATH102\", \"grade\": 12.50},\n"
                  "      {\"semester\": \"14042\", \"course_id\": \"CS104\", \"grade\": 9.75}\n"
                  "    ]\n"
                  "  },\n"
                  "  {\n"
                  "    \"id\": \"402456789\", \"password\": \"pass\", \"first_name\": \"Rozhan\", \"last_name\": \"Azizi\",\n"
                  "    \"national_code\": \"3456789012\", \"field\": \"Civil Engineering\", \"entrance_year\": \"402\",\n"
                  "    \"section\": \"PhD\", \"mentor\": \"Hossein Asadi\", \"department\": \"Civil Engineering\",\n"
                  "    \"ans1\": \"Sanandaj\", \"ans2\": \"The Alchemist\", \"ans3\": \"Green\", \"thesis_title\": \"Advanced Structural Analysis\", \"thesis_abstract\": \"A study on resilient structures.\", \"thesis_citations\": 12, \"thesis_refs\": 5, \"grades\": []\n"
                  "  }\n"
                  "]\n", f);
            fclose(f);
        }
    } else {
        fclose(f);
    }

    f = fopen("faculty.json", "r");
    if (!f) {
        f = fopen("faculty.json", "w");
        if (f) {
            fputs("[\n"
                  "  {\n"
                  "    \"id\": \"FCS101\", \"password\": \"pass\", \"first_name\": \"Hossein\", \"last_name\": \"Asadi\",\n"
                  "    \"national_code\": \"0011223344\", \"field\": \"Computer Engineering\", \"entrance_year\": \"1395\",\n"
                  "    \"degree\": \"PhD\", \"department\": \"Computer Engineering\", \"ans1\": \"Tehran\", \"ans2\": \"Shahnameh\", \"ans3\": \"Black\"\n"
                  "  },\n"
                  "  {\n"
                  "    \"id\": \"FCS105\", \"password\": \"pass\", \"first_name\": \"Hasan\", \"last_name\": \"Rezaei\",\n"
                  "    \"national_code\": \"0022334455\", \"field\": \"Computer Engineering\", \"entrance_year\": \"1398\",\n"
                  "    \"degree\": \"PhD\", \"department\": \"Computer Engineering\", \"ans1\": \"Tabriz\", \"ans2\": \"Hafez\", \"ans3\": \"Red\"\n"
                  "  }\n"
                  "]\n", f);
            fclose(f);
        }
    } else {
        fclose(f);
    }

    f = fopen("courses.json", "r");
    if (!f) {
        f = fopen("courses.json", "w");
        if (f) {
            fputs("[\n"
                  "  {\"course_id\": \"CS101\", \"course_name\": \"Fundamentals of Programming\", \"units\": 3, \"prerequisites\": \"-\", \"section\": \"BSc\", \"field\": \"Computer Engineering\", \"department\": \"Computer Engineering\"},\n"
                  "  {\"course_id\": \"CS201\", \"course_name\": \"Data Structures\", \"units\": 3, \"prerequisites\": \"CS101, CS102\", \"section\": \"BSc\", \"field\": \"Computer Engineering\", \"department\": \"Computer Engineering\"},\n"
                  "  {\"course_id\": \"PHD901\", \"course_name\": \"Doctorate Thesis\", \"units\": 6, \"prerequisites\": \"-\", \"section\": \"PhD\", \"field\": \"General\", \"department\": \"Graduate Studies\"}\n"
                  "]\n", f);
            fclose(f);
        }
    } else {
        fclose(f);
    }

    f = fopen("offerings.json", "r");
    if (!f) {
        f = fopen("offerings.json", "w");
        if (f) {
            fputs("[\n"
                  "  {\"number\": 1, \"course_id\": \"CS201\", \"course_name\": \"Data Structures\", \"faculty_id\": \"FCS101\", \"faculty_name\": \"Dr. Hossein Asadi\", \"semester\": \"14042\", \"capacity\": 35, \"enrollments\": 32, \"department\": \"Computer Engineering\", \"place\": \"Room 405, Science Building\"},\n"
                  "  {\"number\": 2, \"course_id\": \"PHD901\", \"course_name\": \"Doctorate Thesis\", \"faculty_id\": \"FCS101\", \"faculty_name\": \"Dr. Hossein Asadi\", \"semester\": \"14042\", \"capacity\": 10, \"enrollments\": 1, \"department\": \"Graduate Studies\", \"place\": \"Lab 301\"}\n"
                  "]\n", f);
            fclose(f);
        }
    } else {
        fclose(f);
    }

    f = fopen("requests.json", "r");
    if (!f) {
        f = fopen("requests.json", "w");
        if (f) { fputs("[]\n", f); fclose(f); }
    } else fclose(f);

    f = fopen("calendar.json", "r");
    if (!f) {
        f = fopen("calendar.json", "w");
        if (f) {
            fputs("{\n  \"offering\": true,\n  \"unit_selection\": true,\n  \"class_exams\": true,\n  \"grade_recording\": true\n}\n", f);
            fclose(f);
        }
    } else fclose(f);

    f = fopen("lms.json", "r");
    if (!f) {
        f = fopen("lms.json", "w");
        if (f) {
            fputs("{\n  \"homeworks\": [],\n  \"submissions\": [],\n  \"exams\": []\n}\n", f);
            fclose(f);
        }
    } else fclose(f);
}

void parse_json_value(const char* line, const char* key, char* out_val) {
    out_val[0] = '\0';
    char search_key[128];
    snprintf(search_key, sizeof(search_key), "\"%s\":", key);
    char *pos = strstr(line, search_key);
    if (!pos) return;
    pos += strlen(search_key);
    while (*pos == ' ' || *pos == '\"') pos++;
    int i = 0;
    while (*pos != '\0' && *pos != '\"' && *pos != ',' && *pos != '}' && *pos != '\n' && *pos != '\r') {
        out_val[i++] = *pos++;
    }
    out_val[i] = '\0';
}

void load_all_data(void) {
    initialize_data_files();
    FILE *f;
    char line[MAX_BUFFER];

    // Load Students
    g_student_count = 0;
    f = fopen("students.json", "r");
    if (f) {
        Student cur;
        memset(&cur, 0, sizeof(Student));
        bool in_obj = false;
        while (fgets(line, sizeof(line), f)) {
            if (strchr(line, '{')) { in_obj = true; memset(&cur, 0, sizeof(Student)); }
            if (in_obj) {
                char val[256];
                parse_json_value(line, "id", val); if (val[0]) safe_strcpy(cur.id, val, sizeof(cur.id));
                parse_json_value(line, "password", val); if (val[0]) safe_strcpy(cur.password, val, sizeof(cur.password));
                parse_json_value(line, "first_name", val); if (val[0]) safe_strcpy(cur.first_name, val, sizeof(cur.first_name));
                parse_json_value(line, "last_name", val); if (val[0]) safe_strcpy(cur.last_name, val, sizeof(cur.last_name));
                parse_json_value(line, "national_code", val); if (val[0]) safe_strcpy(cur.national_code, val, sizeof(cur.national_code));
                parse_json_value(line, "field", val); if (val[0]) safe_strcpy(cur.field, val, sizeof(cur.field));
                parse_json_value(line, "entrance_year", val); if (val[0]) safe_strcpy(cur.entrance_year, val, sizeof(cur.entrance_year));
                parse_json_value(line, "section", val); if (val[0]) safe_strcpy(cur.section, val, sizeof(cur.section));
                parse_json_value(line, "mentor", val); if (val[0]) safe_strcpy(cur.mentor, val, sizeof(cur.mentor));
                parse_json_value(line, "department", val); if (val[0]) safe_strcpy(cur.department, val, sizeof(cur.department));
                parse_json_value(line, "ans1", val); if (val[0]) safe_strcpy(cur.ans1, val, sizeof(cur.ans1));
                parse_json_value(line, "ans2", val); if (val[0]) safe_strcpy(cur.ans2, val, sizeof(cur.ans2));
                parse_json_value(line, "ans3", val); if (val[0]) safe_strcpy(cur.ans3, val, sizeof(cur.ans3));
                parse_json_value(line, "thesis_title", val); if (val[0]) safe_strcpy(cur.thesis_title, val, sizeof(cur.thesis_title));
                parse_json_value(line, "thesis_abstract", val); if (val[0]) safe_strcpy(cur.thesis_abstract, val, sizeof(cur.thesis_abstract));
                parse_json_value(line, "thesis_citations", val); if (val[0]) cur.thesis_citations = atoi(val);
                parse_json_value(line, "thesis_refs", val); if (val[0]) cur.thesis_refs = atoi(val);

                if (strstr(line, "semester") && strstr(line, "course_id") && strstr(line, "grade")) {
                    char sem[16], cid[32], grd[32];
                    parse_json_value(line, "semester", sem);
                    parse_json_value(line, "course_id", cid);
                    parse_json_value(line, "grade", grd);
                    if (sem[0] && cid[0] && grd[0] && cur.grade_count < 100) {
                        safe_strcpy(cur.grades[cur.grade_count].semester, sem, sizeof(cur.grades[cur.grade_count].semester));
                        safe_strcpy(cur.grades[cur.grade_count].course_id, cid, sizeof(cur.grades[cur.grade_count].course_id));
                        cur.grades[cur.grade_count].grade = atof(grd);
                        cur.grade_count++;
                    }
                }
            }
            if (strchr(line, '}') && in_obj) {
                if (cur.id[0] != '\0' && g_student_count < MAX_ITEMS) g_students[g_student_count++] = cur;
                in_obj = false;
            }
        }
        fclose(f);
    }

    // Load Faculty
    g_faculty_count = 0;
    f = fopen("faculty.json", "r");
    if (f) {
        Faculty cur;
        memset(&cur, 0, sizeof(Faculty));
        bool in_obj = false;
        while (fgets(line, sizeof(line), f)) {
            if (strchr(line, '{')) { in_obj = true; memset(&cur, 0, sizeof(Faculty)); }
            if (in_obj) {
                char val[256];
                parse_json_value(line, "id", val); if (val[0]) safe_strcpy(cur.id, val, sizeof(cur.id));
                parse_json_value(line, "password", val); if (val[0]) safe_strcpy(cur.password, val, sizeof(cur.password));
                parse_json_value(line, "first_name", val); if (val[0]) safe_strcpy(cur.first_name, val, sizeof(cur.first_name));
                parse_json_value(line, "last_name", val); if (val[0]) safe_strcpy(cur.last_name, val, sizeof(cur.last_name));
                parse_json_value(line, "national_code", val); if (val[0]) safe_strcpy(cur.national_code, val, sizeof(cur.national_code));
                parse_json_value(line, "field", val); if (val[0]) safe_strcpy(cur.field, val, sizeof(cur.field));
                parse_json_value(line, "entrance_year", val); if (val[0]) safe_strcpy(cur.entrance_year, val, sizeof(cur.entrance_year));
                parse_json_value(line, "degree", val); if (val[0]) safe_strcpy(cur.degree, val, sizeof(cur.degree));
                parse_json_value(line, "department", val); if (val[0]) safe_strcpy(cur.department, val, sizeof(cur.department));
                parse_json_value(line, "ans1", val); if (val[0]) safe_strcpy(cur.ans1, val, sizeof(cur.ans1));
                parse_json_value(line, "ans2", val); if (val[0]) safe_strcpy(cur.ans2, val, sizeof(cur.ans2));
                parse_json_value(line, "ans3", val); if (val[0]) safe_strcpy(cur.ans3, val, sizeof(cur.ans3));
            }
            if (strchr(line, '}') && in_obj) {
                if (cur.id[0] != '\0' && g_faculty_count < MAX_ITEMS) g_faculty[g_faculty_count++] = cur;
                in_obj = false;
            }
        }
        fclose(f);
    }

    // Load Courses
    g_course_count = 0;
    f = fopen("courses.json", "r");
    if (f) {
        Course cur;
        memset(&cur, 0, sizeof(Course));
        bool in_obj = false;
        while (fgets(line, sizeof(line), f)) {
            if (strchr(line, '{')) { in_obj = true; memset(&cur, 0, sizeof(Course)); }
            if (in_obj) {
                char val[256];
                parse_json_value(line, "course_id", val); if (val[0]) safe_strcpy(cur.course_id, val, sizeof(cur.course_id));
                parse_json_value(line, "course_name", val); if (val[0]) safe_strcpy(cur.course_name, val, sizeof(cur.course_name));
                parse_json_value(line, "units", val); if (val[0]) cur.units = atoi(val);
                parse_json_value(line, "prerequisites", val); if (val[0]) safe_strcpy(cur.prerequisites, val, sizeof(cur.prerequisites));
                parse_json_value(line, "section", val); if (val[0]) safe_strcpy(cur.section, val, sizeof(cur.section));
                parse_json_value(line, "field", val); if (val[0]) safe_strcpy(cur.field, val, sizeof(cur.field));
                parse_json_value(line, "department", val); if (val[0]) safe_strcpy(cur.department, val, sizeof(cur.department));
            }
            if (strchr(line, '}') && in_obj) {
                if (cur.course_id[0] != '\0' && g_course_count < MAX_ITEMS) g_courses[g_course_count++] = cur;
                in_obj = false;
            }
        }
        fclose(f);
    }

    // Load Offerings
    g_offering_count = 0;
    f = fopen("offerings.json", "r");
    if (f) {
        Offering cur;
        memset(&cur, 0, sizeof(Offering));
        bool in_obj = false;
        while (fgets(line, sizeof(line), f)) {
            if (strchr(line, '{')) { in_obj = true; memset(&cur, 0, sizeof(Offering)); }
            if (in_obj) {
                char val[256];
                parse_json_value(line, "number", val); if (val[0]) cur.number = atoi(val);
                parse_json_value(line, "course_id", val); if (val[0]) safe_strcpy(cur.course_id, val, sizeof(cur.course_id));
                parse_json_value(line, "course_name", val); if (val[0]) safe_strcpy(cur.course_name, val, sizeof(cur.course_name));
                parse_json_value(line, "faculty_id", val); if (val[0]) safe_strcpy(cur.faculty_id, val, sizeof(cur.faculty_id));
                parse_json_value(line, "faculty_name", val); if (val[0]) safe_strcpy(cur.faculty_name, val, sizeof(cur.faculty_name));
                parse_json_value(line, "semester", val); if (val[0]) safe_strcpy(cur.semester, val, sizeof(cur.semester));
                parse_json_value(line, "capacity", val); if (val[0]) cur.capacity = atoi(val);
                parse_json_value(line, "enrollments", val); if (val[0]) cur.enrollments = atoi(val);
                parse_json_value(line, "department", val); if (val[0]) safe_strcpy(cur.department, val, sizeof(cur.department));
                parse_json_value(line, "place", val); if (val[0]) safe_strcpy(cur.place, val, sizeof(cur.place));
            }
            if (strchr(line, '}') && in_obj) {
                if (cur.course_id[0] != '\0' && g_offering_count < MAX_ITEMS) g_offerings[g_offering_count++] = cur;
                in_obj = false;
            }
        }
        fclose(f);
    }

    // Load Requests
    g_request_count = 0;
    f = fopen("requests.json", "r");
    if (f) {
        AdminRequest cur;
        memset(&cur, 0, sizeof(AdminRequest));
        bool in_obj = false;
        while (fgets(line, sizeof(line), f)) {
            if (strchr(line, '{')) { in_obj = true; memset(&cur, 0, sizeof(AdminRequest)); }
            if (in_obj) {
                char val[256];
                parse_json_value(line, "id", val); if (val[0]) cur.id = atoi(val);
                parse_json_value(line, "type", val); if (val[0]) safe_strcpy(cur.type, val, sizeof(cur.type));
                parse_json_value(line, "course_name", val); if (val[0]) safe_strcpy(cur.course_name, val, sizeof(cur.course_name));
                parse_json_value(line, "faculty_id", val); if (val[0]) safe_strcpy(cur.faculty_id, val, sizeof(cur.faculty_id));
                parse_json_value(line, "faculty_name", val); if (val[0]) safe_strcpy(cur.faculty_name, val, sizeof(cur.faculty_name));
                parse_json_value(line, "department", val); if (val[0]) safe_strcpy(cur.department, val, sizeof(cur.department));
                parse_json_value(line, "capacity", val); if (val[0]) cur.capacity = atoi(val);
                parse_json_value(line, "enrollments", val); if (val[0]) cur.enrollments = atoi(val);
            }
            if (strchr(line, '}') && in_obj) {
                if (cur.id > 0 && g_request_count < MAX_ITEMS) g_requests[g_request_count++] = cur;
                in_obj = false;
            }
        }
        fclose(f);
    }

    // Load Calendar
    f = fopen("calendar.json", "r");
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            char val[64];
            parse_json_value(line, "offering", val); if (val[0]) g_calendar.offering = (strcmp(val, "true") == 0);
            parse_json_value(line, "unit_selection", val); if (val[0]) g_calendar.unit_selection = (strcmp(val, "true") == 0);
            parse_json_value(line, "class_exams", val); if (val[0]) g_calendar.class_exams = (strcmp(val, "true") == 0);
            parse_json_value(line, "grade_recording", val); if (val[0]) g_calendar.grade_recording = (strcmp(val, "true") == 0);
        }
        fclose(f);
    }

    // Load LMS Data
    g_homework_count = 0; g_submission_count = 0; g_exam_count = 0;
    f = fopen("lms.json", "r");
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "\"hw_id\"")) {
                Homework hw; memset(&hw, 0, sizeof(hw));
                char val[256];
                parse_json_value(line, "hw_id", val); hw.id = atoi(val);
                parse_json_value(line, "offering_num", val); hw.offering_number = atoi(val);
                parse_json_value(line, "title", val); safe_strcpy(hw.title, val, sizeof(hw.title));
                parse_json_value(line, "question", val); safe_strcpy(hw.question, val, sizeof(hw.question));
                parse_json_value(line, "opt1", val); safe_strcpy(hw.opt1, val, sizeof(hw.opt1));
                parse_json_value(line, "opt2", val); safe_strcpy(hw.opt2, val, sizeof(hw.opt2));
                parse_json_value(line, "opt3", val); safe_strcpy(hw.opt3, val, sizeof(hw.opt3));
                parse_json_value(line, "opt4", val); safe_strcpy(hw.opt4, val, sizeof(hw.opt4));
                parse_json_value(line, "correct", val); hw.correct_option = atoi(val);
                if (hw.id > 0 && g_homework_count < MAX_ITEMS) g_homeworks[g_homework_count++] = hw;
            } else if (strstr(line, "\"sub_std\"")) {
                HomeworkSubmission sub; memset(&sub, 0, sizeof(sub));
                char val[256];
                parse_json_value(line, "sub_std", val); safe_strcpy(sub.student_id, val, sizeof(sub.student_id));
                parse_json_value(line, "hw_id", val); sub.homework_id = atoi(val);
                parse_json_value(line, "selected", val); sub.selected_option = atoi(val);
                if (sub.homework_id > 0 && g_submission_count < MAX_ITEMS) g_submissions[g_submission_count++] = sub;
            } else if (strstr(line, "\"exam_id\"")) {
                Exam ex; memset(&ex, 0, sizeof(ex));
                char val[256];
                parse_json_value(line, "exam_id", val); ex.id = atoi(val);
                parse_json_value(line, "offering_num", val); ex.offering_number = atoi(val);
                parse_json_value(line, "title", val); safe_strcpy(ex.title, val, sizeof(ex.title));
                parse_json_value(line, "question", val); safe_strcpy(ex.question, val, sizeof(ex.question));
                parse_json_value(line, "max_score", val); ex.max_score = atoi(val);
                if (ex.id > 0 && g_exam_count < MAX_ITEMS) g_exams[g_exam_count++] = ex;
            }
        }
        fclose(f);
    }
}

void save_all_data(void) {
    FILE *f;

    // Save Students
    f = fopen("students.json", "w");
    if (f) {
        fprintf(f, "[\n");
        for (int i = 0; i < g_student_count; i++) {
            fprintf(f, "  {\n");
            fprintf(f, "    \"id\": \"%s\",\n", g_students[i].id);
            fprintf(f, "    \"password\": \"%s\",\n", g_students[i].password);
            fprintf(f, "    \"first_name\": \"%s\",\n", g_students[i].first_name);
            fprintf(f, "    \"last_name\": \"%s\",\n", g_students[i].last_name);
            fprintf(f, "    \"national_code\": \"%s\",\n", g_students[i].national_code);
            fprintf(f, "    \"field\": \"%s\",\n", g_students[i].field);
            fprintf(f, "    \"entrance_year\": \"%s\",\n", g_students[i].entrance_year);
            fprintf(f, "    \"section\": \"%s\",\n", g_students[i].section);
            fprintf(f, "    \"mentor\": \"%s\",\n", g_students[i].mentor);
            fprintf(f, "    \"department\": \"%s\",\n", g_students[i].department);
            fprintf(f, "    \"ans1\": \"%s\",\n", g_students[i].ans1);
            fprintf(f, "    \"ans2\": \"%s\",\n", g_students[i].ans2);
            fprintf(f, "    \"ans3\": \"%s\",\n", g_students[i].ans3);
            fprintf(f, "    \"thesis_title\": \"%s\",\n", g_students[i].thesis_title);
            fprintf(f, "    \"thesis_abstract\": \"%s\",\n", g_students[i].thesis_abstract);
            fprintf(f, "    \"thesis_citations\": %d,\n", g_students[i].thesis_citations);
            fprintf(f, "    \"thesis_refs\": %d,\n", g_students[i].thesis_refs);
            fprintf(f, "    \"grades\": [\n");
            for (int j = 0; j < g_students[i].grade_count; j++) {
                fprintf(f, "      {\"semester\": \"%s\", \"course_id\": \"%s\", \"grade\": %.2f}%s\n",
                        g_students[i].grades[j].semester,
                        g_students[i].grades[j].course_id,
                        g_students[i].grades[j].grade,
                        (j == g_students[i].grade_count - 1) ? "" : ",");
            }
            fprintf(f, "    ]\n");
            fprintf(f, "  }%s\n", (i == g_student_count - 1) ? "" : ",");
        }
        fprintf(f, "]\n");
        fclose(f);
    }

    // Save Faculty
    f = fopen("faculty.json", "w");
    if (f) {
        fprintf(f, "[\n");
        for (int i = 0; i < g_faculty_count; i++) {
            fprintf(f, "  {\n");
            fprintf(f, "    \"id\": \"%s\",\n", g_faculty[i].id);
            fprintf(f, "    \"password\": \"%s\",\n", g_faculty[i].password);
            fprintf(f, "    \"first_name\": \"%s\",\n", g_faculty[i].first_name);
            fprintf(f, "    \"last_name\": \"%s\",\n", g_faculty[i].last_name);
            fprintf(f, "    \"national_code\": \"%s\",\n", g_faculty[i].national_code);
            fprintf(f, "    \"field\": \"%s\",\n", g_faculty[i].field);
            fprintf(f, "    \"entrance_year\": \"%s\",\n", g_faculty[i].entrance_year);
            fprintf(f, "    \"degree\": \"%s\",\n", g_faculty[i].degree);
            fprintf(f, "    \"department\": \"%s\",\n", g_faculty[i].department);
            fprintf(f, "    \"ans1\": \"%s\",\n", g_faculty[i].ans1);
            fprintf(f, "    \"ans2\": \"%s\",\n", g_faculty[i].ans2);
            fprintf(f, "    \"ans3\": \"%s\"\n", g_faculty[i].ans3);
            fprintf(f, "  }%s\n", (i == g_faculty_count - 1) ? "" : ",");
        }
        fprintf(f, "]\n");
        fclose(f);
    }

    // Save Calendar
    f = fopen("calendar.json", "w");
    if (f) {
        fprintf(f, "{\n");
        fprintf(f, "  \"offering\": %s,\n", g_calendar.offering ? "true" : "false");
        fprintf(f, "  \"unit_selection\": %s,\n", g_calendar.unit_selection ? "true" : "false");
        fprintf(f, "  \"class_exams\": %s,\n", g_calendar.class_exams ? "true" : "false");
        fprintf(f, "  \"grade_recording\": %s\n", g_calendar.grade_recording ? "true" : "false");
        fprintf(f, "}\n");
        fclose(f);
    }

    // Save Requests
    f = fopen("requests.json", "w");
    if (f) {
        fprintf(f, "[\n");
        for (int i = 0; i < g_request_count; i++) {
            fprintf(f, "  {\"id\": %d, \"type\": \"%s\", \"course_name\": \"%s\", \"faculty_id\": \"%s\", \"faculty_name\": \"%s\", \"department\": \"%s\", \"capacity\": %d, \"enrollments\": %d}%s\n",
                    g_requests[i].id, g_requests[i].type, g_requests[i].course_name,
                    g_requests[i].faculty_id, g_requests[i].faculty_name, g_requests[i].department,
                    g_requests[i].capacity, g_requests[i].enrollments,
                    (i == g_request_count - 1) ? "" : ",");
        }
        fprintf(f, "]\n");
        fclose(f);
    }

    // Save Offerings
    f = fopen("offerings.json", "w");
    if (f) {
        fprintf(f, "[\n");
        for (int i = 0; i < g_offering_count; i++) {
            fprintf(f, "  {\"number\": %d, \"course_id\": \"%s\", \"course_name\": \"%s\", \"faculty_id\": \"%s\", \"faculty_name\": \"%s\", \"semester\": \"%s\", \"capacity\": %d, \"enrollments\": %d, \"department\": \"%s\", \"place\": \"%s\"}%s\n",
                    g_offerings[i].number, g_offerings[i].course_id, g_offerings[i].course_name,
                    g_offerings[i].faculty_id, g_offerings[i].faculty_name, g_offerings[i].semester,
                    g_offerings[i].capacity, g_offerings[i].enrollments,
                    g_offerings[i].department, g_offerings[i].place,
                    (i == g_offering_count - 1) ? "" : ",");
        }
        fprintf(f, "]\n");
        fclose(f);
    }

    // Save Courses
    f = fopen("courses.json", "w");
    if (f) {
        fprintf(f, "[\n");
        for (int i = 0; i < g_course_count; i++) {
            fprintf(f, "  {\"course_id\": \"%s\", \"course_name\": \"%s\", \"units\": %d, \"prerequisites\": \"%s\", \"section\": \"%s\", \"field\": \"%s\", \"department\": \"%s\"}%s\n",
                    g_courses[i].course_id, g_courses[i].course_name, g_courses[i].units,
                    g_courses[i].prerequisites, g_courses[i].section, g_courses[i].field,
                    g_courses[i].department, (i == g_course_count - 1) ? "" : ",");
        }
        fprintf(f, "]\n");
        fclose(f);
    }

    // Save LMS Data
    f = fopen("lms.json", "w");
    if (f) {
        fprintf(f, "{\n  \"homeworks\": [\n");
        for (int i = 0; i < g_homework_count; i++) {
            fprintf(f, "    {\"hw_id\": %d, \"offering_num\": %d, \"title\": \"%s\", \"question\": \"%s\", \"opt1\": \"%s\", \"opt2\": \"%s\", \"opt3\": \"%s\", \"opt4\": \"%s\", \"correct\": %d}%s\n",
                    g_homeworks[i].id, g_homeworks[i].offering_number, g_homeworks[i].title, g_homeworks[i].question,
                    g_homeworks[i].opt1, g_homeworks[i].opt2, g_homeworks[i].opt3, g_homeworks[i].opt4, g_homeworks[i].correct_option,
                    (i == g_homework_count - 1) ? "" : ",");
        }
        fprintf(f, "  ],\n  \"submissions\": [\n");
        for (int i = 0; i < g_submission_count; i++) {
            fprintf(f, "    {\"sub_std\": \"%s\", \"hw_id\": %d, \"selected\": %d}%s\n",
                    g_submissions[i].student_id, g_submissions[i].homework_id, g_submissions[i].selected_option,
                    (i == g_submission_count - 1) ? "" : ",");
        }
        fprintf(f, "  ],\n  \"exams\": [\n");
        for (int i = 0; i < g_exam_count; i++) {
            fprintf(f, "    {\"exam_id\": %d, \"offering_num\": %d, \"title\": \"%s\", \"question\": \"%s\", \"max_score\": %d}%s\n",
                    g_exams[i].id, g_exams[i].offering_number, g_exams[i].title, g_exams[i].question, g_exams[i].max_score,
                    (i == g_exam_count - 1) ? "" : ",");
        }
        fprintf(f, "  ]\n}\n");
        fclose(f);
    }
}

/* Authentication Flows */

void login_flow(int role) {
    char username[64], password[64];
    while (1) {
        clear_screen();
        printf("Enter your username: ");
        get_input(username, sizeof(username));

        if (role == 3) {
            if (strcmp(username, "admin") != 0) {
                printf("Username not found.\n");
                press_enter_to_continue();
                return;
            }
            printf("Enter password: ");
            get_input(password, sizeof(password));
            if (strcmp(password, "admin") == 0) {
                safe_strcpy(g_current_user_id, "admin", sizeof(g_current_user_id));
                g_current_role = 3;
                admin_dashboard();
                return;
            } else {
                printf("Incorrect password.\n");
                press_enter_to_continue();
                return;
            }
        } else if (role == 1) {
            int idx = -1;
            for (int i = 0; i < g_student_count; i++) {
                if (strcmp(g_students[i].id, username) == 0) { idx = i; break; }
            }
            if (idx == -1) {
                printf("Username not found.\n");
                press_enter_to_continue();
                return;
            }
            printf("Enter password: ");
            get_input(password, sizeof(password));
            if (strcmp(g_students[idx].password, password) == 0) {
                safe_strcpy(g_current_user_id, username, sizeof(g_current_user_id));
                g_current_role = 1;
                student_dashboard();
                return;
            } else {
                printf("Incorrect password.\n");
                press_enter_to_continue();
                return;
            }
        } else if (role == 2) {
            int idx = -1;
            for (int i = 0; i < g_faculty_count; i++) {
                if (strcmp(g_faculty[i].id, username) == 0) { idx = i; break; }
            }
            if (idx == -1) {
                printf("Username not found.\n");
                press_enter_to_continue();
                return;
            }
            printf("Enter password: ");
            get_input(password, sizeof(password));
            if (strcmp(g_faculty[idx].password, password) == 0) {
                safe_strcpy(g_current_user_id, username, sizeof(g_current_user_id));
                g_current_role = 2;
                faculty_dashboard();
                return;
            } else {
                printf("Incorrect password.\n");
                press_enter_to_continue();
                return;
            }
        }
    }
}

void forgot_password_flow(void) {
    char username[64], ans1[64], ans2[64], ans3[64], pass1[64], pass2[64], opt[16];
    int user_idx = -1;
    int user_type = 0;

    while (1) {
        clear_screen();
        printf("Enter your username: ");
        get_input(username, sizeof(username));

        user_idx = -1;
        for (int i = 0; i < g_student_count; i++) {
            if (strcmp(g_students[i].id, username) == 0) { user_idx = i; user_type = 1; break; }
        }
        if (user_idx == -1) {
            for (int i = 0; i < g_faculty_count; i++) {
                if (strcmp(g_faculty[i].id, username) == 0) { user_idx = i; user_type = 2; break; }
            }
        }

        if (user_idx == -1) {
            printf("Username not found.\n");
            printf("1. Retry\n2. Go to login menu\nEnter an option: ");
            get_input(opt, sizeof(opt));
            if (strcmp(opt, "1") == 0) continue;
            else return;
        }
        break;
    }

    char *correct_ans1 = (user_type == 1) ? g_students[user_idx].ans1 : g_faculty[user_idx].ans1;
    char *correct_ans2 = (user_type == 1) ? g_students[user_idx].ans2 : g_faculty[user_idx].ans2;
    char *correct_ans3 = (user_type == 1) ? g_students[user_idx].ans3 : g_faculty[user_idx].ans3;

    while (1) {
        printf("Where were you born? ");
        get_input(ans1, sizeof(ans1));
        if (my_strcasecmp(ans1, correct_ans1) != 0) {
            printf("Incorrect answer.\n1. Retry\n2. Go to login menu\nEnter an option: ");
            get_input(opt, sizeof(opt));
            if (strcmp(opt, "1") == 0) continue;
            else return;
        }
        break;
    }

    printf("What was the title of the first book you read? "); get_input(ans2, sizeof(ans2));
    printf("What was the color of your first bicycle? "); get_input(ans3, sizeof(ans3));

    if (my_strcasecmp(ans2, correct_ans2) != 0 || my_strcasecmp(ans3, correct_ans3) != 0) {
        printf("Incorrect answer.\n1. Retry\n2. Go to login menu\nEnter an option: ");
        get_input(opt, sizeof(opt));
        if (strcmp(opt, "1") == 0) { forgot_password_flow(); return; }
        else return;
    }

    printf("Authentication successful.\n\n");

    while (1) {
        printf("Enter your new password: "); get_input(pass1, sizeof(pass1));
        printf("Confirm your password: "); get_input(pass2, sizeof(pass2));

        if (strcmp(pass1, pass2) != 0) {
            printf("Passwords aren’t matching.\n1. Retry.\n2. Cancel (go to login menu).\nEnter an option: ");
            get_input(opt, sizeof(opt));
            if (strcmp(opt, "1") == 0) continue;
            else return;
        } else {
            if (user_type == 1) safe_strcpy(g_students[user_idx].password, pass1, sizeof(g_students[user_idx].password));
            else safe_strcpy(g_faculty[user_idx].password, pass1, sizeof(g_faculty[user_idx].password));
            save_all_data();
            printf("Password changed successfully.\n");
            press_enter_to_continue();
            return;
        }
    }
}

/* Shared Menu Flows */

void courses_menu_flow(int role) {
    char opt[16];
    while (1) {
        clear_screen();
        if (role == 3) printf("Admin: Courses\n");
        else if (role == 2) printf("Faculty: List of courses\n");
        else printf("Student: Courses\n");

        printf("List of courses\n");
        printf("| course name | course id | units | prerequisites | section | field | department |\n");
        printf("|-------------|-----------|-------|---------------|---------|-------|------------|\n");
        for (int i = 0; i < g_course_count; i++) {
            printf("| %s | %s | %d | %s | %s | %s | %s |\n",
                   g_courses[i].course_name, g_courses[i].course_id, g_courses[i].units,
                   g_courses[i].prerequisites, g_courses[i].section, g_courses[i].field, g_courses[i].department);
        }

        if (role == 3) printf("\n1. Search\n2. Add a course\n3. Remove a course\n4. Go back\nEnter an option: ");
        else printf("\n1. Search\n2. Go back\nEnter an option: ");

        get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) {
            char query[64]; printf("The phrase to search: "); get_input(query, sizeof(query));
            printf("\nSearch Results:\n");
            for (int i = 0; i < g_course_count; i++) {
                if (my_strcasestr_contains(g_courses[i].course_name, query) ||
                    my_strcasestr_contains(g_courses[i].course_id, query)) {
                    printf("| %s | %s | Units: %d |\n", g_courses[i].course_name, g_courses[i].course_id, g_courses[i].units);
                }
            }
            press_enter_to_continue();
        } else if (role == 3 && strcmp(opt, "2") == 0) {
            if (g_course_count >= MAX_ITEMS) { printf("Storage full.\n"); press_enter_to_continue(); continue; }
            Course c; memset(&c, 0, sizeof(Course));
            printf("Enter Course ID: "); get_input(c.course_id, sizeof(c.course_id));
            printf("Enter Course Name: "); get_input(c.course_name, sizeof(c.course_name));
            char ubuf[16]; printf("Enter Units: "); get_input(ubuf, sizeof(ubuf)); c.units = atoi(ubuf);
            printf("Enter Prerequisites: "); get_input(c.prerequisites, sizeof(c.prerequisites));
            printf("Enter Section: "); get_input(c.section, sizeof(c.section));
            printf("Enter Field: "); get_input(c.field, sizeof(c.field));
            printf("Enter Department: "); get_input(c.department, sizeof(c.department));
            g_courses[g_course_count++] = c; save_all_data();
            printf("Course added successfully.\n"); press_enter_to_continue();
        } else if (role == 3 && strcmp(opt, "3") == 0) {
            char cid[32]; printf("Enter course id to remove: "); get_input(cid, sizeof(cid));
            int idx = -1;
            for (int i = 0; i < g_course_count; i++) {
                if (strcmp(g_courses[i].course_id, cid) == 0) { idx = i; break; }
            }
            if (idx != -1) {
                for (int i = idx; i < g_course_count - 1; i++) g_courses[i] = g_courses[i + 1];
                g_course_count--; save_all_data();
                printf("Course removed successfully.\n");
            } else printf("Course ID not found.\n");
            press_enter_to_continue();
        } else if ((role == 3 && strcmp(opt, "4") == 0) || (role != 3 && strcmp(opt, "2") == 0)) break;
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
    }
}

void offerings_menu_flow(int role) {
    char sem[16], opt[16];
    clear_screen();
    if (role == 3) printf("Admin: Offerings\n");
    else if (role == 2) printf("Faculty: List of offerings in semester\n");
    else printf("Student: Offerings\n");

    printf("Enter semester number: "); get_input(sem, sizeof(sem));
    if (strlen(sem) == 0) safe_strcpy(sem, "14042", sizeof(sem));

    while (1) {
        clear_screen();
        if (role == 3) printf("Admin: Offerings\n");
        else if (role == 2) printf("Faculty: List of offerings in semester\n");
        else printf("Student: Offerings\n");

        printf("\nList of offerings - %s\n", sem);
        printf("| number | course name | course id | faculty | semester | capacity | no. enrollments | department | place |\n");
        printf("|--------|-------------|-----------|---------|----------|----------|-----------------|------------|-------|\n");
        for (int i = 0; i < g_offering_count; i++) {
            if (strcmp(g_offerings[i].semester, sem) == 0) {
                printf("| %d | %s | %s | %s | %s | %d | %d | %s | %s |\n",
                       g_offerings[i].number, g_offerings[i].course_name, g_offerings[i].course_id,
                       g_offerings[i].faculty_name, g_offerings[i].semester, g_offerings[i].capacity,
                       g_offerings[i].enrollments, g_offerings[i].department, g_offerings[i].place);
            }
        }

        if (role == 1) printf("\n1. Search\n2. Enroll in course\n3. Withdraw course\n4. Go back\nEnter an option: ");
        else if (role == 3) printf("\n1. Search\n2. Add student to an offering\n3. Remove student from an offering\n4. Go back\nEnter an option: ");
        else printf("\n1. Search\n2. Go back\nEnter an option: ");

        get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) {
            char query[64]; printf("The phrase to search: "); get_input(query, sizeof(query));
            printf("\nSearch Results:\n");
            for (int i = 0; i < g_offering_count; i++) {
                if (strcmp(g_offerings[i].semester, sem) == 0 &&
                    (my_strcasestr_contains(g_offerings[i].course_name, query) ||
                     my_strcasestr_contains(g_offerings[i].faculty_name, query))) {
                    printf("| %d | %s | %s | Capacity: %d |\n", g_offerings[i].number, g_offerings[i].course_name, g_offerings[i].faculty_name, g_offerings[i].capacity);
                }
            }
            press_enter_to_continue();
        } else if (role == 1 && strcmp(opt, "2") == 0) { // Student Enroll
            if (!g_calendar.unit_selection) { printf("Unit selection period is currently disabled.\n"); press_enter_to_continue(); continue; }
            printf("Enter offering number to enroll: "); get_input(opt, sizeof(opt));
            int off_num = atoi(opt);
            int idx = -1;
            for (int i = 0; i < g_offering_count; i++) {
                if (g_offerings[i].number == off_num && strcmp(g_offerings[i].semester, sem) == 0) { idx = i; break; }
            }
            if (idx != -1) {
                // PhD Mentor check
                int s_idx = -1;
                for (int s = 0; s < g_student_count; s++) {
                    if (strcmp(g_students[s].id, g_current_user_id) == 0) { s_idx = s; break; }
                }
                if (s_idx != -1 && my_strcasestr_contains(g_offerings[idx].course_name, "Thesis")) {
                    if (!my_strcasestr_contains(g_offerings[idx].faculty_name, g_students[s_idx].mentor)) {
                        printf("Error: PhD students can only take thesis with their assigned mentor (%s).\n", g_students[s_idx].mentor);
                        press_enter_to_continue();
                        continue;
                    }
                }

                if (g_offerings[idx].enrollments >= g_offerings[idx].capacity) printf("Class is full.\n");
                else {
                    g_offerings[idx].enrollments++;
                    if (s_idx != -1 && g_students[s_idx].grade_count < 100) {
                        safe_strcpy(g_students[s_idx].grades[g_students[s_idx].grade_count].semester, sem, sizeof(g_students[s_idx].grades[0].semester));
                        safe_strcpy(g_students[s_idx].grades[g_students[s_idx].grade_count].course_id, g_offerings[idx].course_id, sizeof(g_students[s_idx].grades[0].course_id));
                        g_students[s_idx].grades[g_students[s_idx].grade_count].grade = -1.0;
                        g_students[s_idx].grade_count++;
                    }
                    save_all_data();
                    printf("Enrolled successfully.\n");
                }
            } else printf("Offering not found.\n");
            press_enter_to_continue();
        } else if (role == 1 && strcmp(opt, "3") == 0) { // Student Withdraw
            if (!g_calendar.unit_selection) { printf("Unit selection period is currently disabled.\n"); press_enter_to_continue(); continue; }
            printf("Enter offering number to withdraw: "); get_input(opt, sizeof(opt));
            int off_num = atoi(opt);
            int idx = -1;
            for (int i = 0; i < g_offering_count; i++) {
                if (g_offerings[i].number == off_num && strcmp(g_offerings[i].semester, sem) == 0) { idx = i; break; }
            }
            if (idx != -1) {
                if (g_offerings[idx].enrollments > 0) g_offerings[idx].enrollments--;
                int s_idx = -1;
                for (int s = 0; s < g_student_count; s++) {
                    if (strcmp(g_students[s].id, g_current_user_id) == 0) { s_idx = s; break; }
                }
                if (s_idx != -1) {
                    for (int g = 0; g < g_students[s_idx].grade_count; g++) {
                        if (strcmp(g_students[s_idx].grades[g].course_id, g_offerings[idx].course_id) == 0 &&
                            strcmp(g_students[s_idx].grades[g].semester, sem) == 0) {
                            for (int k = g; k < g_students[s_idx].grade_count - 1; k++) g_students[s_idx].grades[k] = g_students[s_idx].grades[k + 1];
                            g_students[s_idx].grade_count--;
                            break;
                        }
                    }
                }
                save_all_data();
                printf("Withdrawn successfully.\n");
            } else printf("Offering not found.\n");
            press_enter_to_continue();
        } else if (role == 3 && strcmp(opt, "2") == 0) {
            printf("Enter offering number: "); get_input(opt, sizeof(opt));
            int off_num = atoi(opt);
            int idx = -1;
            for (int i = 0; i < g_offering_count; i++) { if (g_offerings[i].number == off_num) { idx = i; break; } }
            if (idx != -1) { g_offerings[idx].enrollments++; save_all_data(); printf("Student added.\n"); }
            else printf("Offering not found.\n");
            press_enter_to_continue();
        } else if (role == 3 && strcmp(opt, "3") == 0) {
            printf("Enter offering number: "); get_input(opt, sizeof(opt));
            int off_num = atoi(opt);
            int idx = -1;
            for (int i = 0; i < g_offering_count; i++) { if (g_offerings[i].number == off_num) { idx = i; break; } }
            if (idx != -1) { if (g_offerings[idx].enrollments > 0) g_offerings[idx].enrollments--; save_all_data(); printf("Student removed.\n"); }
            else printf("Offering not found.\n");
            press_enter_to_continue();
        } else if ((role == 2 && strcmp(opt, "2") == 0) || (role != 2 && strcmp(opt, "4") == 0)) break;
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
    }
}

/* ADMIN PANELS */

void admin_calendar_menu(void) {
    char opt[16];
    while (1) {
        clear_screen();
        printf("Admin:Calendar\n");
        printf("1. offering:         %s\n", g_calendar.offering ? "enabled" : "disabled");
        printf("2. unit selection:   %s\n", g_calendar.unit_selection ? "enabled" : "disabled");
        printf("3. class & exams:    %s\n", g_calendar.class_exams ? "enabled" : "disabled");
        printf("4. grade recording:  %s\n", g_calendar.grade_recording ? "enabled" : "disabled");
        printf("5. go to main menu\n\nEnter a time to trigger: ");
        get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) g_calendar.offering = !g_calendar.offering;
        else if (strcmp(opt, "2") == 0) g_calendar.unit_selection = !g_calendar.unit_selection;
        else if (strcmp(opt, "3") == 0) g_calendar.class_exams = !g_calendar.class_exams;
        else if (strcmp(opt, "4") == 0) g_calendar.grade_recording = !g_calendar.grade_recording;
        else if (strcmp(opt, "5") == 0) break;
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); continue; }
        save_all_data();
    }
}

void admin_students_menu(void) {
    char opt[16];
    while (1) {
        clear_screen();
        printf("Admin: Students\n1. students list\n2. register student(s)\n3. remove student(s)\n4. go back\nEnter an option: ");
        get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) {
            while (1) {
                clear_screen();
                printf("Admin: Students: students list\nStudents list\n");
                printf("|first name |last name |student id |national code |field |entrance year |section |mentor |department |\n");
                printf("|-----------|----------|-----------|--------------|------|--------------|--------|-------|------------|\n");
                for (int i = 0; i < g_student_count; i++) {
                    printf("|%s |%s |%s|%s|%s|%s|%s|%s |%s|\n",
                           g_students[i].first_name, g_students[i].last_name, g_students[i].id,
                           g_students[i].national_code, g_students[i].field, g_students[i].entrance_year,
                           g_students[i].section, g_students[i].mentor, g_students[i].department);
                }
                printf("\n1. search\n2. go back\nEnter an option: ");
                get_input(opt, sizeof(opt));
                if (strcmp(opt, "1") == 0) {
                    char query[64]; printf("The phrase to search: "); get_input(query, sizeof(query));
                    printf("\nSearch Results:\n");
                    for (int i = 0; i < g_student_count; i++) {
                        if (my_strcasestr_contains(g_students[i].first_name, query) ||
                            my_strcasestr_contains(g_students[i].last_name, query) ||
                            my_strcasestr_contains(g_students[i].id, query)) {
                            printf("| %s %s | ID: %s | Field: %s |\n", g_students[i].first_name, g_students[i].last_name, g_students[i].id, g_students[i].field);
                        }
                    }
                    press_enter_to_continue();
                } else if (strcmp(opt, "2") == 0) break;
                else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
            }
        } else if (strcmp(opt, "2") == 0) {
            if (g_student_count >= MAX_ITEMS) { printf("Storage full.\n"); press_enter_to_continue(); continue; }
            printf("Admin: Students: register student(s)\n1. Register one student\n2. Import file\nEnter an option: ");
            get_input(opt, sizeof(opt));
            if (strcmp(opt, "1") == 0) {
                Student s; memset(&s, 0, sizeof(Student));
                printf("Enter Student ID: "); get_input(s.id, sizeof(s.id));
                printf("Enter First Name: "); get_input(s.first_name, sizeof(s.first_name));
                printf("Enter Last Name: "); get_input(s.last_name, sizeof(s.last_name));
                printf("Enter National Code: "); get_input(s.national_code, sizeof(s.national_code));
                printf("Enter Field: "); get_input(s.field, sizeof(s.field));
                printf("Enter Entrance Year: "); get_input(s.entrance_year, sizeof(s.entrance_year));
                printf("Enter Section (BSc/MSc/PhD): "); get_input(s.section, sizeof(s.section));
                printf("Enter Mentor: "); get_input(s.mentor, sizeof(s.mentor));
                printf("Enter Department: "); get_input(s.department, sizeof(s.department));
                printf("Enter Security Answer 1: "); get_input(s.ans1, sizeof(s.ans1));
                printf("Enter Security Answer 2: "); get_input(s.ans2, sizeof(s.ans2));
                printf("Enter Security Answer 3: "); get_input(s.ans3, sizeof(s.ans3));
                safe_strcpy(s.password, "pass", sizeof(s.password));
                g_students[g_student_count++] = s; save_all_data();
                printf("Student registered.\n"); press_enter_to_continue();
            }
        } else if (strcmp(opt, "3") == 0) {
            printf("Enter student id: "); char sid[32]; get_input(sid, sizeof(sid));
            int idx = -1;
            for (int i = 0; i < g_student_count; i++) { if (strcmp(g_students[i].id, sid) == 0) { idx = i; break; } }
            if (idx != -1) {
                printf("Remove student %s %s? [y/n] ", g_students[idx].first_name, g_students[idx].last_name);
                get_input(opt, sizeof(opt));
                if (strcmp(opt, "y") == 0 || strcmp(opt, "Y") == 0) {
                    for (int i = idx; i < g_student_count - 1; i++) g_students[i] = g_students[i + 1];
                    g_student_count--; save_all_data(); printf("Student removed.\n");
                }
            } else printf("Student not found.\n");
            press_enter_to_continue();
        } else if (strcmp(opt, "4") == 0) break;
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
    }
}

void admin_faculty_menu(void) {
    char opt[16];
    while (1) {
        clear_screen();
        printf("Admin: Faculty members\n1. faculty list\n2. register faculty\n3. remove faculty\n4. go back\nEnter an option: ");
        get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) {
            while (1) {
                clear_screen();
                printf("Admin: Faculty: faculty list\nFaculty list\n");
                printf("|first name |last name |faculty id |national code |field |degree |department |\n");
                printf("|-----------|----------|-----------|--------------|------|-------|------------|\n");
                for (int i = 0; i < g_faculty_count; i++) {
                    printf("|%s |%s |%s|%s|%s|%s|%s|\n",
                           g_faculty[i].first_name, g_faculty[i].last_name, g_faculty[i].id,
                           g_faculty[i].national_code, g_faculty[i].field, g_faculty[i].degree, g_faculty[i].department);
                }
                printf("\n1. search\n2. go back\nEnter an option: ");
                get_input(opt, sizeof(opt));
                if (strcmp(opt, "1") == 0) {
                    char query[64]; printf("The phrase to search: "); get_input(query, sizeof(query));
                    printf("\nSearch Results:\n");
                    for (int i = 0; i < g_faculty_count; i++) {
                        if (my_strcasestr_contains(g_faculty[i].first_name, query) ||
                            my_strcasestr_contains(g_faculty[i].last_name, query) ||
                            my_strcasestr_contains(g_faculty[i].id, query)) {
                            printf("| Dr. %s %s | ID: %s | Department: %s |\n", g_faculty[i].first_name, g_faculty[i].last_name, g_faculty[i].id, g_faculty[i].department);
                        }
                    }
                    press_enter_to_continue();
                } else if (strcmp(opt, "2") == 0) break;
                else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
            }
        } else if (strcmp(opt, "2") == 0) {
            if (g_faculty_count >= MAX_ITEMS) { printf("Storage full.\n"); press_enter_to_continue(); continue; }
            Faculty f; memset(&f, 0, sizeof(Faculty));
            printf("Enter Faculty ID: "); get_input(f.id, sizeof(f.id));
            printf("Enter First Name: "); get_input(f.first_name, sizeof(f.first_name));
            printf("Enter Last Name: "); get_input(f.last_name, sizeof(f.last_name));
            printf("Enter National Code: "); get_input(f.national_code, sizeof(f.national_code));
            printf("Enter Field: "); get_input(f.field, sizeof(f.field));
            printf("Enter Entrance Year: "); get_input(f.entrance_year, sizeof(f.entrance_year));
            printf("Enter Degree: "); get_input(f.degree, sizeof(f.degree));
            printf("Enter Department: "); get_input(f.department, sizeof(f.department));
            printf("Enter Security Answer 1: "); get_input(f.ans1, sizeof(f.ans1));
            printf("Enter Security Answer 2: "); get_input(f.ans2, sizeof(f.ans2));
            printf("Enter Security Answer 3: "); get_input(f.ans3, sizeof(f.ans3));
            safe_strcpy(f.password, "pass", sizeof(f.password));
            g_faculty[g_faculty_count++] = f; save_all_data();
            printf("Faculty member registered.\n"); press_enter_to_continue();
        } else if (strcmp(opt, "3") == 0) {
            printf("Enter faculty id: "); char fid[32]; get_input(fid, sizeof(fid));
            int idx = -1;
            for (int i = 0; i < g_faculty_count; i++) { if (strcmp(g_faculty[i].id, fid) == 0) { idx = i; break; } }
            if (idx != -1) {
                printf("Remove Dr. %s %s? [y/n] ", g_faculty[idx].first_name, g_faculty[idx].last_name);
                get_input(opt, sizeof(opt));
                if (strcmp(opt, "y") == 0 || strcmp(opt, "Y") == 0) {
                    for (int i = idx; i < g_faculty_count - 1; i++) g_faculty[i] = g_faculty[i + 1];
                    g_faculty_count--; save_all_data(); printf("Faculty removed.\n");
                }
            } else printf("Faculty not found.\n");
            press_enter_to_continue();
        } else if (strcmp(opt, "4") == 0) break;
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
    }
}

void admin_requests_menu(void) {
    char opt[16];
    while (1) {
        clear_screen();
        printf("Admin: Requests\nList of requests\n");
        if (g_request_count == 0) printf("No pending requests.\n");
        else {
            for (int i = 0; i < g_request_count; i++) {
                printf("%d. %s\n   Course: %s\n   Faculty: %s\n   Department: %s\n   Capacity: %d\n",
                       g_requests[i].id, g_requests[i].type, g_requests[i].course_name,
                       g_requests[i].faculty_name, g_requests[i].department, g_requests[i].capacity);
            }
        }
        printf("\n1. Go to request number\n2. Go back\nEnter an option: ");
        get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) {
            printf("Enter request number: "); get_input(opt, sizeof(opt));
            int req_num = atoi(opt);
            int idx = -1;
            for (int i = 0; i < g_request_count; i++) { if (g_requests[i].id == req_num) { idx = i; break; } }
            if (idx != -1) {
                printf("1. Approve\n2. Reject\nEnter an option: "); get_input(opt, sizeof(opt));
                if (strcmp(opt, "1") == 0) {
                    if (strcmp(g_requests[idx].type, "course offering") == 0) {
                        if (g_offering_count < MAX_ITEMS) {
                            Offering o; memset(&o, 0, sizeof(Offering));
                            int max_num = 0;
                            for (int k = 0; k < g_offering_count; k++) if (g_offerings[k].number > max_num) max_num = g_offerings[k].number;
                            o.number = max_num + 1;
                            safe_strcpy(o.course_name, g_requests[idx].course_name, sizeof(o.course_name));
                            safe_strcpy(o.course_id, "CS101", sizeof(o.course_id));
                            for (int c = 0; c < g_course_count; c++) {
                                if (my_strcasecmp(g_courses[c].course_name, g_requests[idx].course_name) == 0) {
                                    safe_strcpy(o.course_id, g_courses[c].course_id, sizeof(o.course_id));
                                    break;
                                }
                            }
                            safe_strcpy(o.faculty_id, g_requests[idx].faculty_id, sizeof(o.faculty_id));
                            safe_strcpy(o.faculty_name, g_requests[idx].faculty_name, sizeof(o.faculty_name));
                            safe_strcpy(o.semester, "14042", sizeof(o.semester));
                            o.capacity = g_requests[idx].capacity;
                            o.enrollments = 0;
                            safe_strcpy(o.department, g_requests[idx].department, sizeof(o.department));
                            safe_strcpy(o.place, "Room 101, Science Building", sizeof(o.place));
                            g_offerings[g_offering_count++] = o;
                        }
                    } else if (strcmp(g_requests[idx].type, "capacity increment") == 0) {
                        for (int k = 0; k < g_offering_count; k++) {
                            if (my_strcasecmp(g_offerings[k].course_name, g_requests[idx].course_name) == 0) {
                                g_offerings[k].capacity = g_requests[idx].capacity; break;
                            }
                        }
                    } else if (strcmp(g_requests[idx].type, "course removing") == 0) {
                        for (int k = 0; k < g_offering_count; k++) {
                            if (my_strcasecmp(g_offerings[k].course_name, g_requests[idx].course_name) == 0) {
                                for (int m = k; m < g_offering_count - 1; m++) g_offerings[m] = g_offerings[m + 1];
                                g_offering_count--; break;
                            }
                        }
                    }
                    for (int i = idx; i < g_request_count - 1; i++) g_requests[i] = g_requests[i + 1];
                    g_request_count--; save_all_data();
                    printf("Request approved.\n");
                } else if (strcmp(opt, "2") == 0) {
                    for (int i = idx; i < g_request_count - 1; i++) g_requests[i] = g_requests[i + 1];
                    g_request_count--; save_all_data();
                    printf("Request rejected.\n");
                }
            } else printf("Request not found.\n");
            press_enter_to_continue();
        } else if (strcmp(opt, "2") == 0) break;
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
    }
}

void admin_dashboard(void) {
    char opt[16];
    while (1) {
        clear_screen();
        printf("Welcome %s\n1. Calendar\n2. Students\n3. Faculty members\n4. Requests\n5. Offerings\n6. Courses\n7. Log out\nEnter an option: ", g_current_user_id);
        get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) admin_calendar_menu();
        else if (strcmp(opt, "2") == 0) admin_students_menu();
        else if (strcmp(opt, "3") == 0) admin_faculty_menu();
        else if (strcmp(opt, "4") == 0) admin_requests_menu();
        else if (strcmp(opt, "5") == 0) offerings_menu_flow(3);
        else if (strcmp(opt, "6") == 0) courses_menu_flow(3);
        else if (strcmp(opt, "7") == 0) break;
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
    }
}

/* FACULTY PANELS */

void faculty_offering_detail_menu(int idx) {
    char opt[16];
    while (1) {
        clear_screen();
        printf("|%d| %s | %s | %s | %d | %d | %s | %s |\n",
               g_offerings[idx].number, g_offerings[idx].course_id, g_offerings[idx].faculty_id,
               g_offerings[idx].semester, g_offerings[idx].capacity, g_offerings[idx].enrollments,
               g_offerings[idx].department, g_offerings[idx].place);
        printf("1. Add capacity\n2. Record grades\n3. Remove offering\n4. Publish a homework\n5. Publish an exam\n6. Go back\nEnter an option: ");
        get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) {
            printf("Enter added capacity: "); get_input(opt, sizeof(opt));
            g_offerings[idx].capacity += atoi(opt); save_all_data();
            printf("Capacity updated.\n"); press_enter_to_continue();
        } else if (strcmp(opt, "2") == 0) {
            if (!g_calendar.grade_recording) { printf("Grade recording period is currently disabled.\n"); press_enter_to_continue(); continue; }
            char sid[32], gbuf[16];
            printf("Enter student id: "); get_input(sid, sizeof(sid));
            int s_idx = -1;
            for (int s = 0; s < g_student_count; s++) { if (strcmp(g_students[s].id, sid) == 0) { s_idx = s; break; } }
            if (s_idx != -1) {
                printf("Enter grade (0.00 to 20.00): "); get_input(gbuf, sizeof(gbuf));
                double grade_val = atof(gbuf);
                bool updated = false;
                for (int g = 0; g < g_students[s_idx].grade_count; g++) {
                    if (strcmp(g_students[s_idx].grades[g].course_id, g_offerings[idx].course_id) == 0 &&
                        strcmp(g_students[s_idx].grades[g].semester, g_offerings[idx].semester) == 0) {
                        g_students[s_idx].grades[g].grade = grade_val; updated = true; break;
                    }
                }
                if (!updated && g_students[s_idx].grade_count < 100) {
                    safe_strcpy(g_students[s_idx].grades[g_students[s_idx].grade_count].semester, g_offerings[idx].semester, sizeof(g_students[s_idx].grades[0].semester));
                    safe_strcpy(g_students[s_idx].grades[g_students[s_idx].grade_count].course_id, g_offerings[idx].course_id, sizeof(g_students[s_idx].grades[0].course_id));
                    g_students[s_idx].grades[g_students[s_idx].grade_count].grade = grade_val;
                    g_students[s_idx].grade_count++;
                }
                save_all_data(); printf("Grade recorded successfully.\n");
            } else printf("Student not found.\n");
            press_enter_to_continue();
        } else if (strcmp(opt, "3") == 0) {
            printf("Remove offering? [y/n] "); get_input(opt, sizeof(opt));
            if (strcmp(opt, "y") == 0 || strcmp(opt, "Y") == 0) {
                for (int i = idx; i < g_offering_count - 1; i++) g_offerings[i] = g_offerings[i + 1];
                g_offering_count--; save_all_data(); printf("Offering removed.\n"); press_enter_to_continue(); break;
            }
        } else if (strcmp(opt, "4") == 0) { // Publish Homework
            if (g_homework_count >= MAX_ITEMS) { printf("LMS Storage full.\n"); press_enter_to_continue(); continue; }
            Homework hw; memset(&hw, 0, sizeof(hw));
            hw.id = g_homework_count + 1;
            hw.offering_number = g_offerings[idx].number;
            printf("Enter homework title: "); get_input(hw.title, sizeof(hw.title));
            printf("Enter multiple choice question: "); get_input(hw.question, sizeof(hw.question));
            printf("Enter Option 1: "); get_input(hw.opt1, sizeof(hw.opt1));
            printf("Enter Option 2: "); get_input(hw.opt2, sizeof(hw.opt2));
            printf("Enter Option 3: "); get_input(hw.opt3, sizeof(hw.opt3));
            printf("Enter Option 4: "); get_input(hw.opt4, sizeof(hw.opt4));
            char cbuf[16]; printf("Enter correct option number (1-4): "); get_input(cbuf, sizeof(cbuf)); hw.correct_option = atoi(cbuf);
            g_homeworks[g_homework_count++] = hw; save_all_data();
            printf("Homework '%s' published successfully!\n", hw.title);
            press_enter_to_continue();
        } else if (strcmp(opt, "5") == 0) { // Publish Exam
            if (g_exam_count >= MAX_ITEMS) { printf("LMS Storage full.\n"); press_enter_to_continue(); continue; }
            Exam ex; memset(&ex, 0, sizeof(ex));
            ex.id = g_exam_count + 1;
            ex.offering_number = g_offerings[idx].number;
            printf("Enter exam title: "); get_input(ex.title, sizeof(ex.title));
            printf("Enter exam main question: "); get_input(ex.question, sizeof(ex.question));
            char sbuf[16]; printf("Enter max score: "); get_input(sbuf, sizeof(sbuf)); ex.max_score = atoi(sbuf);
            g_exams[g_exam_count++] = ex; save_all_data();
            printf("Exam '%s' published successfully!\n", ex.title);
            press_enter_to_continue();
        } else if (strcmp(opt, "6") == 0) break;
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
    }
}

void faculty_my_offerings(void) {
    char opt[16];
    while (1) {
        clear_screen();
        printf("Faculty: My offerings\nList of my offerings (all offerings ordered by semester)\n");
        printf("|number | course name | course id | faculty id | semester | capacity | no. enrollments | department | place |\n");
        printf("|-------|-------------|-----------|------------|----------|----------|-----------------|------------|-------|\n");
        for (int i = 0; i < g_offering_count; i++) {
            if (strcmp(g_offerings[i].faculty_id, g_current_user_id) == 0) {
                printf("|%d| %s | %s | %s | %s | %d | %d | %s | %s |\n",
                       g_offerings[i].number, g_offerings[i].course_name, g_offerings[i].course_id,
                       g_offerings[i].faculty_id, g_offerings[i].semester, g_offerings[i].capacity,
                       g_offerings[i].enrollments, g_offerings[i].department, g_offerings[i].place);
            }
        }
        printf("\n1. Go to offering\n2. Search\n3. Go back\nEnter an option: ");
        get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) {
            printf("\nEnter offering number: "); get_input(opt, sizeof(opt));
            int off_num = atoi(opt);
            int idx = -1;
            for (int i = 0; i < g_offering_count; i++) {
                if (g_offerings[i].number == off_num && strcmp(g_offerings[i].faculty_id, g_current_user_id) == 0) {
                    idx = i; break;
                }
            }
            if (idx != -1) faculty_offering_detail_menu(idx);
            else { printf("Offering not found or does not belong to you.\n"); press_enter_to_continue(); }
        } else if (strcmp(opt, "2") == 0) {
            char query[64]; printf("The phrase to search: "); get_input(query, sizeof(query));
            printf("\nSearch Results:\n");
            for (int i = 0; i < g_offering_count; i++) {
                if (strcmp(g_offerings[i].faculty_id, g_current_user_id) == 0 &&
                    (my_strcasestr_contains(g_offerings[i].course_name, query) ||
                     my_strcasestr_contains(g_offerings[i].course_id, query))) {
                    printf("|%d| %s | %s | Semester: %s |\n", g_offerings[i].number, g_offerings[i].course_name, g_offerings[i].course_id, g_offerings[i].semester);
                }
            }
            press_enter_to_continue();
        } else if (strcmp(opt, "3") == 0) break;
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
    }
}

void faculty_offer_course(void) {
    clear_screen();
    printf("Faculty: Offer a course\n");
    if (!g_calendar.offering) { printf("Offering period is currently disabled.\n"); press_enter_to_continue(); return; }

    char cid[32], cap[16];
    printf("Enter the course id: "); get_input(cid, sizeof(cid));

    int idx = -1;
    for (int i = 0; i < g_course_count; i++) { if (strcmp(g_courses[i].course_id, cid) == 0) { idx = i; break; } }
    if (idx != -1) {
        printf("| %s | %s | %d | %s | %s | %s | %s |\n\n",
               g_courses[idx].course_name, g_courses[idx].course_id, g_courses[idx].units,
               g_courses[idx].prerequisites, g_courses[idx].section, g_courses[idx].field, g_courses[idx].department);
        printf("Enter the capacity: "); get_input(cap, sizeof(cap));

        if (g_request_count < MAX_ITEMS) {
            AdminRequest req; memset(&req, 0, sizeof(AdminRequest));
            req.id = g_request_count + 1;
            safe_strcpy(req.type, "course offering", sizeof(req.type));
            safe_strcpy(req.course_name, g_courses[idx].course_name, sizeof(req.course_name));
            safe_strcpy(req.faculty_id, g_current_user_id, sizeof(req.faculty_id));
            char real_name[128] = "Faculty Member";
            for (int f = 0; f < g_faculty_count; f++) {
                if (strcmp(g_faculty[f].id, g_current_user_id) == 0) {
                    snprintf(real_name, sizeof(real_name), "Dr. %s %s", g_faculty[f].first_name, g_faculty[f].last_name); break;
                }
            }
            safe_strcpy(req.faculty_name, real_name, sizeof(req.faculty_name));
            safe_strcpy(req.department, g_courses[idx].department, sizeof(req.department));
            req.capacity = atoi(cap); req.enrollments = 0;
            g_requests[g_request_count++] = req; save_all_data();
            printf("Sent request to admin.\n");
        } else printf("Request queue full.\n");
    } else printf("Course not found.\n");
    press_enter_to_continue();
}

void faculty_dashboard(void) {
    char opt[16];
    int f_idx = -1;
    for (int i = 0; i < g_faculty_count; i++) { if (strcmp(g_faculty[i].id, g_current_user_id) == 0) { f_idx = i; break; } }
    char name[128] = "Faculty Member";
    if (f_idx != -1) snprintf(name, sizeof(name), "Dr. %s %s", g_faculty[f_idx].first_name, g_faculty[f_idx].last_name);

    while (1) {
        clear_screen();
        printf("Faculty\nWelcome %s\n1. My offerings\n2. List of offerings in semester\n3. List of courses\n4. Offer a course\n5. Log out\nEnter an option: ", name);
        get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) faculty_my_offerings();
        else if (strcmp(opt, "2") == 0) offerings_menu_flow(2);
        else if (strcmp(opt, "3") == 0) courses_menu_flow(2);
        else if (strcmp(opt, "4") == 0) faculty_offer_course();
        else if (strcmp(opt, "5") == 0) break;
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
    }
}

/* STUDENT PANELS */

void student_phd_thesis_menu(void) {
    int s_idx = -1;
    for (int i = 0; i < g_student_count; i++) {
        if (strcmp(g_students[i].id, g_current_user_id) == 0) { s_idx = i; break; }
    }
    if (s_idx == -1) return;

    char opt[16];
    while (1) {
        clear_screen();
        printf("PhD Doctorate Thesis Details\n");
        printf("Student ID:     %s\n", g_students[s_idx].id);
        printf("Mentor:         %s\n", g_students[s_idx].mentor);
        printf("Thesis Title:   %s\n", strlen(g_students[s_idx].thesis_title) > 0 ? g_students[s_idx].thesis_title : "Not registered");
        printf("Abstract:       %s\n", strlen(g_students[s_idx].thesis_abstract) > 0 ? g_students[s_idx].thesis_abstract : "None");
        printf("Citations:      %d\n", g_students[s_idx].thesis_citations);
        printf("References:     %d\n\n", g_students[s_idx].thesis_refs);

        printf("1. Edit Title and Abstract\n2. Go back\nEnter an option: ");
        get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) {
            printf("Enter Thesis Title: "); get_input(g_students[s_idx].thesis_title, sizeof(g_students[s_idx].thesis_title));
            printf("Enter Abstract: "); get_input(g_students[s_idx].thesis_abstract, sizeof(g_students[s_idx].thesis_abstract));
            save_all_data();
            printf("Thesis details updated successfully.\n");
            press_enter_to_continue();
        } else if (strcmp(opt, "2") == 0) break;
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
    }
}

void student_lms_menu(void) {
    char opt[16];
    while (1) {
        clear_screen();
        printf("Student LMS Panel (Homeworks & Exams)\n1. View & Submit Homeworks\n2. View Exams\n3. Go back\nEnter an option: ");
        get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) {
            clear_screen();
            printf("Published Homeworks:\n");
            if (g_homework_count == 0) printf("No homeworks published yet.\n");
            else {
                for (int i = 0; i < g_homework_count; i++) {
                    printf("Homework #%d: %s\n   Question: %s\n   1) %s  2) %s  3) %s  4) %s\n",
                           g_homeworks[i].id, g_homeworks[i].title, g_homeworks[i].question,
                           g_homeworks[i].opt1, g_homeworks[i].opt2, g_homeworks[i].opt3, g_homeworks[i].opt4);
                    // Check if already submitted
                    int submitted_choice = 0;
                    for (int s = 0; s < g_submission_count; s++) {
                        if (strcmp(g_submissions[s].student_id, g_current_user_id) == 0 && g_submissions[s].homework_id == g_homeworks[i].id) {
                            submitted_choice = g_submissions[s].selected_option; break;
                        }
                    }
                    if (submitted_choice > 0) printf("   Status: Submitted (Choice: %d)\n\n", submitted_choice);
                    else printf("   Status: Not submitted\n\n");
                }
                printf("Enter Homework # to solve (or 0 to cancel): "); get_input(opt, sizeof(opt));
                int hw_id = atoi(opt);
                if (hw_id > 0) {
                    printf("Enter option choice (1-4): "); get_input(opt, sizeof(opt));
                    int choice = atoi(opt);
                    if (choice >= 1 && choice <= 4) {
                        bool updated = false;
                        for (int s = 0; s < g_submission_count; s++) {
                            if (strcmp(g_submissions[s].student_id, g_current_user_id) == 0 && g_submissions[s].homework_id == hw_id) {
                                g_submissions[s].selected_option = choice; updated = true; break;
                            }
                        }
                        if (!updated && g_submission_count < MAX_ITEMS) {
                            safe_strcpy(g_submissions[g_submission_count].student_id, g_current_user_id, sizeof(g_submissions[0].student_id));
                            g_submissions[g_submission_count].homework_id = hw_id;
                            g_submissions[g_submission_count].selected_option = choice;
                            g_submission_count++;
                        }
                        save_all_data();
                        printf("Answer choice %d submitted successfully!\n", choice);
                    } else printf("Invalid option choice.\n");
                }
            }
            press_enter_to_continue();
        } else if (strcmp(opt, "2") == 0) {
            clear_screen();
            printf("Published Exams:\n");
            if (g_exam_count == 0) printf("No exams published yet.\n");
            else {
                for (int i = 0; i < g_exam_count; i++) {
                    printf("Exam #%d: %s\n   Question: %s\n   Max Score: %d\n\n",
                           g_exams[i].id, g_exams[i].title, g_exams[i].question, g_exams[i].max_score);
                }
            }
            press_enter_to_continue();
        } else if (strcmp(opt, "3") == 0) break;
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
    }
}

void student_report_card(void) {
    while (1) {
        clear_screen();
        int s_idx = -1;
        for (int i = 0; i < g_student_count; i++) {
            if (strcmp(g_students[i].id, g_current_user_id) == 0) { s_idx = i; break; }
        }
        if (s_idx == -1) return;

        double sum = 0.0; int total_units = 0;
        for (int i = 0; i < g_students[s_idx].grade_count; i++) {
            if (g_students[s_idx].grades[i].grade >= 0.0) {
                int u = 3;
                for (int c = 0; c < g_course_count; c++) {
                    if (strcmp(g_courses[c].course_id, g_students[s_idx].grades[i].course_id) == 0) { u = g_courses[c].units; break; }
                }
                sum += g_students[s_idx].grades[i].grade * u; total_units += u;
            }
        }
        double gpa = total_units > 0 ? (sum / total_units) : 0.0;

        printf("Student: Report Card\n");
        printf("|student id     |%s |\n", g_students[s_idx].id);
        printf("|first name     |%s |\n", g_students[s_idx].first_name);
        printf("|last name      |%s |\n", g_students[s_idx].last_name);
        printf("|national code  |%s |\n", g_students[s_idx].national_code);
        printf("|field          |%s |\n", g_students[s_idx].field);
        printf("|entrance year  |%s |\n", g_students[s_idx].entrance_year);
        printf("|section        |%s |\n", g_students[s_idx].section);
        printf("|mentor         |%s |\n", g_students[s_idx].mentor);
        printf("|department     |%s |\n", g_students[s_idx].department);
        printf("|GPA            |%.2f |\n\n", gpa);

        printf("1. Go to semester\n2. Go back\nEnter an option: ");
        char opt[16]; get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) {
            char sem[16]; printf("Student: Report Card: Go to semester\nEnter semester number: ");
            get_input(sem, sizeof(sem));
            if (strlen(sem) == 0) safe_strcpy(sem, "14042", sizeof(sem));

            clear_screen();
            printf("Report card - %s %s - %s\n", g_students[s_idx].first_name, g_students[s_idx].last_name, sem);
            printf("| course name | course id | units | grade | passed | instructor's name |\n");
            printf("|-------------|-----------|-------|-------|--------|-------------------|\n");

            int enrolled = 0, passed = 0, failed = 0;
            double sem_sum = 0.0; int sem_units = 0;

            for (int i = 0; i < g_students[s_idx].grade_count; i++) {
                if (strcmp(g_students[s_idx].grades[i].semester, sem) == 0) {
                    enrolled++;
                    char cname[128] = "Course", instructor[128] = "Dr. Teacher";
                    int u = 3;
                    for (int c = 0; c < g_course_count; c++) {
                        if (strcmp(g_courses[c].course_id, g_students[s_idx].grades[i].course_id) == 0) {
                            safe_strcpy(cname, g_courses[c].course_name, sizeof(cname)); u = g_courses[c].units; break;
                        }
                    }
                    for (int o = 0; o < g_offering_count; o++) {
                        if (strcmp(g_offerings[o].course_id, g_students[s_idx].grades[i].course_id) == 0 &&
                            strcmp(g_offerings[o].semester, sem) == 0) {
                            safe_strcpy(instructor, g_offerings[o].faculty_name, sizeof(instructor)); break;
                        }
                    }

                    double gr = g_students[s_idx].grades[i].grade;
                    bool is_passed = gr >= 10.0;
                    if (gr >= 0.0) {
                        if (is_passed) passed++; else failed++;
                        sem_sum += gr * u; sem_units += u;
                        printf("| %s | %s | %d | %.2f | %s | %s |\n", cname, g_students[s_idx].grades[i].course_id, u, gr, is_passed ? "Yes" : "No", instructor);
                    } else {
                        printf("| %s | %s | %d | N/A | Pending | %s |\n", cname, g_students[s_idx].grades[i].course_id, u, instructor);
                    }
                }
            }

            double sem_gpa = sem_units > 0 ? (sem_sum / sem_units) : 0.0;
            printf("\nEnrolled courses: %d\nPassed courses: %d\nFailed courses: %d\nGPA: %.2f\n", enrolled, passed, failed, sem_gpa);
            press_enter_to_continue();
        } else if (strcmp(opt, "2") == 0) break;
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
    }
}

void student_dashboard(void) {
    char opt[16];
    int s_idx = -1;
    for (int i = 0; i < g_student_count; i++) { if (strcmp(g_students[i].id, g_current_user_id) == 0) { s_idx = i; break; } }
    bool is_phd = (s_idx != -1 && my_strcasecmp(g_students[s_idx].section, "PhD") == 0);

    while (1) {
        clear_screen();
        printf("Student\n1. Offerings\n2. Courses\n3. Report card\n4. LMS (Homeworks & Exams)\n");
        if (is_phd) printf("5. Doctorate Thesis Details\n6. Log out\n");
        else printf("5. Log out\n");
        printf("Enter an option: ");
        get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) offerings_menu_flow(1);
        else if (strcmp(opt, "2") == 0) courses_menu_flow(1);
        else if (strcmp(opt, "3") == 0) student_report_card();
        else if (strcmp(opt, "4") == 0) student_lms_menu();
        else if (is_phd && strcmp(opt, "5") == 0) student_phd_thesis_menu();
        else if ((is_phd && strcmp(opt, "6") == 0) || (!is_phd && strcmp(opt, "5") == 0)) break;
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
    }
}

/* MAIN ENTRY POINT */

void main_menu(void) {
    char opt[16];
    while (1) {
        clear_screen();
        printf("First page\n1. Login as student\n2. Login as faculty\n3. Login as admin\n4. Forgot password\n5. Exit\nEnter an option: ");
        get_input(opt, sizeof(opt));

        if (strcmp(opt, "1") == 0) login_flow(1);
        else if (strcmp(opt, "2") == 0) login_flow(2);
        else if (strcmp(opt, "3") == 0) login_flow(3);
        else if (strcmp(opt, "4") == 0) forgot_password_flow();
        else if (strcmp(opt, "5") == 0) exit(0);
        else { printf("Invalid option, try again.\n"); press_enter_to_continue(); }
    }
}

int main(void) {
    load_all_data();
    main_menu();
    return 0;
}
