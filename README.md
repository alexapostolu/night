# Night

An (interpreted?) programming language. Still in early development. I've never made one before, so I don't really know what I'm doing, but it should work out in the end.

The `source.night` file is where you write the code, but I'm thinking once I'm done this language, I'm going to have a website where you can just write your code. I'll also include instructions on how to build this from source if you'd like to. I'm also working on a logo, but I'm really bad at designing. You can check it out in the `logo2.png` file.

---

**Language features**

- variable types (bit, syb, int, dec, str)
- variable declaration, initialization, and assignment
- expressions (integer, boolean, and string concatenation)
- print (standard ouput)
- if statements (if, else if, and else)

**In progress**

Currently I'm working on better decimal and string support, and improving other minor things. Then I'll start work on functions, predefined function, local variables and more! It might take me 2-3 weeks to get all of them done. Stay tuned :)

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
```
