# Night

An interpreted programming language that combines the simplicity of Python, with the type concepts of the C family.

It's still in early development, and I have a lot left to do. I'm also working on a [website](https://night-web.dynamicsquid.repl.co/) which you can definitly check out!

Currently, the website is the recommended place to code using Night, however support for a source build will be coming soon.

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

I just got done with `v2.4-beta`, which is a release about user input. Now, I'm working on `v3.0`, which will be the first non-beta release!

Here's a list of all the future relases I have planned:

- `v3.0` baseline for `v3`
  - function definitions and overloading
  - while and for loops
  - better error checking
  - fluid arrays
  - source build
  - cleaner code :)
- `v3.1` [sqdlib](https://github.com/DynamicSquid/sqdlib)