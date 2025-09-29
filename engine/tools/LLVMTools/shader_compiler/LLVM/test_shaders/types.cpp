struct TemplateD;

template <typename T>
struct Templatee
{
    void dpd();
    T t;
};

struct D
{
    int a;
    int b;
};

int main()
{
    Templatee<int> ti;
    ti.t = 1;
    ti.dpd();
    D d = {1, 2};
    return 0;
}