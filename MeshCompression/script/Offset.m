% Options
optimize_info.use_const_bound = true;
optimize_info.depth_bound = 0.03;


optimize_info.lower_bound = 0.1;
optimize_info.upper_bound = 0.9;
optimize_info.method      = 2;
% method: 1: buoyancy
% method: 2: static
% method: 3: spin
optimize_info.use_nonlcon = false;
optimize_info.w1  = [ 0.0, 100.0, 10.0, 10.0, 100.0, 0.01 ];
optimize_info.w2  = [ 0.0, 100.0, 50.0, 1e-2 ];
% optimize_info.w2  = [ -10.0, 0.0, 0.0, 0.01 ];
% [ M1,  Center_Z, /, panalty ]
optimize_info.rho = [ 1.2, 1.0 ];
optimize_info.max_iter = 40;
optimize_info.max_fun_evals = 150;
n_eigs = 18;

% %fish
% %% Options
% optimize_info.use_const_bound = true;
% optimize_info.depth_bound = 0.1;
% % optimize_info.use_const_bound = false;
% optimize_info.lower_bound = 0.05;
% optimize_info.upper_bound = 0.95;
% optimize_info.method      = 1;
% % method: 1: buoyancy
% % method: 2: static
% % method: 3: spin
% optimize_info.use_nonlcon = false;
% optimize_info.w1  = [ 0.0, 100.0, 10.0, 50.0, 0.0, 0.0 ];
% % optimize_info.w1  = [ 0.0, 100.0, 0.0, 10.0, 100.0, 0.01 ];
% optimize_info.w2  = [ 0.0, 100.0, 10.0, 1e-2 ];
% % optimize_info.w2  = [ -10.0, 0.0, 0.0, 0.01 ];
% % [ M1,  Center_Z, /, panalty ]
% optimize_info.rho = [ 1.2, 0.8 ];
% optimize_info.max_iter = 40;
% optimize_info.max_fun_evals = 1500;
% n_eigs = 36;


%%
n_vertices = size(Lap, 1);
n_tri = size(Tri, 1);

opts.v0 = full(ones(n_vertices, 1));
opts.issym = 1;
[eigV, eigD] = eigs(Lap, n_eigs, 'sm', opts);
DdDa = eigV;


[ X_in_result ] = Optimize( X_0, V, bounds, Tri, DdDa, 'v', optimize_info);

if Verbose == 1
    ShowDiffModel(X_0, X_in_result, Tri);
end