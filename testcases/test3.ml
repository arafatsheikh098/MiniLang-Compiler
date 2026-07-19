// Test 3: Complex arithmetic + unary minus + block scoping
int a = 10;
int b = 20;
int c = a + b * 2;
print c;

int result = (a + b) * (c - a) / 5;
print result;

// Block scoping: inner variable shadows outer
int temp = 100;
{
    int temp = 999;
    print temp;
}
print temp;

// Unary minus
int neg = -42;
print neg;
