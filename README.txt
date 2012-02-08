Checkpointable Box2D
--------------------

This is a (slightly) tweaked version of Box2D which allows for checkpointing.
Nothing of the library itself is changed (except a tweak to the StackAllocator
to save on memory).

The real magic is in the driver code that tracks allocations and stores memory
diffs.


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
cd ../.. #we were in the box2d/Build subdirectory.
make

#NOTE: the example code requires libSDL and expects sdl-config to be in
# the path. Also, beware llvm-gcc on OSX; it's a buggy compiler.

5) Run the example app:
./checkpoint_example


Who owns this stuff
-------------------
I (Jim McCann) hereby release my changes to Box2D and the example code I
provide herein into the public domain (without any warranty).
If you use the code and like it, or have some questions, or are just bored,
please feel free to drop me an e-mail at ix@tchow.com .
