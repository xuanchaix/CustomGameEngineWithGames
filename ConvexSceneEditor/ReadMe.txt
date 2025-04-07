------------Convex Scene Editor SD4 A1---------------
How to Use
S/E=Ray Start/End
W/R=Rotate Object
L/K=Scale Object
LMB=Drag Object
F2=Draw Mode
F3-Show Debug BVH AABB2 Tree
F4=Show Discs
F8=Randomize
N/M=halve/double number of objects
T=Test with t random rays 
Y/U=halve/double number of rays t

Known Issues
Float precision issues: sometimes optimized ray cast will reject extra objects(hit less), and all of my optimization methods will have the same issue if these objects are small.

Deep Learning
How many raycasts/ms can you do vs. 16 objects?  256?  1024?
How does this improve (or worsen!) when you enable your chosen hashing/partitioning scheme?
scene size: 200x100 scene bounding disc radius range: 4.0-16.0
16 objects fastest: disc rejection Avg 1.6ms for 8192 rays no optimization: 4.3ms
32 objects fastest: disc rejection Avg 3.0ms for 8192 rays no optimization: 10.2ms
64 objects fastest: disc rejection Avg 6.4ms for 8192 rays no optimizaiton: 21.0ms
256 objects fastest: AABB2 Tree(BVH) Avg 25ms for 8192 rays no optimizaiton: 90ms
1024 objects fastest: AABB2 Tree(BVH) Avg 90ms for 8192 rays no optimizaiton: 360ms
2048 objects fastest: Symmetric Quadtree Avg 175ms for 8192 rays no optimizaiton: 710ms
when there are less objects in the scene, partitioning scheme will make the process slower
when there are a lot of objects in the scene, these schemes will perform better

How do these speeds compare in each build configuration (Debug, DebugInline, FastBreak, Release)?
256 objects in debug: 370ms
256 objects in debug inline: 380ms
256 objects in fast break: 175ms
256 objects in release: 90ms

Any general trends you can observe, i.e. the speed seems to be O(N) or O(N2) with #objects, #rays, etc.
The time consumption seems to be doubled if the num of objects doubled, but it will be a little bit smaller than the actual doubled time consumption number.

Any data specific to your hashing/partitioning scheme you can observe (e.g. AABB Tree depth)
Fot symmetric quadtree, a good depth is needed and unrelated to num of objects in the scene
For AABB2, the performance will be differ if the num of objects changes, and we need to test for a good depth for certain objects

Anything else interesting you observe about your results
Space partitioning schemes performs better when the objects are more sparse.

All speed measurements (other than cross-build comparisons) are taken in Release builds

