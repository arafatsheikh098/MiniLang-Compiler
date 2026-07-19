// Test 5: Factorial computation using while loop
int n = 6;
int fact = 1;
int i = 1;

while (i < n + 1) {
    fact = fact * i;
    i = i + 1;
}

// Expected: 720 (6!)
print fact;
