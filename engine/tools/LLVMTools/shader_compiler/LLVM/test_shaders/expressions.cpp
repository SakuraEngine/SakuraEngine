auto TestBinary() {
    // binary op
    int n = 0 + 2 - 56;
    n = n + 2;
    n = n + 2;

    // binary assign ops
    int m = n += 65, dm = 1 + 4;
    int x = n -= 65;
    int xx = n *= 65;
    int yy = n /= 65;
    int ww = n %= 65;

    int v = 5;
    int complex = v += 5;

    v = v + 54;

    return m + x + xx + yy + ww + complex;
}

void Test(int d)
{
    
}

int main()
{
    TestBinary();
    return 0;
}