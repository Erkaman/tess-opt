# tess-opt

Demonstration of how we can use tessellation shaders to make faster fragment shaders.

But it's not done yet!

## Building

If on Linux or OS X, you can build it in the terminal by doing:

```
mkdir build && cmake .. && make
```

you can then launch the app by using the launch script:

```
./launch-tess_opt.sh
```

If on Windows, create a `build/` folder, and run `cmake ..` from
inside that folder. This will create a visual studio solution(if you
have visual studio). Launch that solution, and then choose to compile the
project named `tess_opt`.
