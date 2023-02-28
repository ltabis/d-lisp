# d-lisp

An implementation of a custom Lisp language using C, based on the wonderful [Build Your Own Lisp book](https://buildyourownlisp.com/), that you can buy [here](https://www.amazon.com/Build-Your-Lisp-Daniel-Holden/dp/1501006622).

## Build

```bash
make
```

## Run

Running the interpreter.

```bash
./d-lisp
> print "Hello World!"
```

Evaluating files.

```bash
./d-lisp hello-world.dlsp fibonacci.dlsp
```

Check the examples directory to have an overview of the features of the language.
