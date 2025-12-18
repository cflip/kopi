## Generics
```java
public <T> void qsort(T[] array, int (*comparator)(T a, T b)) {
	// ...
	comparator(a, b);
	// ...
}

private int compareIntegers(int a, int b) {
	return a - b;
}

public int main(char[][] args) {
	int[] myArray = { 23, 66, 4, 6, 31, 432 };
	<int>qsort(myArray, compareIntegers);
	return 0;
}

```
