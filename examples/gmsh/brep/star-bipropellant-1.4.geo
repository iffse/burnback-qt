Merge "star-bipropellant-1.4.brep";

Physical Curve("inlet 0", 10) = {6, 5, 4, 3};
Physical Curve("symmetry 90", 11) = {7};
Physical Curve("symmetry 0", 12) = {2};
Physical Curve("outlet", 14) = {8, 1};

Curve Loop(1) = {7, -6, 5, 9, 8};
Plane Surface(1) = {1};
Curve Loop(2) = {1, -9, -4, -3, -2};
Plane Surface(2) = {2};

Physical Surface("recession 1", 15) = {1};
Physical Surface("recession 0.5", 16) = {2};
