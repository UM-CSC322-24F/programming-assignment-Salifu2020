#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOATS 120
#define MAX_NAME_LENGTH 128
#define MAX_TRAILOR_TAG 10

// Rates per foot per month
#define RATE_SLIP 12.5
#define RATE_LAND 14.0
#define RATE_TRAILOR 25.0
#define RATE_STORAGE 11.2

typedef enum { SLIP, LAND, TRAILOR, STORAGE } PlaceType;

typedef union {
    int slip_number;
    char bay_letter;
    char trailor_tag[MAX_TRAILOR_TAG];
    int storage_space;
} ExtraInfo;

typedef struct {
    char name[MAX_NAME_LENGTH];
    float length;
    PlaceType place;
    ExtraInfo info;
    float amount_owed;
} Boat;

Boat *boats[MAX_BOATS] = { NULL };
int boat_count = 0;

// Function prototypes
void load_data(const char *filename);
void save_data(const char *filename);
void display_menu();
void add_boat(char *csv_line);
void remove_boat(const char *name);
void accept_payment(const char *name, float payment);
void update_monthly_amount();
void print_inventory();
Boat *create_boat(const char *name, float length, PlaceType place, ExtraInfo info, float amount_owed);
void free_boats();
int compare_boats(const void *a, const void *b);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <BoatData.csv>\n", argv[0]);
        return 1;
    }

    load_data(argv[1]);
    char choice;
    char input[256];

    printf("Welcome to the Boat Management System\n-------------------------------------\n");
    while (1) {
        display_menu();
        printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        if (scanf(" %c", &choice) != 1) break;
        choice = toupper(choice);

        switch (choice) {
            case 'I': print_inventory(); break;
            case 'A':
                printf("Enter boat data in CSV format: ");
                scanf(" %[^\n]", input);
                add_boat(input);
                break;
            case 'R':
                printf("Enter the boat name: ");
                scanf(" %[^\n]", input);
                remove_boat(input);
                break;
            case 'P': {
                printf("Enter the boat name: ");
                scanf(" %[^\n]", input);
                float payment;
                printf("Enter payment amount: ");
                if (scanf("%f", &payment) == 1) accept_payment(input, payment);
                break;
            }
            case 'M': update_monthly_amount(); break;
            case 'X': 
                save_data(argv[1]);
                free_boats();
                printf("Exiting Boat Management System\n");
                return 0;
            default:
                printf("Invalid option %c\n", choice);
        }
    }
}

void load_data(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file) && boat_count < MAX_BOATS) {
        add_boat(line);
    }
    
    fclose(file);
}

void save_data(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error saving file");
        return;
    }

    for (int i = 0; i < boat_count; i++) {
        fprintf(file, "%s,%.2f,", boats[i]->name, boats[i]->length);
        switch (boats[i]->place) {
            case SLIP:
                fprintf(file, "slip,%d,%.2f\n", boats[i]->info.slip_number, boats[i]->amount_owed);
                break;
            case LAND:
                fprintf(file, "land,%c,%.2f\n", boats[i]->info.bay_letter, boats[i]->amount_owed);
                break;
            case TRAILOR:
                fprintf(file, "trailor,%s,%.2f\n", boats[i]->info.trailor_tag, boats[i]->amount_owed);
                break;
            case STORAGE:
                fprintf(file, "storage,%d,%.2f\n", boats[i]->info.storage_space, boats[i]->amount_owed);
                break;
        }
    }

    fclose(file);
}

void display_menu() {
    printf("\nBoat Management System Menu:\n");
    printf(" (I)nventory\n (A)dd\n (R)emove\n (P)ayment\n (M)onth\n e(X)it");
}

