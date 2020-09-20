# Night

An interpreted programming language that combines the simplicity of Python, with the type concepts of the C family.

You can read more about Night on its [website](https://night-website.dynamicsquid.repl.co/). You'll also find instructions on how to build this from source there.

---

**Night Overview**

Here's a sample of my current language:

```
print("Hello World!\n");

// this is a comment

bit boolean = true; // supports '! || && == != < > <= >= ( )' all of those
syb character = 'c'; // doesn't support any expressions yet
int integer = 10; // supports '+ - * / % ( )' all of those
dec float = 3.14; // same with 'int', but without 'mod'
str string = "squid"; // suports string concatenation with 'syb', 'int', 'dec', and 'str'

int answer = 10 + 5;
answer += 2 + 3;

int legs = 10;
bit smart = true;

if (legs == 10 && smart)
{
    print("Hi squid\n");
}
else if (legs == 8 && smart)
{
    print("Hi octopus\n");
}
else if (legs == 2 && !smart)
{
    print("Hi human\n");
}
else
{
    print("Not sure who you are\n");
}

int add(int a, int b)
{
    print("Adding to numbers:\n");
    return a + b;
}

int number = add(2, 3) + 4;
print(number + "\n");

loop (5)
{
    print("squid");
}

int[3] arr = [ 6, 6, 7 ];
arr[1] = 5;

int userAge = input();
```

Note that for the beta versions, the language is buggy and might not work sometimes.

---

**Progress Updates**

Here's a list of all the future relases I have planned:

- `v3.0` baseline for `v3`
  - while and for loops
  - better error checking
  - fluid arrays
  - source build
- `v3.1` arrays with functions
  - return arrays
  - array parameters
- `v3.2` [sqdlib](https://github.com/DynamicSquid/sqdlib)
  - standard library
  - package manager
