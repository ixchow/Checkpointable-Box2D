Checkpointable Box2D
--------------------
This is a (slightly) tweaked version of Box2D which allows for checkpointing.
Nothing of the library itself is changed (except a tweak to the StackAllocator
to save on memory).

The real magic is in the code that tracks allocations and stores memory
diffs -- this happens in SimMem.*pp and Sim.*pp's b2Alloc/b2Free functions.
Basically, you can do:

Sim sim; //the simulation.

SimMem mem;
sim.save(mem);
//do whatever with sim, e.g. sim.tick()...
sim.load(mem);
//sim is now in *exactly* the same state is was.

//NOTE:
// you shouldn't try to load memory saved by another 'Sim' object, or have
// more than one Sim object per thread.
Sim otherSim; //BAD! Will assert()
otherSim.load(mem); //WORSE! Will probably segfault!


The demo program shows how this is useful for time control and saving replays.
Use the left and right arrows to control the circle.
Use the keys 'a' and 'd' to rewind and fast-forward time.
Press 'SPACE' to start a replay from the beginning of the level.

Time control is a simple as storing SimMem's during play and doing a
sim.load() to go back.

Replays take advantage of the fact that a simulator which has loaded a SimMem
is guaranteed to be in *exactly* the same state, so just re-simulates from the
beginning with the same control signals. (It even uses the *same Sim* for the
replay and the main play, by switching it between active states with save and
load!)


Substantial memory savings can be had by calling sim.save(mem, rel) to store
memory diffs relative to a recent 'keyframe' of memory.


How to make it go
-----------------
1) Get yourself Box2D version 241 (though probably more recent versions will
continue to work -- the changes I made to the internals are minimal):
svn co -r241 http://box2d.googlecode.com/svn/trunk box2d

2) Apply the patch:
cd box2d
patch -p0 -i ../patch_for_r241.diff

3) Build Box2D:
cd Box2D
cd Build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBOX2D_BUILD_EXAMPLES=OFF ..
make

#NOTE: I have to comment out lines 199 and 203 of Box2D/Box2D/CMakeLists.txt
# to make the above cmake command work. Your results may vary.

4) Build the example app and driver code:
cd ../../.. #we were in the box2d/Box2D/Build subdirectory.
make

#NOTE: the example code requires libSDL and expects sdl-config to be in
# the path. Also, beware llvm-gcc on OSX; it's a buggy compiler.

5) Run the example app:
./main


Who owns this stuff
-------------------
I (Jim McCann) hereby release my changes to Box2D and the example code I
provide herein into the public domain (without any warranty).
If you use the code and like it, or have some questions, or are just bored,
please feel free to drop me an e-mail at ix@tchow.com .
