function ShowModel( X, Tri )
%     figure;
%     scatter3(X(:,1), X(:,2), X(:,3), '.');
%     axis equal;
    figure
    trisurf(Tri, X(:,1), X(:,2), X(:,3), ...
        'FaceAlpha', 0.5, 'FaceLighting', 'gouraud', ...
        'EdgeColor', 'flat', 'EdgeLighting', 'gouraud');
    axis equal;
%     light('Position', [-1,-1,0], 'Color', [1,1,1], 'Style', 'local');
end

