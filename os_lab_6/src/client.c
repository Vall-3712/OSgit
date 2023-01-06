#include "interprocess.h"
#define INITIAL_CAPACITY 5

#define CLIENT_ID -1

unsigned child_count = 0;
unsigned capacity = INITIAL_CAPACITY;

int main(int argc, char const *argv[]) {

    List *system = (List *)calloc(sizeof(List), INITIAL_CAPACITY);
    for (int i = 0; i < INITIAL_CAPACITY; ++i)
        system[i] = create_node(CLIENT_ID);

    void *context = zmq_ctx_new();
    if (context == NULL) {
        perror("context");
        exit(EXIT_FAILURE);
    }

    void **child_sockets = calloc(sizeof(void *), INITIAL_CAPACITY);
    id *child_ids = calloc(sizeof(id), INITIAL_CAPACITY);
    const char *arrow = "> ";
    size_t arrow_len = strlen(arrow);

    while (1) {
        if (write(1, arrow, arrow_len) <= 0) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        command_t current = get_command();
        switch (current) {
        case PRINT:
            for (int i = 0; i <= child_count; ++i) {
                print_list(system[i]);
            }
            break;
        case EXIT:
            for (int i = 0; i < child_count; ++i) {
                send_exit(child_sockets[i]);
            }
            break;
        case CREATE:;
            const char *init_id_str = read_word();
            const char *parent_id_str = read_word();
            int init_id = atoi(init_id_str);
            if (init_id <= 0) {
                printf("Error: invalid node id (id should be an integer greater than 0).\n");
                break;
            }
            if (exists(system, init_id, child_count)) {
                printf("Error: already exists.\n");
                break;
            }
            int parent_id = atoi(parent_id_str);
            if (parent_id <= 0 && parent_id != CLIENT_ID) {
                printf("Error: invalid parent id (parent should be an integer greater than 0 or %d for root).\n", CLIENT_ID);
                break;
            }
            if ((parent_id != CLIENT_ID) && (!exists(system, parent_id, child_count))) {
                printf("Error: there is no nodes with id = %d.\n", parent_id);
                break;
            }

            if (parent_id == CLIENT_ID) {
                int fork_val = fork();
                if (fork_val == 0)
                    create_worker(init_id, 0);
                printf("Created node %d with PID %d\n", init_id, fork_val);

                if (child_count >= capacity) {
                    capacity *= 2;
                    child_sockets = realloc(child_sockets, sizeof(void *) * capacity);
                    child_ids = realloc(child_ids, sizeof(id) * capacity);
                    for (int i = child_count; i < capacity; ++i)
                        system[i] = create_node(CLIENT_ID);
                }
                child_sockets[child_count] = zmq_socket(context, ZMQ_REQ);
                int opt_val = 0;
                int rc = zmq_setsockopt(child_sockets[child_count], ZMQ_LINGER, &opt_val, sizeof(opt_val));
                assert(rc == 0);
                if (child_sockets[child_count] == NULL) {
                    perror("socket");
                    exit(EXIT_FAILURE);
                }

                rc = zmq_connect(child_sockets[child_count], portname_client(BASE_PORT + init_id));
                assert(rc == 0);

                child_ids[child_count] = init_id;
                system[child_count] = add_node(system[child_count], parent_id, init_id);
                child_count++;
                break;
            } else {
                int replied = 0;
                for (int i = 0; i < child_count; ++i) {
                    send_create(child_sockets[i], init_id, parent_id);
                    if (!available_recive(child_sockets[i])) {
                        continue;
                    }
                    const char *reply = get_reply(child_sockets[i]);
                    if (strcmp(EMPTY_MSG, reply) != 0) {
                        printf("%s\n", reply);
                        replied = 1;
                        break;
                    }
                }
                if (replied == 0)
                    printf("Node %d seems to be unavailable\n", parent_id);
                else
                    for (int i = 0; i < child_count; ++i)
                        system[i] = add_node(system[i], parent_id, init_id);
            }
            break;

        case EXEC:;
            const char *target_id_str = read_word();
            int target_id = atoi(target_id_str);
            if (target_id <= 0) {
                printf("Error: invalid node id (id should be an integer greater than 0).\n");
                break;
            }
            if (!exists(system, target_id, child_count)) {
                printf("Error: this node does not exists.\n");
                break;
            }
            subcommand_t current_sub = get_subcommand();
            if (current_sub == UNDEFINED) {
                printf("Invalid subcommand!\n");
                break;
            }

            int replied = 0;
            for (int i = 0; i < child_count; ++i) {
                send_exec(child_sockets[i], current_sub, target_id);
                if (!available_recive(child_sockets[i])) {
                    continue;
                }
                char *reply = get_reply(child_sockets[i]);
                if (strcmp(EMPTY_MSG, reply) != 0) {
                    printf("%s\n", reply);
                    replied = 1;
                    break;
                }
            }
            if (replied == 0)
                printf("Node %d seems to be unavailable\n", target_id);

            break;
        case REMOVE:;
            const char *remove_id_str = read_word();
            int remove_id = atoi(remove_id_str);
            if (remove_id <= 0) {
                printf("Error: invalid node id (id should be an integer greater than 0).\n");
                break;
            }
            if (!exists(system, remove_id, child_count)) {
                printf("Error: this node does not exist.\n");
                break;
            }
            int i = in_list(child_ids, remove_id, child_count);
            if (i != -1) {
                shift_id(child_ids, i, child_count);
                send_exit(child_sockets[i]);
                zmq_close(child_sockets[i]);
                shift_void(child_sockets, i, child_count);
                child_count--;
                printf("Successfully removed node %d from system\n", remove_id);
            } else {
                replied = 0;
                for (int i = 0; i < child_count; ++i) {
                    send_remove(child_sockets[i], remove_id);
                    if (!available_recive(child_sockets[i])) {
                        continue;
                    }
                    char *reply = get_reply(child_sockets[i]);
                    if (strcmp(EMPTY_MSG, reply) != 0) {
                        printf("%s\n", reply);
                        replied = 1;
                        break;
                    }
                }
                if (replied == 0)
                    printf("Node %d seems to be unavailable, removed it from tree anyways\n", remove_id);
            }
            for (int i = 0; i < child_count; ++i)
                system[i] = remove_node(system[i], remove_id);
            break;
        case PINGALL:;
            int not_replied = 0;
            int count = 0;
            char *unavailable = (char *)calloc(sizeof(char), STR_LEN);
            for (int i = 0; i < child_count; ++i) {
                send_pingall(child_sockets[i], count + 1);
                if (!available_recive_pingall(child_sockets[i], count)) {
                    unavailable = strcat(unavailable, int_to_string(child_ids[i]));
                    unavailable = strcat(unavailable, " ");
                    not_replied++;
                    continue;
                }
                char *reply_child = get_reply(child_sockets[i]);
                if (strcmp(EMPTY_MSG, reply_child) != 0) {
                    unavailable = strcat(unavailable, reply_child);
                    replied = 1;
                    break;
                }
            }
            if (not_replied == 0 && replied == 0) {
                printf("Every process is available\n");
            } else {
                fprintf(stderr, "%s\n", unavailable);
            }
            break;
        default:
            printf("Error: invalid command.\n");
            break;
        }
        if (current == EXIT) {
            break;
        }
    }

    for (int i = 0; i < child_count; ++i) {
        zmq_close(child_sockets[i]);
    }
    zmq_ctx_destroy(context);

    return 0;
}