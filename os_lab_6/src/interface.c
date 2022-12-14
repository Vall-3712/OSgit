#include "interface.h"

const char *read_word() {
    char *result = (char *)calloc(sizeof(char), STR_LEN);
    char current;
    int i = 0;
    TRY_READ(&current);
    while (current != ' ') {
        if (current == '\n' || current == '\0')
            break;
        result[i++] = current;
        TRY_READ(&current);
    }
    result = (char *)realloc(result, sizeof(char) * (strlen(result) + 1));
    return result;
}

command_t get_command() {
    const char *input = read_word();

    if (strcmp("exit", input) == 0)
        return EXIT;

    if (strcmp("print", input) == 0)
        return PRINT;

    if (strcmp("create", input) == 0)
        return CREATE;

    if (strcmp("remove", input) == 0)
        return REMOVE;

    if (strcmp("exec", input) == 0)
        return EXEC;

    if (strcmp("pingall", input) == 0)
        return PINGALL;

    else
        return UNKNOWN;
}

subcommand_t get_subcommand() {
    const char *input = read_word();

    if (strcmp("time", input) == 0)
        return TIME;

    if (strcmp("start", input) == 0)
        return START;

    if (strcmp("stop", input) == 0)
        return STOP;

    else
        return UNDEFINED;
}

const char *int_to_string(unsigned a) {
    int x = a, i = 0;
    if (a == 0)
        return "0";
    while (x > 0) {
        x /= 10;
        i++;
    }
    char *result = (char *)calloc(sizeof(char), i + 1);
    while (i >= 1) {
        result[--i] = a % 10 + '0';
        a /= 10;
    }

    return result;
}

const char *portname_client(unsigned short port) {
    const char *port_string = int_to_string(port);
    char *name = (char *)calloc(sizeof(char), strlen(CLIENT_ADRESS_PREFIX) + strlen(port_string) + 1);
    strcpy(name, CLIENT_ADRESS_PREFIX);
    strcpy(name + strlen(CLIENT_ADRESS_PREFIX) * sizeof(char), port_string);
    return name;
}

const char *portname_server(unsigned short port) {
    const char *port_string = int_to_string(port);
    char *name = (char *)calloc(sizeof(char), strlen(SERVER_ADRESS_PREFIX) + strlen(port_string) + 1);
    strcpy(name, SERVER_ADRESS_PREFIX);
    strcpy(name + strlen(SERVER_ADRESS_PREFIX) * sizeof(char), port_string);
    return name;
}

char *message_prefix(unsigned node_id, subcommand_t s) {
    char *result = (char *)calloc(sizeof(char), STR_LEN);
    result[0] = 'O';
    result[1] = 'k';
    result[2] = ' ';
    result[3] = '\0';
    const char *id_str = int_to_string(node_id);
    result = strcat(result, id_str);
    result = strcat(result, ": ");
    switch (s) {
    case START:
        result = strcat(result, "started");
        break;
    case STOP:
        result = strcat(result, "stopped");
        break;
    default:
        break;
    }
    result = (char *)realloc(result, strlen(result) + 1);
    return result;
}
