static void hexdump(void *p, uint16_t length)
{
    int i;
    int left;

    char *bp = (char *)p;

    while (length > 0) {
        left = length > 16 ? 16 : length;
        for (i = 0; i < left; i++) {
            printf("%02x ", bp[i] & 0xFF);
        }
        for (i = 0; i < 16 - left; i++) {
            printf("   ");
        }
        for (i = 0; i < left; i++) {
            printf("%c", (bp[i] > 32 && bp[i] < 126) ? bp[i] : '.');
        }
        bp += 16;
        length -= left;
        printf("\n");
    }
}