void add_boat(char *csv_line) {
    if (boat_count >= MAX_BOATS) {
        printf("The marina is full.\n");
        return;
    }

    char name[MAX_NAME_LENGTH];
    float length;
    char place_str[10];
    ExtraInfo info;
    float amount_owed;
    char extra_info[20];

    // Scan the initial fields from the CSV line
    if (sscanf(csv_line, "%127[^,],%f,%9[^,],%19[^,],%f", name, &length, place_str, extra_info, &amount_owed) != 5) {
        printf("Invalid data format\n");
        return;
    }

    PlaceType place;
    if (strcmp(place_str, "slip") == 0) {
        place = SLIP;
        sscanf(extra_info, "%d", &info.slip_number);
    } else if (strcmp(place_str, "land") == 0) {
        place = LAND;
        sscanf(extra_info, "%c", &info.bay_letter);
    } else if (strcmp(place_str, "trailor") == 0) {
        place = TRAILOR;
        strncpy(info.trailor_tag, extra_info, MAX_TRAILOR_TAG - 1);
        info.trailor_tag[MAX_TRAILOR_TAG - 1] = '\0';  // Ensure null-termination
    } else if (strcmp(place_str, "storage") == 0) {
        place = STORAGE;
        sscanf(extra_info, "%d", &info.storage_space);
    } else {
        printf("Invalid place type\n");
        return;
    }

    boats[boat_count++] = create_boat(name, length, place, info, amount_owed);
    qsort(boats, boat_count, sizeof(Boat *), compare_boats);
}

Boat *create_boat(const char *name, float length, PlaceType place, ExtraInfo info, float amount_owed) {
    Boat *boat = (Boat *)malloc(sizeof(Boat));
    if (!boat) {
        perror("Failed to allocate memory for boat");
        return NULL;
    }

    strncpy(boat->name, name, MAX_NAME_LENGTH);
    boat->name[MAX_NAME_LENGTH - 1] = '\0'; // Ensure null-termination
    boat->length = length;
    boat->place = place;
    boat->info = info;
    boat->amount_owed = amount_owed;

    return boat;
}

void free_boats() {
    for (int i = 0; i < boat_count; i++) {
        free(boats[i]);
        boats[i] = NULL;
    }
    boat_count = 0;
}

int compare_boats(const void *a, const void *b) {
    Boat *boatA = *(Boat **)a;
    Boat *boatB = *(Boat **)b;
    return strcasecmp(boatA->name, boatB->name);
}

void remove_boat(const char *name) {
    int i;
    for (i = 0; i < boat_count; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            free(boats[i]);
            boats[i] = boats[--boat_count];
            qsort(boats, boat_count, sizeof(Boat *), compare_boats);
            printf("Boat %s removed.\n", name);
            return;
        }
    }
    printf("No boat with that name.\n");
}

void accept_payment(const char *name, float payment) {
    for (int i = 0; i < boat_count; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            if (payment > boats[i]->amount_owed) {
                printf("Payment exceeds amount owed.\n");
                return;
            }
            boats[i]->amount_owed -= payment;
            printf("Payment accepted. New balance: $%.2f\n", boats[i]->amount_owed);
            return;
        }
    }
    printf("No boat with that name.\n");
}

void update_monthly_amount() {
    for (int i = 0; i < boat_count; i++) {
        switch (boats[i]->place) {
            case SLIP: boats[i]->amount_owed += boats[i]->length * RATE_SLIP; break;
            case LAND: boats[i]->amount_owed += boats[i]->length * RATE_LAND; break;
            case TRAILOR: boats[i]->amount_owed += boats[i]->length * RATE_TRAILOR; break;
            case STORAGE: boats[i]->amount_owed += boats[i]->length * RATE_STORAGE; break;
        }
    }
    printf("Monthly charges updated.\n");
}

void print_inventory() {
    for (int i = 0; i < boat_count; i++) {
        printf("%s %.0f' ", boats[i]->name, boats[i]->length);
        switch (boats[i]->place) {
            case SLIP:
                printf("slip #%d ", boats[i]->info.slip_number);
                break;
            case LAND:
                printf("land bay %c ", boats[i]->info.bay_letter);
                break;
            case TRAILOR:
                printf("trailor tag %s ", boats[i]->info.trailor_tag);
                break;
            case STORAGE:
                printf("storage space %d ", boats[i]->info.storage_space);
                break;
        }
        printf("- Amount owed: $%.2f\n", boats[i]->amount_owed);
    }
}
