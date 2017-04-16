n_vertices = size(Lap, 1);

X = X_0;

for i = 0:Iteration_Limit
    A = [Lap; Weight_Preserve * eye(n_vertices)];
    b = [zeros(n_vertices, 3); Weight_Preserve * X];
    X = A\b;
end
