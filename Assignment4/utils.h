typedef struct packet
{
    char data[1024];
}Packet;

typedef struct frame
{
    int frame_kind; // Acknowledgement Frame:0, Data Frame:1
    int sq_no;
    Packet packet;
}Frame;