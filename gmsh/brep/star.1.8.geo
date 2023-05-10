Merge "star.1.8.brep";
//+
Curve Loop(1) = {3, -2, 1, 5, 4};
//+
Plane Surface(1) = {1};
//+
Physical Curve("outlet", 6) = {4};
//+
Physical Curve("symmetry 90", 7) = {3};
//+
Physical Curve("symmetry 45", 8) = {5};
//+
Physical Curve("inlet 0", 9) = {2, 1};

Physical Surface("domain") = {1};

Field[1] = MathEval;
Field[1].F = "0.0001";
BackgroundField = 1;
