function ShowDiffModel( X, S, Tri )
%     figure;
%     scatter3(X(:,1), X(:,2), X(:,3), '.');
%     axis equal;
    figure
    trisurf(Tri, X(:,1), X(:,2), X(:,3), ...
        'FaceAlpha', 0.3, 'FaceLighting', 'gouraud', ...
        'EdgeColor', 'none', 'EdgeLighting', 'none', ...
        'EdgeAlpha', 0.5);
    light;
    hold on;
    trisurf(Tri, S(:,1), S(:,2), S(:,3), ...
        'FaceAlpha', 0.5, 'FaceLighting', 'gouraud', ...
        'EdgeColor', 'flat', 'EdgeLighting', 'gouraud');
    hold off;
    axis equal;
%     light('Position', [-1,-1,0], 'Color', [1,1,1], 'Style', 'local');
end

