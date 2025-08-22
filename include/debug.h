#pragma once

#ifdef DEBUG

const char* type_to_str(struct token* tok){
    char str[16];
    switch(tok->category){
        case CATEGORY_SERVICE:
            switch(tok->service){
                case SERV_ILLEGAL: return strcpy(str, "ILLEGAL");                
                default: return strcpy(str, "SERVICE"); 
            }
            break;
        case CATEGORY_LITERAL:
            switch(tok->value){
                case LIT_IDENT: return strcpy(str, "IDENT");            
                case LIT_STRING: return strcpy(str, "STRING");            
                case LIT_TRUE: case LIT_FALSE: return strcpy(str, "BOOLEAN");            
                default: return strcpy(str, "NUMBER");
            }
            break;
        case CATEGORY_DATATYPE: return strcpy(str, "DATATYPE");
        case CATEGORY_KEYWORD: return strcpy(str, "KEYWORD");
        case CATEGORY_DELIMITER: return strcpy(str, "DELIMITER");
        case CATEGORY_OPERATOR: return strcpy(str, "OPERATOR");
        case CATEGORY_MODIFIER: return strcpy(str, "MODIFIER");
        case CATEGORY_PAREN: return strcpy(str, "PAREN");        
        default: return strcpy(str, "UNKNOWN");
    }
}

#define print_token(t)                         \
printf("%s(%s)\n", type_to_str(t), (t)->literal)
#else
#define print_token(t)
#endif