# Night

An (interpreted?) programming language. Still in development. I've never made one before, so I don't really know what I'm doing, but it should work out in the end.

The `source.night` file is where you write the code. I'm also working on a logo, but I'm really bad at designing. I'm also thinking of making a website with documentation and a place to write code, but that won't be until the actual language is done.

Also, my first stable release will be coming up this weekend :)

---

**Language features**

- variable declaration, initialization, and assignment
- expressions (integer and boolean)
- print thingy (standard ouput)
- if statements (if, else if, and else)

I should make a documentation soon...

**In progress**

Currently I'm working on testing my language and making sure all the error messages correspond to the right code error. And that basically means testing every single possible line of code that could possibly be written. *sigh*. I'm also working on polishing up some things with decimals, and all that'll be in this week's release.

After that release, I'll start work on functions, local variables (variables declared inside curly brackets), and more! Stay tuned :)

---

**Example**

This is what the stable version will (hopefully) look like this weekend :)

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
            thanks recursion :)
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
