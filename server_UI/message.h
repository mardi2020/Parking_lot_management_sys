#ifndef _MESSAGE_H
#define _MESSAGE_H
typedef struct message_format {
    short message_id; // tag
    int len; // length
    //short sub_id;
    char data[256]; // data
} message_format;

#endif