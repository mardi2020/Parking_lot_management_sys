struct message_format {
    short message_id; // tag
    int len; // length
    char data[256]; // data
};
