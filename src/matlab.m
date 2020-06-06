
%point of interest
P = [10,40]


W=20 %width of rectangle
L=80; % length of rectangle

% Matrix->  points of the ractangle
R = [P(1) P(1)+L P(1)+L P(1);
    (P(2)-(W/2)) (P(2)-(W/2)) (P(2)+(W/2)) (P(2)+(W/2))]

    
    
%Rotate matrix
Theta = +45

RM = [ (cos((Theta*pi)/180))  (sin((Theta*pi)/180));
      (-sin((Theta*pi)/180)) (cos((Theta*pi)/180))  ]
    
figure

ResultMatrix = RM*R
%ResultMatrix = R 
for col=1:size(ResultMatrix,2)
    hold on;
    plot(ResultMatrix(1,col),ResultMatrix(2,col), 'r', 'MarkerSize', 30);
end  
xlim([-100 100])
ylim([-100 100])