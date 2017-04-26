% X_0: positions
%  V : offset vector field
% Tri: vertex index in triangles
% DdDa: a_(36) -> d_(n)
function [ Min, Mout, Mio, DmDd_in, DmDd_out, DmDd_io ] = CompPhyProperty(X_0, V, Tri, DdDa, a, VERBOSE)
    n_p = size(X_0, 1);
    n_tri = size(Tri, 1);
    d = DdDa * a;
    % a1 = a0(k+1:end);

    P = X_0;
    Pout = P;
    Pin  = P + bsxfun(@times, d, V);
    Pio  = [Pin; Pout];


    % get points in 3-block (for each triangle) for inner
    pin   = Pin(Tri, :);
    % get points in 3-block (for each triangle) for outer
    pout  = Pout(Tri, :);
    % get points in 3-block (for each triangle) for all
    Tri_io = [ Tri(:, [2, 1, 3]) ; Tri + n_p];
    pio   = Pio(Tri_io, :);

    [ Min, ~, DmDd_in ] = ...
        comp_moments_grad_3d( pin(1:n_tri,:), pin(n_tri+1:2*n_tri,:), ...
        pin(2*n_tri+1:3*n_tri,:), Tri, V);

    [ Mout, ~, DmDd_out ] = ...
        comp_moments_grad_3d( pout(1:n_tri,:), pout(n_tri+1:2*n_tri,:), ...
        pout(2*n_tri+1:3*n_tri,:), Tri, -V);

    [ Mio, ~, DmDd_io ] = ...
        comp_moments_grad_3d( pio(1:2*n_tri,:), pio(2*n_tri+1:4*n_tri,:), ...
        pio(4*n_tri+1:6*n_tri,:), Tri_io, [V; -V]);

    %% test
%     if (strcmp(VERBOSE, 'v'))
%         ShowDiffModel(Pout, Pin, Tri);
%         pause;
%     end
    f = Min;
end