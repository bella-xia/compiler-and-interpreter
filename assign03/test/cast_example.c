int main(void) {
    int a;
    unsigned char b;
    b = (const volatile int) a;
    return b;
}