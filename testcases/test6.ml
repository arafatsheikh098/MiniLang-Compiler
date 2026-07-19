// Test 6: Comprehensive test — all language features
// Comments: single-line and block

/* 
   This program demonstrates:
   - Variable declarations (int)
   - Arithmetic (+, -, *, /)
   - Relational (<, >, ==, !=)
   - if-else conditionals
   - while loops
   - Block scoping
   - print statements
   - Unary minus
*/

// Declarations
int a = 5;
int b = 3;
int c = 0;

// Arithmetic
c = a + b;
print c;

c = a - b;
print c;

c = a * b;
print c;

c = a / b;
print c;

// While loop: count down
int count = 5;
while (count > 0) {
    print count;
    count = count - 1;
}

// If-else with nested blocks
int val = 50;
if (val > 25) {
    int inner = val * 2;
    print inner;
} else {
    print 0;
}

// Unary minus
int neg = -10;
print neg;

// Final computed value
int final = (a + b) * (a - b) + neg;
print final;
