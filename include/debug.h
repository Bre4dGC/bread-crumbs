#pragma once

#ifdef DEBUG

static inline const char* type_to_str(const struct token* tok){
    if (!tok) return "NULL";
    switch(tok->category){
        case CATEGORY_SERVICE:
            switch(tok->type_service){
                case SERV_ILLEGAL: return "ILLEGAL";
                default: return "SERVICE";
            }
        case CATEGORY_LITERAL:
            switch(tok->type_literal){
                case LIT_IDENT: return "IDENT";
                case LIT_STRING: return "STRING";
                case LIT_TRUE: case LIT_FALSE: return "BOOLEAN";
                default: return "NUMBER";
            }
        case CATEGORY_DATATYPE: return "DATATYPE";
        case CATEGORY_KEYWORD: return "KEYWORD";
        case CATEGORY_DELIMITER: return "DELIMITER";
        case CATEGORY_OPERATOR: return "OPERATOR";
        case CATEGORY_MODIFIER: return "MODIFIER";
        case CATEGORY_PAREN: return "PAREN";
        default: return "UNKNOWN";
    }
}

// static inline void ast_print(struct ast_node* node, int indent) {
//     if (!node) return;

//     for (int i = 0; i < indent; i++) {
//         printf("  ");
//     }
    
//     switch (node->type) {
//         case NODE_LITERAL: printf("Literal: %s\n", node->literal.value);
//             break;            
//         case NODE_VAR_REF: printf("VarRef: %s\n", node->var_ref.name);
//             break;            
//         case NODE_BIN_OP: 
//             printf("BinOp: %s\n", operator_to_str(node->bin_op.code));
//             ast_print(node->bin_op.left, indent + 1);
//             ast_print(node->bin_op.right, indent + 1);
//             break;            
//         case NODE_VAR:
//             printf("VarDecl: %s\n", node->var_decl->name);
//             ast_print(node->var_decl->value, indent + 1);
//             break;            
//         case NODE_BLOCK:
//             printf("Block:\n");
//             for (size_t i = 0; i < node->block.count; i++) {
//                 ast_print(node->block.statements[i], indent + 1);
//             }
//             break;
//     }
// }

#define print_token(t) do { if (t) printf("\033[34m%s\033[0m(\033[1m%s\033[0m)\n", type_to_str(t), (t)->literal ? (t)->literal : ""); } while(0)
#define print_node(n, i) ast_print(n, i)
#else
#define print_token(t)
#endif
