# Night

An interpreted programming language. It combines the simplicity of Python, with the type concepts of the C family. It features basic syntax such as variables, if statements, and expressions. It also supports functions, however it is not stable.

It's still in early development, and I have a lot to do for `v2`.

Here's a sample of my current language:

```
print "Hello World!\n";

// this is a comment

bit boolean = true;
syb character = 'c';
int integer = 10;
dec float = 3.14;
str string = "squid";

bit smart = true;
int legs = 10;

if (smart && legs == 10)
{
    print "Hello squid!";
}
else if (smart && legs == 8)
{
    print "Hello octopus!";
}
else
{
    print "Hmm... not sure who you are";
}

func add(int a, int b)
{
    return a + b;
}

print add(1, 2);
```

---

Here's what I'm planning do add/fix for `v2`:

- complete refactor of code, makes it easier to mantain and develop
- new operators: `+= -= /= *= %= < > <= >=`
- predefined functions and small libraries
- explicit function return types
- function definitions
- function overloading
- loops and arrays
- implicit conversions: `"string" + 3` or `bit a = 0`
- user input

Stay tuned :)
