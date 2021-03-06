void test_strl() {
    llog("\t+ Running test_strl... ");
    char* str;
    struct str_list* strl = strl_allocate();

    strl_append(strl, "hello");
    str = strl_get(strl, 0);
    assert(strcmp(str, "hello") == 0);
    assert(strl_len(strl) == 1);
    assert(strl->capacity == INITIAL_STRL_LENGTH);

    str = strl_get(strl, 1);
    assert(str == NULL);

    strl_append(strl, "the");
    str = strl_get(strl, 1);
    assert(strcmp(str, "the") == 0);
    assert(strl_len(strl) == 2);
    assert(strl->capacity == INITIAL_STRL_LENGTH);

    strl_append(strl, "quick");
    str = strl_get(strl, 2);
    assert(strcmp(str, "quick") == 0);
    assert(strl_len(strl) == 3);
    assert(strl->capacity == INITIAL_STRL_LENGTH);

    strl_append(strl, "brown");
    str = strl_get(strl, 3);
    assert(strcmp(str, "brown") == 0);
    assert(strl_len(strl) == 4);
    assert(strl->capacity == INITIAL_STRL_LENGTH);

    strl_append(strl, "fox");
    str = strl_get(strl, 4);
    assert(strcmp(str, "fox") == 0);
    assert(strl_len(strl) == 5);
    assert(strl->capacity == INITIAL_STRL_LENGTH);

    strl_append(strl, "jumps");
    str = strl_get(strl, 5);
    assert(strcmp(str, "jumps") == 0);
    assert(strl_len(strl) == 6);
    assert(strl->capacity == INITIAL_STRL_LENGTH);

    strl_append(strl, "over");
    str = strl_get(strl, 6);
    assert(strcmp(str, "over") == 0);
    assert(strl_len(strl) == 7);
    assert(strl->capacity == INITIAL_STRL_LENGTH);

    strl_append(strl, "the");
    str = strl_get(strl, 7);
    assert(strcmp(str, "the") == 0);
    assert(strl_len(strl) == 8);
    assert(strl->capacity == INITIAL_STRL_LENGTH);

    strl_append(strl, "lazy");
    str = strl_get(strl, 8);
    assert(strcmp(str, "lazy") == 0);
    assert(strl_len(strl) == 9);
    assert(strl->capacity == INITIAL_STRL_LENGTH);

    strl_append(strl, "dog");
    str = strl_get(strl, 9);
    assert(strcmp(str, "dog") == 0);
    assert(strl_len(strl) == 10);
    assert(strl->capacity == INITIAL_STRL_LENGTH);

    // Doubling of capacity should happen now.
    strl_append(strl, "last straw");
    str = strl_get(strl, 10);
    assert(strcmp(str, "last straw") == 0);
    assert(strl_len(strl) == 11);
    assert(strl->capacity == INITIAL_STRL_LENGTH * 2 + 1);

    strl_append(strl, "and then some");
    str = strl_get(strl, 11);
    assert(strcmp(str, "and then some") == 0);
    assert(strl_len(strl) == 12);
    assert(strl->capacity == INITIAL_STRL_LENGTH * 2 + 1);

    strl_deallocate(strl);
    llog("Success!!!\n");
}

void test_stack() {
    initialiseStack();
    struct rule *rule = (struct rule *) malloc(sizeof(struct rule));
    rule->non_terminal = _PROGRAM;
    rule->head = newRule(_VALUE, PLUS, TERMINAL);
    rule->head->next = newRule(_OP1, EPSILON, NON_TERMINAL);
    rule->head->next->next = newRule(_OP2, EPSILON, NON_TERMINAL);

    // struct stackNode *stack_node = (struct stackNode *) malloc(sizeof(struct stackNode));
    // stack_node->symbol.non_terminal = rule->non_terminal;
    // stack_node->flag = NON_TERMINAL;
    // stack_node->next = NULL;
    // push(stack_node);
    // pushRuleIntoStack(rule);
    printStack();
}


void test_hashMap() {
    llog("\t+ Running test_hashMap... ");
    struct hashMap *hash_map = initialiseHashMap();

    int count = 10;
    char *keys[] = {"hello", "olleh", "the", "quick", "brown",
                    "fox", "jumps", "over", "the", "lazy dog"};
    int vals[] = {9392, 9204, 8809, 8384, 8840,
                  9458, 9647, 9668, 9835, 8460};

    for (int i = 0; i < count; ++i) {
        hashMapPut(keys[i], vals[i], hash_map);
    }

    assert(hashMapGet("hello", hash_map) == 9392);
    assert(hashMapGet("olleh", hash_map) == 9204);
    assert(hashMapGet("quick", hash_map) == 8384);
    assert(hashMapGet("brown", hash_map) == 8840);
    assert(hashMapGet("fox", hash_map) == 9458);
    assert(hashMapGet("jumps", hash_map) == 9647);
    assert(hashMapGet("over", hash_map) == 9668);
    assert(hashMapGet("the", hash_map) == 9835);  // Overwrite the previous value.
    assert(hashMapGet("lazy dog", hash_map) == 8460);

    deallocateHashMap(hash_map);

    llog("Success!!!\n");
}

// void populateGrammar() {
//     extern grammar G;

//     // E -> TE'
//     G[0].non_terminal = E;
//     G[0].head = newRule(T, EPS_, NON_TERMINAL);
//     G[0].head->next = newRule(E_, EPS_, NON_TERMINAL);
//     // E' -> +TE'
//     G[1].non_terminal = E_;
//     G[1].head = newRule(E, PLUS_, TERMINAL);
//     G[1].head->next = newRule(T, EPS_, NON_TERMINAL);
//     G[1].head->next->next = newRule(E_, EPS_, NON_TERMINAL);
//     // E' -> eps
//     G[2].non_terminal = E_;
//     G[2].head = newRule(E, EPS_, TERMINAL);
//     // T -> FT'
//     G[3].non_terminal = T;
//     G[3].head = newRule(F_, EPS_, NON_TERMINAL);
//     G[3].head->next = newRule(T_, EPS_, NON_TERMINAL);
//     // T' -> *FT'
//     G[4].non_terminal = T_;
//     G[4].head = newRule(E, MUL_, TERMINAL);
//     G[4].head->next = newRule(F_, EPS_, NON_TERMINAL);
//     G[4].head->next->next = newRule(T_, EPS_, NON_TERMINAL);
//     // T' -> eps
//     G[5].non_terminal = T_;
//     G[5].head = newRule(E, EPS_, TERMINAL);
//     // F -> id
//     G[6].non_terminal = F_;
//     G[6].head = newRule(E, ID_, TERMINAL);
//     // F -> (E)
//     G[7].non_terminal = F_;
//     G[7].head = newRule(E, BO_, TERMINAL);
//     G[7].head->next = newRule(E, EPS_, NON_TERMINAL);
//     G[7].head->next->next = newRule(E, BC_, TERMINAL);
// }
