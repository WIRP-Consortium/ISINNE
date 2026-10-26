
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_INPUT_SIZE 1024
#define MAX_NAME_SIZE 256
#define MAX_IP_SIZE 64

int find_imperian(const char *name, char *output_ip, size_t ip_size) {
    FILE *conf_file = fopen("data/name.conf", "r");

    if (conf_file == NULL) {
        perror("Could not open data/name.conf");
        return 1;
    }

    char line[MAX_INPUT_SIZE];
    char file_name[MAX_NAME_SIZE];
    char file_ip[MAX_IP_SIZE];

    while (fgets(line, sizeof(line), conf_file) != NULL) {
        if (sscanf(line, "%255s %63s", file_name, file_ip) == 2) {
            if (strcmp(name, file_name) == 0) {
                snprintf(output_ip, ip_size, "%s", file_ip);
                fclose(conf_file);
                return 0; // Found
            }
        }
    }

    fclose(conf_file);
    return 2; // Not found
}

int main(void) {
    char input[MAX_INPUT_SIZE];

    printf("GINNA Authority Operator CLI\n");

    while (true) {
        printf("GCAO@ISINNE~$ ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            continue;
        }

        char *command = strtok(input, " \t");

        if (strcmp(command, "exit") == 0) {
            printf("Exiting CLI. Goodbye!\n");
            break;
        } else if (strcmp(command, "help") == 0) {
            printf("Available commands:\n");
            printf("  help          - Show this list\n");
            printf("  greet         - Display a greeting\n");
            printf("  search <name> - Search for a name\n");
            printf("  exit          - Close the CLI\n");
        } else if (strcmp(command, "greet") == 0) {
            printf("Hello! The C-based CLI is working perfectly.\n");
        } else if (strcmp(command, "search") == 0) {
            char *name = strtok(NULL, " \t");

            if (name == NULL) {
                printf("Usage: search <name>\n");
                continue;
            }

            char output_ip[MAX_IP_SIZE];
            int status = find_imperian(name, output_ip,
                                       sizeof(output_ip));

            if (status == 0) {
                printf("Found: %s -> %s\n", name, output_ip);
            } else if (status == 2) {
                printf("Name not found: %s\n", name);
            } else {
                printf("Search failed because the file could not be read.\n");
            }
        } else {
            printf("INVALID COMMAND!\n");
        }
    }

    return 0;
}
