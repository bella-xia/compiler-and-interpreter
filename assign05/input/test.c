void glad(int a[0], int b) {
    b;
}

int happy(int a[0], int b, int c) {
    glad(a, b);
    return b;
}

int main(void) {
    int arr[5], b, c;
    happy(arr, b, c);
    return 0;
}