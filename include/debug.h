#ifdef DEBUG

const char *token_type_to_str(enum category_tag category){
    static const char *names[] = {
        "SERVICE", "OPERATOR", "KEYWORD", "PAREN",
        "DELIMITER", "DATATYPE", "VALUE", "MODIFIER"};
    return (category < sizeof(names) / sizeof(names[0])) ? names[category] : "UNKNOWN";
}

#define print_token(t)                         \
printf("%s(%s)\n", token_type_to_str((t)->category), (t)->literal)
#else
#define print_token(t)
#endif