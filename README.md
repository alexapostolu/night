# Night

**I did not make the majority of this, [DynamicSquid](https://github.com/DynamicSquid) did. This branch is intended to have a less strict type system. This website is for my fork: [![Run on Repl.it](https://repl.it/badge/github/RZED786/Night)](https://repl.it/github/RZED786/Night)**

An interpreted programming language. It combines the simplicity of Python, with the type concepts of the C family.

It's still in early development, and I have a lot left to do. I'm also working on a [website](https://night-web.dynamicsquid.repl.co/) which you can definitly check out! There's even a place to code in there! I'm also working on a logo, can you can check that out in the `logo.png` and `logo3.png` files.

Here's a sample of my current language:

```
print("Hello World!\n");

// this is a comment

bit boolean = true; // supports '! || && == != < > <= >= ( )' all of those
syb character = 'c'; // doesn't support any expressions yet
int integer = 10; // supports '+ - * / % ( )' all of those
dec float = 3.14; // same with 'int', but without 'mod'
str string = "squid"; // suports string concatenation

int answer = 10 + 5;
answer = 2 + 3;

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
```

There's still a lot to be done though. Here are the things I'm working on (in order):

- new operators: `+=`, `-=`, `/=`, `*=`, `%=`, `<`, `>`, `<=`, `>=`
- more predefined functions and small libraries
  - right now, the only predefined function is `print()`
- loops and arrays
- user input
- "include" thingy like in C++
- function definitions and overloading
- more support for expressions

---

**Progress Updates**

I just finished with `v2.0`, so now I'm working on the list above.
