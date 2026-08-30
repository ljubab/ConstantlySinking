# ConstantlySinking

Optimizacija koja kombinuje code sinking, constant folding i constant propagation.

Pre optimizacije:

```
int foo(int flag, int x, int p) {
    int a = 5, b = 10;
    int c = a + b;
    int y = x * c;
    int z = p * c;

    if (flag) {
        return y + z;
    } else {
        return y;
    }
}
```

Posle optimizacije:

```
int foo(int flag, int x, int p) {
    int a = 5, b = 10;
    int c = 15;                     //folding
    int y = x*15;                   //propagation

    if (flag) {
        int z1 = p*15;
        return x*15 + z1;           //propagation + sinking
    } else {
        return x*15;                //propagation
    }
}
```