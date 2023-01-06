#include "list.h"

Node *create_node(id init_id) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->node_id = init_id;
    new_node->has_child = 0;
    new_node->child = (Node *)NULL;

    return new_node;
}

int exists_list(Node *root, id target_id) {
    if (root == NULL)
        return 0;
    if (root->node_id == target_id)
        return 1;

    int result = 0;
    result |= exists_list(root->child, target_id);
    return result;
}

int exists(List *root, id target_id, int count) {
    int result = 0;
    for (int i = 0; i < count; ++i) {
        result |= exists_list(root[i], target_id);
    }
    return result;
}

Node *add_node(Node *root, id parent_id, id new_id) {
    if (root->node_id == parent_id) {
        if (root->has_child) {
            Node *tmp = root->child;
            root->child = create_node(new_id);
            (root->child)->child = tmp;
            return root;
        }
        root->child = create_node(new_id);
        root->has_child = 1;
        return root;
    }

    if (root->has_child == 0)
        return root;

    root->child = add_node(root->child, parent_id, new_id);

    return root;
}

Node *delete_list(Node *root) {
    if (root->has_child == 0) {
        free(root);
        return NULL;
    }

    root->child = delete_list(root->child);
    free(root);

    return NULL;
}

Node *remove_node(Node *root, id target_id) {
    if (root->node_id == target_id)
        return delete_list(root);

    if (root->has_child == 0)
        return root;

    root->child = remove_node(root->child, target_id);
    return root;
}

void print_list_rec(const Node *root) {
    if (root == NULL) {
        return;
    }
    if (root->node_id != (id)-1)
        printf("%d ", root->node_id);
    print_list_rec(root->child);
}

void print_list(const Node *root) {
    printf("  ");
    print_list_rec(root);
    printf("\n");
}