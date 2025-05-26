#include <stdio.h>

int main() {
    int x = 5;
    int y = 10;
    int z = x + y * 2;
    
    if (z > 20) {
        z = z - 5;
    } else {
        z = z + 5;
    }
    
    return z;
}
