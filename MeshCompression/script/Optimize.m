function [ X_in_result ] = Optimize( X_0, V, bounds, Tri, DdDa, VERBOSE, optimize_info)

%%
    M1 = 1;
    CX = 2; CY = 3; CZ = 4;
    XX = 5; YY = 6; ZZ = 7;
    XY = 8; YZ = 9; ZX = 10;
    w1 = optimize_info.w1;
    w2 = optimize_info.w2;
    rho = optimize_info.rho;
    lower_bound = optimize_info.lower_bound;
    upper_bound = optimize_info.upper_bound;
    r_ = 0.05;

    % global
    P_in  = [];
    P_out = [];
    P_shell  = [];
    DmDd_in  = [];
    DmDd_out = [];
    DmDd_shell  = [];
    DP_shell = [];
    DP_out = [];

    method = optimize_info.method;
    if method == 1
        Target = @Target1;
        NonLcon = @NonLcon1;
    elseif method == 2
        Target = @Target2;
        NonLcon = @NonLcon2;
    elseif method == 3
        Target = @Target3;
        NonLcon = @NonLcon3;
    end

    if optimize_info.use_nonlcon == false
        NonLcon = @NonLcon0;
    end


%%
    np = size(X_0, 1);
    k  = size(DdDa, 2);
    a0 = DdDa \ ((upper_bound - lower_bound) * bounds);
    A  = [DdDa; - DdDa];
    b  = [upper_bound * bounds; - lower_bound * bounds];

    options = optimset(...
        'Algorithm', 'active-set',...
        'GradObj','on',...
        'GradConstr','on',...
        'Display', 'iter',...
        'MaxFunEvals', optimize_info.max_fun_evals*size(X_0, 1),...
        'TolCon',1e-3,... % Tolerance on the constraint violation, a positive scalar. The default is 1e-6.
        'TolFun',1e-3,... % Termination tolerance on the function value, a positive scalar. The default is 1e-6.
        'TolX', 1e-5, ... % Relative changes in all elements of x is the normalized step vector. The default is 1e-10.
        'Hessian','bfgs',... % 'bfgs'{'lbfgs',positive integer}
        'MaxIter', optimize_info.max_iter);

    tic;
    a0 = DdDa \ (((upper_bound - lower_bound) / 2) * bounds);
    fprintf('intial: a0 = %f\n', a0);

    [a,fval,exitflag] = fmincon(Target, a0,   A, b,  [], [],  [], [],  NonLcon, options );
    time = toc;
    fprintf('Time use: %f\n', time);
    fprintf('final P_shell: %f\n', P_shell);
    fprintf('final P_out:   %f\n', P_out);

    d = DdDa * a;
    X_in_result  = X_0 + bsxfun(@times, d, V);

%%
% Buoyancy
    function [f, gf] = Target1(a)
        [ P_in, P_out, P_shell, DmDd_in, DmDd_out, DmDd_shell ] = CompPhyProperty(X_0, V, Tri, DdDa, a, VERBOSE);

        f0 = P_shell(M1);
        % COM_shell = center of gravtiy
        % COM_out = center of boyoucy
        % center of masses
        f1x = (P_shell(CX)-P_out(CX))^2;
        f1y = (P_shell(CY)-P_out(CY))^2;
        f1z = (P_shell(CZ)-P_out(CZ))^2;
        % disY must be: Cy_out>Cy_shell

        % rroh = rho_water/rho_mat
        %rrho=rho(2)/rho(1);
        f2 = (rho(2)*P_out(M1)-rho(1)*P_shell(M1))^2;

        % boyouncy point towards center
        f3 = P_out(CX)^2+P_out(CY)^2+P_out(CZ)^2;

        % maxmimize the distance between COG and POB
        f4 = P_shell(CZ);

        % this term penalizes outer displacement
        f5 = sum(a.^2);

        % objective
        f = w1(1)*(f1x+f1y) + w1(2)*f2 + w1(3)*f3 + w1(4)*f4 + w1(5)*f1y + w1(6)*f5;
        % f = -f0;

        if nargout>1
            % compute final derivatives
            DP_shell = DmDd_shell(:, 1:np)*DdDa;
            DP_out = DmDd_out*DdDa;
            DP_shell(M1,:) = rho(1)*DP_shell(M1,:);
            DP_out(M1,:) = rho(2)*DP_out(M1,:);

            gf0 = (DP_shell(M1, :));

            gf1x = 2*(P_out(CX) - P_shell(CX))*(DP_out(CX,:) - DP_shell(CX,:));
            gf1y = 2*(P_out(CY) - P_shell(CY))*(DP_out(CY,:) - DP_shell(CY,:));
            gf1z = 2*(P_out(CZ) - P_shell(CZ))*(DP_out(CZ,:) - DP_shell(CZ,:));
            gf2  = 2*(rho(2)*P_out(M1) - rho(1)*P_shell(M1))*(rho(2)*DP_out(M1,:) - rho(1)*DP_shell(M1,:));

            gf3  = 2*(P_out(CX)*DP_out(CX,:)+P_out(CY)*DP_out(CY,:)+P_out(CZ)*DP_out(CZ,:));

            gf4  = DP_shell(CZ,:);

            gf5 = 2 * a;

            gf = w1(1)*(gf1x+gf1y)' + w1(2)*gf2' + w1(3)*gf3' + w1(4)*gf4' + w1(6)*gf5 + w1(5)*gf1y';
        end
    end

