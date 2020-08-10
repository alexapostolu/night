# Night

An interpreted programming language. It combines the simplicity of Python, with the type concepts of the C family.

It's still in early development, and I have a lot left to do. I'm also working on a [website](https://night-web.dynamicsquid.repl.co/) which you can definitly check out!

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
bool smart = true;

if (legs == 10 && smart)
{
    print("Hi squid");
}
else if (legs == 8 && smart)
{
    print("Hi octopus");
}
else if (legs == 2 && !smart)
{
    print("Hi human");
}
else
{
    print("Not sure who you are");
}

int add(int a, int b)
{
    print("Adding to numbers:\n");
    return a + b;
}

int number = add(2, 3) + 4;
print(number);
```

There's still a lot to be done though. Here are the things I'm working on:

- new operators: `+=`, `-=`, `/=`, `*=`, `%=`, `<`, `>`, `<=`, `>=`
- predefined functions and small libraries
- function definitions and overloading
- loops and arrays
- user input
- more support for expressions
