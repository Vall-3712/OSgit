#ifndef __LIST_TREE_H__
#define __LIST_TREE_H__

#include <stdio.h>
#include <stdlib.h>

#define INIT_CAPACITY 5

typedef unsigned short id;

typedef struct _node {
    id node_id;
    struct _node *child;
    int has_child;
} Node;

typedef Node *List;

Node *create_node(id init_id);
int exists_list(Node *root, id target_id);
int exists(List *system, id target_id, int count);
Node *add_node(Node *root, id parent_id, id init_id);
Node *remove_node(Node *root, id init_id);
void print_list(const Node *root);

#endif