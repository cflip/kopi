## Data Types

### Primitive types
The basic numerical data types are mostly the same as [Java's primitive data types](https://docs.oracle.com/javase/tutorial/java/nutsandbolts/datatypes.html). 

|Keyword|Bits|Format|
|-|-|-|
|`byte`|8|*Unsigned* integer
|`char`|8|*Signed* integer
|`short`|16|Signed integer
|`int`|32|Signed integer
|`long`|64|Signed integer
|`float`|32|IEEE754 floating point
|`size_t`|[platform dependent]|Unsigned integer|
|`double`|64|IEEE754 floating point
|`boolean`|[undefined]|`true` or `false`

You may also define unsigned integers by prepending any of the above signed integer types with the `unsigned` keyword.

Note that `byte` is unsigned and `char` is signed. This is because `char` represents 8-bit ASCII or UTF-8 encoded values, for consistency with C/C++.

New data types can be defined using the `type` keyword.

```c
public type medium_uint unsigned short;
```
Note that this syntax doesn't match that of C's `typedef` keyword. This is to be more consistent with the order of `struct`, `enum` and `union` definitions, i.e. the typename comes first, then the definition. This is also why the `type` keyword is used instead of `typedef`.

### Arrays
Arrays are fixed-with linear data structures.

Arrays in Kopi behave much more like their Java counterparts than in C. Unlike in C, arrays cannot decay into pointers unless explicitly told to do so. Instead, the compiler will always manage an implicit length field with the array, which can be accessed at any time using the `countof` operator.

Kopi arrays also use the same declaration style as Java arrays, with the square brackets to the right of the type name and not the variable name.

```java
import stdio;
import string;

void printAllMembers(int[] array) {
	for (int i = 0; i < countof(array); i++) {
		printf("%d ", array);
	}
	putc('\n');
}

public int main(char[][] args) {
	int[] copyFrom = { 4, 6, 23, 66, 31, 432 };
	int[3] copyTo;

	// The `arraycopy` function has the same parameters as Java's System.arraycopy,
	arraycopy(copyFrom, 2, copyTo, 0, 3);

	printAllMembers(copyFrom);
	printAllMembers(copyTo);

	return 0;
}
```

Arrays can be dynamically allocated as well, in which case they will be stored as an object containing a pointer to the array data and an integer containing the array length. 

Dynamically-allocated array
```java
private int[] createAnArray() {
    int *memory = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        memory[i] = i;
    }
    // Construct an array using a pointer to the start of the memory and the number of elements
    return [memory | size];
}

public int main() {
    int[] numbers = createAnArray();

    // Access the number of elements using countof()
    for (int i = 0; i < countof(numbers); i++) {
        printf("%d\n", numbers[i]);
    }

    // Access the memory pointer using the & operator
    free(&arrayMem);

    return 0;
}
```

### Enums
Although I use them often, I've always found C's enums lacking, and Kopi brings the full power of Java's enums to a C-like language.

Example using enums for a table data structure (based on [Oracle's Java tutorials](https://docs.oracle.com/javase/tutorial/java/javaOO/enum.html)).
```java
import stdio;

// universal gravitational constant  (m3 kg-1 s-2)
final double G = 6.67300E-11;

enum Planet {
	// There are no constructors so we use C-style initializer lists instead.
    MERCURY = { 3.303e+23, 2.4397e6 },
    VENUS   = { 4.869e+24, 6.0518e6 },
    EARTH   = { 5.976e+24, 6.37814e6 },
    MARS    = { 6.421e+23, 3.3972e6 },
    JUPITER = { 1.9e+27,   7.1492e7 },
    SATURN  = { 5.688e+26, 6.0268e7 },
    URANUS  = { 8.686e+25, 2.5559e7 },
    NEPTUNE = { 1.024e+26, 2.4746e7 };

    double mass;   // in kilograms
    double radius; // in meters
};

double surfaceGravity(Planet p) {
	return G * p.mass / (p.radius * p.radius);
}

double surfaceWeight(Planet p, double otherMass) {
	return otherMass * surfaceGravity(p);
}

public int main(char[][] args) {
	if (countof(args) != 2) {
		printf("Usage: %s <earth_weight>\n", args[0]);
		return -1;
	}

	double earthWeight = parseDouble(args[1]);
	double mass = earthWeight / surfaceGravity(Planet.EARTH);

	for (Planet p : Planet.values()) {
		printf("Your weight on %s is %f\n", p, surfaceWeight(p, mass));
	}
	return 0;
}
```
