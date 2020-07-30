# Night

An (interpreted?) programming language. Still in early development. I've never made one before, so I don't really know what I'm doing, but it should work out in the end.

The `source.night` file is where you write the code, but I'm thinking once I'm done this language, I'm going to have a website where you can just write your code. I'm also working on a logo, but I'm really bad at designing. You can check it out in the `logo.png` file.

---

**Language features**

- variable types (bit, syb, int, dec, str)
- variable declaration, initialization, and assignment
- expressions (integer, boolean, and string concatenation)
- print (standard ouput)
- if statements (if, else if, and else)
- functions

**In progress**

So I just finsihed work on funciton parameters, so now you can call a function with a parameter! I'm still working on the error messages associated with it, and then some bugs with local variables.

---

**Example**

Here's and example showcasing everything my language can currently do:

```
print "Hello World!";

// this is a comment

bit boolean = true; // supports `! && || == != ( )` all of those
syb character = 'c';
int integer = 2 + 3; // supports '+ - / * % ( )' all of those
dec float = 3.14;
str string = "dynamic " + "squid"; // supports string concatenation

int answer; // a default value will be given:   false - bit,   ' ' - syb,   0 - int,   0.0 - dec,   "" - str
answer = 10;

if (string == "dynamic squid")
{
    print "hello " + string;
    
    // also supports nested if statements
    if (true)
    {
        if (true)
        {
            print "thanks recursion :)";
        }
    }
}
else if (string == "octopus")
{
    print "no";
}
else
{
    print "hi";
}

func helloFunction(str name)
{
    print "Hello " + name;
}

helloFunction("squid");
```
