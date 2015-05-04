#ifndef PARSER_H
#define PARSER_H

typedef enum {
    NODE_CMD    = 0,
    NODE_RDIR   = 1,
    NODE_READ   = 2,
    NODE_PIPE   = 3
} node_type;

struct parser_context {
  struct token  *       p_tokens;
  int                   p_position;  
  struct ast_node *     p_root;
};

struct ast_node {
    struct ast_node *   n_children;
    node_type           n_type;
    struct ast_node *   next;
};

struct gsh_command {
    struct ast_node     c_base;
    char **             c_args;
};

void init_parser(struct parser_context *ctx, struct token *tokens);
void uninit_parser(struct parser_context *ctx);
struct ast_node *parse(struct parser_context *ctx);

#endif