%  NonLcon_Buoyancy
    function [ c, ceq, gc, gceq ] = NonLcon1(a0)

        ceq = [
            P_shell(CX)-P_out(CX);
            P_shell(CY)-P_out(CY);
            ];
        c = [
            P_shell(CZ)-P_out(CZ);
            ];

        if nargout>2
            gceq = [
                DP_shell(CX,:)-DP_out(CX,:);
                DP_shell(CY,:)-DP_out(CY,:);
                ]';
            gc = [
                DP_shell(CZ,:)-DP_out(CZ,:);
                ]';
        end
    end

%%

% Static
    function [f, gf] = Target2(a)
        [ P_in, P_out, P_shell, DmDd_in, DmDd_out, DmDd_shell ] = CompPhyProperty(X_0, V, Tri, DdDa, a, VERBOSE);

        % COM_shell = center of gravtiy
        % COM_out = center of boyoucy
        % center of masses
        f0 = P_shell(M1);

        f1x = P_shell(CX) ^ 2;
        f1y = P_shell(CY) ^ 2;
        f1z = P_shell(CZ);

        f5 = sum(a.^2);

        f =  w2(1)*f0 + w2(2)*f1z + w2(4)*f5 + w2(3) * (f1x + f1y);

        if nargout>1
            % compute final derivatives
            DP_shell = DmDd_shell(:, 1:np)*DdDa;
            DP_out = DmDd_out*DdDa;
            DP_shell(M1,:) = rho(1)*DP_shell(M1,:);
            DP_out(M1,:) = rho(2)*DP_out(M1,:);

            gf0 = (DP_shell(M1, :));
            gf1x = 2*(P_shell(CX))*(DP_shell(CX,:));
            gf1y = 2*(P_shell(CY))*(DP_shell(CY,:));
            gf1z = DP_shell(CZ,:);

            gf5 = 2 * a';

            gf = w2(1) * gf0 + w2(2) * gf1z + w2(4) * gf5 + w2(3) * (gf1x + gf1y);
        end
    end

% use global: R_, r_
%  NonLcon_Static
    function [ c, ceq, gc, gceq ] = NonLcon2(a0)
        c = [
            (P_shell(CX) + P_shell(CY)) ^ 2 - r_ ^ 2;
            - P_shell(CZ) * 0
            ];
        ceq = 0;

        if nargout>2
            gc = [
                2 * (P_shell(CX) + P_shell(CY)) * (DP_shell(CX,:) + DP_shell(CY,:));
                - DP_shell(CZ, :) * 0
                ]';
            gceq = zeros(k, 1);
        end
    end

%%
%  No NonLcon
    function [ c, ceq, gc, gceq ] = NonLcon0(a0)
        c = -1;
        ceq = 0;
        gc = zeros(k, 1);
        gceq = zeros(k, 1);
    end
end