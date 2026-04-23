#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// MADE BY XRS_MC

FILE *adbShell = NULL;

void clearBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int ensureAdbInstalled() {
    int exists = system("command -v adb > /dev/null 2>&1"); // ensure ADB is installed on the LINUX system.

    if (exists == 0) {
        return 1;
    }

    char choice;

    while (1) {
        printf("ADB not found. This is required for the program to work. Install it now? (y/n): ");
        choice = getchar();
        clearBuffer();

        if (choice == 'y' || choice == 'Y') {
            printf("\nInstalling ADB...\n\n");

            int result = system("sudo apt update && sudo apt install -y adb");

            if (result != 0) {
                printf("Failed to install adb!\n");
                return 0;
            }

            printf("\nADB installed successfully!\n\n");
            return 1;
        }

        if (choice == 'n' || choice == 'N') {
            printf("\nADB is required. Exiting.\n");
            return 0;
        }

        printf("\nInvalid input! Please enter y or n.\n\n");
    }
}

void sendCommand(const char *cmd) {
    if (adbShell) {
        fprintf(adbShell, "%s\n", cmd);
        fflush(adbShell);
    }
}

void sendKey(int key) {
    char command[64];
    snprintf(command, sizeof(command), "input keyevent %d", key);
    sendCommand(command);
}

int connect(const char *ip) {
    char command[128];
    char buffer[128];
    int success = 0;

    snprintf(command, sizeof(command), "adb connect %s", ip);
    FILE *pipe = popen(command, "r");
    if (!pipe) return 0;

    while (fgets(buffer, sizeof(buffer), pipe)) {
        if (strstr(buffer, "connected") && !strstr(buffer, "failed")) {
            success = 1;
        }
    }
    pclose(pipe);

    if (success) {
        adbShell = popen("adb shell", "w"); // use one persistent shell instead of opening one every time sending a key.
        if (!adbShell) {
            printf("Failed to start adb shell!\n");
            return 0;
        }
    }

    return success;
}

void menu() {
    printf("\n1. Up    2. Down\n");
    printf("3. Left  4. Right\n");
    printf("5. Ok    6. Back\n");
    printf("7. Home  8. Set Volume\n");
    printf("9. Type  10. Exit\n");
    printf("Choice: ");
}

void askIp() {
    int success = 0;
    char ip[64];

    while (!success) {
        printf("IP: ");
        if (scanf("%63s", ip) != 1) {
            clearBuffer();
            continue;
        }
        clearBuffer();

        success = connect(ip);
        if (!success) printf("\nFailed!\n\n");
        else printf("\nConnected!\n");
    }
}

void setVolume() {
    int volume;

    printf("Volume: ");
    while (scanf("%d", &volume) != 1) {
        printf("Only a number!\n");
        clearBuffer();
        printf("Volume: ");
    }

    clearBuffer();

    char command[128];
    snprintf(command, sizeof(command),
             "cmd media_session volume --stream 3 --set %d",
             volume);
    sendCommand(command);
}

void type() {
    char raw[128];
    char safe[512];
    char command[1024];

    safe[0] = '\0';

    printf("Text: ");

    if (!fgets(raw, sizeof(raw), stdin)) return;

    raw[strcspn(raw, "\n")] = 0;

    for (int i = 0; raw[i]; i++) {
        if (raw[i] == ' ') {
            strncat(safe, "%s", sizeof(safe) - strlen(safe) - 1);
        } else if (isalnum((unsigned char)raw[i])) {
            strncat(safe, &raw[i], 1);
        }
    }

    snprintf(command, sizeof(command), "input text \"%s\"", safe);
    sendCommand(command);
}

int main() {
    int running = 1;
    int choice;

    if (!ensureAdbInstalled()) {
        return 1;
    }

    askIp();

    while (running) {
        menu();

        if (scanf("%d", &choice) != 1) {
            printf("\nOption not found!\n");
            clearBuffer();
            continue;
        }
        clearBuffer();

        switch (choice) {
            case 1: sendKey(19); break;
            case 2: sendKey(20); break;
            case 3: sendKey(21); break;
            case 4: sendKey(22); break;
            case 5: sendKey(66); break;
            case 6: sendKey(4);  break;
            case 7: sendKey(3);  break;
            case 8: setVolume(); break;
            case 9: type();      break; // more features will be added.
            case 10: running = 0; break;
            default:
                printf("\nOption not found!\n");
                break;
        }
    }

    if (adbShell) {
        pclose(adbShell); // close the shell when the program is going to stop.
    }

    return 0;
}