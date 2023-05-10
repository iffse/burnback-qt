Merge "star.1.4.brep";
//+
Field[1] = MathEval;
Field[1].F = "0.0001";
BackgroundField = 1;
//+
Curve Loop(1) = {6, -5, 4, -3, -2, -1, 8, 7};
//+
Plane Surface(1) = {1};
//+
Physical Curve("outlet", 9) = {7, 8};
//+
Physical Curve("inlet 0", 10) = {5, 4, 3, 2};
//+
Physical Curve("symmetry 90", 11) = {6};
//+
Physical Curve("symmetry 0", 12) = {1};

Physical Surface("domain") = {1};
