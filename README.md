# Night

An interpreted programming language. It combines the simplicity of Python, with the type concepts of the C family.


It's still in early development, and I have a lot left to do. I'm also working on a [website](https://night-web.dynamicsquid.repl.co/) which you can definitly check out!

Currently, the website is the recommended place to code using Night, however support for a source build will be coming soon.

---

**Night Overview**

Here's a sample of my current language:

```night
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
```

## How to Use

### Configuring

1. Download and install a c/c++ compiler
2. Open Shell (cmd, bash, or powershell)
3. Change directory to Night/src folder (`cd %path to Night/src%`)
4. Type :

```bash
%path to compiler bin% -o Night.exe %path to Night.cpp%
```

e.g.:

```bash
D:/MinGW/bin/g++.exe -o Night.exe D:/VS+VSCodeField/VSCode/C-variant/Night/src/Night.cpp
```

### Running

### Making Usage Easier (Windows Only)

Type 'path' to search bar and this will show up:

![img1](/instructionimage/1.png "'Edit the system envirovment variable' Button")

Click the best match, and this will show up:

![img2](/instructionimage/2.png "'System Properties' Window")

Click 'Envirovement Variables':

![img3](/instructionimage/3.png "Button with 'Envirovement Variables' as the Label")

This will show up:

![img4](/instructionimage/4.png "'Envirovement Variables' Window")

Search and select 'Path' and click 'Edit':

![img5](/instructionimage/5.png "'Path' as System Variable and 'Edit...' Button")

Select and go to the end of 'Variable Value' and type ";%path to Night folder%/src":

![img6](/instructionimage/6.png "'Path Variable' Window")
![img7](/instructionimage/7.png "'D:\VS+VSCodeField\VSCode\C-variant\Night\src'")

And ";%path to compiler bin%":

![img8](/instructionimage/8.png "'D:\MinGW\bin'")

Press Oks:

![img11](/instructionimage/11.png "'Ok' button")
![img10](/instructionimage/10.png "'Ok' button")
![img9](/instructionimage/9.png "'Ok' button")

Note that for the beta versions, the language is buggy and might not work sometimes.

---

## Progress Updates

I just got done with `v2.3-beta`, which is a release about arrays. Now, I'm working on `v2.4-beta`, which is user input!

Here's a list of all the future relases I have planned:

- `v2.4-beta` user input
- `v3.0` baseline for `v3`
  - everything in `v2`
  - function def and overload
    - also support for recursion (maybe)
  - while and for loops
    - non-static conditions
  - cleaner code :)
- `v3.1` support for source build
  - with the help of [AntimatterReactor](https://github.com/AntimatterReactor)
- `v3.2` classes
