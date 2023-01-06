#include <sys/time.h>
#include "interprocess.h"

#define OFF 0
#define ON 1

#define INITIAL_CAPACITY 5

short timer = OFF;

unsigned has_child = 0;

int main(int argc, const char **argv) {
    void *context = zmq_ctx_new();
    if (context == NULL) {
        perror("context");
        exit(EXIT_FAILURE);
    }
    void *parent = zmq_socket(context, ZMQ_REP);
    if (parent == NULL) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    int self_id = atoi(argv[1]);
    int rc = zmq_bind(parent, portname_server(BASE_PORT + self_id));
    assert(rc == 0);

    void *child_socket;
    id child_id;

    if (atoi(argv[2]) != 0) {
        child_socket = zmq_socket(context, ZMQ_REQ);
        has_child = 1;
        int opt_val = 0;
        int rc = zmq_setsockopt(child_socket, ZMQ_LINGER, &opt_val, sizeof(opt_val));
        assert(rc == 0);
        if (child_socket == NULL) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        rc = zmq_connect(child_socket, portname_client(atoi(argv[2])));
        assert(rc == 0);

        child_id = atoi(argv[2]) - BASE_PORT;
    }

    double diff_sec = 0, diff_msec = 0;

    struct timeval start, finish;

    while (1) {
        command_t current = UNKNOWN;
        subcommand_t current_sub = UNDEFINED;
        int target_id = 0;
        int parent_id = 0;
        int arg = 0;
        int count = 0;

        while (1) {
            zmq_msg_t part;
            int rec = zmq_msg_init(&part);
            assert(rec == 0);
            rec = zmq_msg_recv(&part, parent, 0);
            assert(rec != -1);

            switch (arg) {
            case 0:
                memcpy(&current, zmq_msg_data(&part), zmq_msg_size(&part));
                break;
            case 1:
                switch (current) {
                case EXEC:
                    memcpy(&current_sub, zmq_msg_data(&part), zmq_msg_size(&part));
                    break;
                case CREATE:
                    memcpy(&target_id, zmq_msg_data(&part), zmq_msg_size(&part));
                    break;
                case REMOVE:
                    memcpy(&target_id, zmq_msg_data(&part), zmq_msg_size(&part));
                    break;
                case PINGALL:
                    memcpy(&count, zmq_msg_data(&part), zmq_msg_size(&part));
                    break;
                default:
                    break;
                }
                break;
            case 2:
                switch (current) {
                case EXEC:
                    memcpy(&target_id, zmq_msg_data(&part), zmq_msg_size(&part));
                    break;
                case CREATE:
                    memcpy(&parent_id, zmq_msg_data(&part), zmq_msg_size(&part));
                    break;
                default:
                    break;
                }
                break;
            default:
                printf("UNEXPECTED\n");
                exit(EXIT_FAILURE);
            }
            zmq_msg_close(&part);
            ++arg;
            if (!zmq_msg_more(&part))
                break;
        }
        if (current == EXIT) {
            if (has_child)
                send_exit(child_socket);
            break;
        }

        int replied = 0;
        int not_replied = 0;
        char *reply = (char *)calloc(sizeof(char), STR_LEN);

        if (current == CREATE) {
            if (parent_id == self_id) {
                if (has_child == 0) {
                    int fork_val = fork();
                    if (fork_val == 0)
                        create_worker(target_id, 0);

                    child_socket = zmq_socket(context, ZMQ_REQ);
                    has_child = 1;
                    int opt_val = 0;
                    int rc = zmq_setsockopt(child_socket, ZMQ_LINGER, &opt_val, sizeof(opt_val));
                    assert(rc == 0);
                    if (child_socket == NULL) {
                        perror("socket");
                        exit(EXIT_FAILURE);
                    }

                    rc = zmq_connect(child_socket, portname_client(BASE_PORT + target_id));
                    assert(rc == 0);

                    child_id = target_id;

                    sprintf(reply, "Created node %d with PID %d", target_id, fork_val);
                    replied = 1;
                } else {
                    zmq_close(child_socket);
                    int fork_val = fork();
                    if (fork_val == 0)
                        create_worker(target_id, BASE_PORT + child_id);

                    child_socket = zmq_socket(context, ZMQ_REQ);
                    has_child = 1;
                    int opt_val = 0;
                    int rc = zmq_setsockopt(child_socket, ZMQ_LINGER, &opt_val, sizeof(opt_val));
                    assert(rc == 0);
                    if (child_socket == NULL) {
                        perror("socket");
                        exit(EXIT_FAILURE);
                    }

                    rc = zmq_connect(child_socket, portname_client(BASE_PORT + target_id));
                    assert(rc == 0);

                    child_id = target_id;

                    sprintf(reply, "Created node %d with PID %d", target_id, fork_val);
                    replied = 1;
                }

            } else {
                if (has_child) {
                    send_create(child_socket, target_id, parent_id);
                    if (available_recive(child_socket)) {
                        const char *reply_child = get_reply(child_socket);
                        if (strcmp(EMPTY_MSG, reply_child) != 0) {
                            sprintf(reply, "%s", reply_child);
                            replied = 1;
                        }
                    }
                }
            }
        }

        else if (current == EXEC) {

            if (target_id == self_id) {
                switch (current_sub) {
                case START:
                    gettimeofday(&start, NULL);
                    timer = ON;
                    break;
                case STOP:
                    if (timer == ON) {
                        gettimeofday(&finish, NULL);
                        timer = OFF;
                        diff_sec = finish.tv_sec - start.tv_sec;
                        diff_msec = (diff_sec * 1000) + (finish.tv_usec - start.tv_usec) / 1000;
                    }
                    break;
                case TIME:
                    if (timer == ON) {
                        gettimeofday(&finish, NULL);
                        diff_sec = finish.tv_sec - start.tv_sec;
                        diff_msec = (diff_sec * 1000) + (finish.tv_usec - start.tv_usec) / 1000;
                    }
                    break;
                default:
                    break;
                }

                char *result = message_prefix(self_id, current_sub);

                if (current_sub == TIME) {
                    const char *time_string = int_to_string((unsigned)diff_msec);
                    result = strcat(result, time_string);
                }
                strcpy(reply, result);
                replied = 1;
            } else {
                if (has_child) {
                    send_exec(child_socket, current_sub, target_id);
                    if (available_recive(child_socket)) {
                        char *reply_child = get_reply(child_socket);
                        if (strcmp(EMPTY_MSG, reply_child) != 0) {
                            sprintf(reply, "%s", reply_child);
                            replied = 1;
                        }
                    }
                }
            }
        }

        else if (current == REMOVE) {
            if (target_id == child_id) {
                send_exit(child_socket);
                zmq_close(child_socket);
                has_child = 0;
                sprintf(reply, "Successfully removed node %d from system", target_id);
                replied = 1;
            } else {
                if (has_child) {
                    send_remove(child_socket, target_id);
                    if (available_recive(child_socket)) {
                        char *reply_child = get_reply(child_socket);
                        if (strcmp(EMPTY_MSG, reply_child) != 0) {
                            sprintf(reply, "%s", reply_child);
                            replied = 1;
                        }
                    }
                }
            }
        }

        else if (current == PINGALL) {

            if (has_child) {
                send_pingall(child_socket, count + 1);
                if (!available_recive_pingall(child_socket, count)) {
                    reply = strcat(reply, int_to_string(child_id));
                    reply = strcat(reply, " ");
                    not_replied++;
                } else {
                    char *reply_child = get_reply(child_socket);
                    if (strcmp(EMPTY_MSG, reply_child) != 0) {
                        reply = strcat(reply, reply_child);
                        replied = 1;
                    }
                }
            }
        }

        if (replied == 0 && (current != PINGALL || (current == PINGALL && not_replied == 0)))
            strcpy(reply, EMPTY_MSG);
        size_t rep_len = strlen(reply) + 1;
        zmq_msg_t create_response;
        int rec = zmq_msg_init(&create_response);
        assert(rec != -1);
        zmq_msg_init_size(&create_response, rep_len);
        memcpy(zmq_msg_data(&create_response), reply, rep_len);
        zmq_msg_send(&create_response, parent, 0);
        zmq_msg_close(&create_response);
        free(reply);
    }

    zmq_close(parent);
    zmq_ctx_destroy(context);

    return 0;
}